#include "ChatController.h"

#include "../app/SessionManager.h"
#include "../app/DebugController.h"
#include "../data/DatabaseManager.h"
#include "../infrastructure/Logger.h"

#include <QDateTime>

namespace glm {

ChatController::ChatController(ProviderRegistry *registry, SessionManager *sessions,
                               DebugController *debug, QObject *parent)
    : QObject(parent)
    , m_registry(registry)
    , m_provider(registry ? registry->provider(registry->ids().isEmpty() ? QString() : registry->ids().first()) : nullptr)
    , m_sessions(sessions)
    , m_debug(debug)
{
}

void ChatController::persistMessage(const Message &m)
{
    if (!m_sessions) return;
    const QString sid = m_sessions->currentSessionId();
    if (!sid.isEmpty()) DatabaseManager::instance().appendMessage(sid, m);
}

void ChatController::recordDebug(LlmReply *reply)
{
    if (!m_debug || !reply) return;
    RequestRecord r;
    r.sessionId = m_sessions ? m_sessions->currentSessionId() : QString();
    r.model = m_params.model;
    r.temperature = m_params.temperature;
    r.topP = m_params.topP;
    r.maxTokens = m_params.maxTokens;
    r.rawRequest = reply->rawRequest();
    r.rawResponse = reply->rawResponse();
    r.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_debug->record(r);
}

void ChatController::send(const QString &userText)
{
    // 只拒"生成中"(Sending/Streaming),允许终态(Idle/Finished/Error/Aborted)再发新消息
    if (m_state == State::Sending || m_state == State::Streaming) {
        logWarning("chat", QStringLiteral("send ignored: 生成中"));
        return;
    }
    if (!m_provider) { emit errorOccurred(QStringLiteral("未配置 Provider")); return; }

    Message userMsg;
    userMsg.role = Role::User;
    userMsg.content = userText;
    userMsg.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_messages.append(userMsg);
    emit messageAppended(userMsg);
    persistMessage(userMsg);

    LlmRequest req;
    req.model = m_params.model;
    req.messages = m_messages;
    req.temperature = m_params.temperature;
    req.topP = m_params.topP;
    req.maxTokens = m_params.maxTokens;
    req.stream = true;

    setState(State::Sending);

    Message asstMsg;
    asstMsg.role = Role::Assistant;
    asstMsg.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_messages.append(asstMsg);
    emit messageAppended(asstMsg);
    // 不预占插 DB:assistant content 空 → NOT NULL 失败;finished 时插(有内容)

    m_currentReply = m_provider->send(req);
    m_currentReply->setParent(this);
    connectReply(m_currentReply);
}

void ChatController::stop()
{
    if (m_state != State::Sending && m_state != State::Streaming) return;
    setState(State::Aborted);
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply = nullptr;   // 防御:abort 后置空,防异步信号期间访问
    }
}

void ChatController::clearHistory()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply = nullptr;
    }
    m_messages.clear();
    setState(State::Idle);
}

void ChatController::setParams(const GenerationParams &p)
{
    m_params = p;
    logDebug("chat", QStringLiteral("params: model=%1 temp=%2").arg(p.model).arg(p.temperature));
}

void ChatController::setProviderById(const QString &id)
{
    if (!m_registry) return;
    ILlmProvider *p = m_registry->provider(id);
    if (p && p != m_provider) {
        m_provider = p;
        emit providerChanged(id);
        logInfo("chat", QStringLiteral("provider switched to %1").arg(id));
    }
}

QString ChatController::currentProviderId() const
{
    return m_provider ? m_provider->id() : QString();
}

QStringList ChatController::providerIds() const
{
    return m_registry ? m_registry->ids() : QStringList{};
}

void ChatController::setSession(const QList<Message> &msgs)
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply = nullptr;
    }
    m_messages = msgs;
    setState(State::Idle);
    emit historyReplaced(msgs);
}

void ChatController::connectReply(LlmReply *reply)
{
    QObject::connect(reply, &LlmReply::chunkReceived, this, [this](const QString &text) {
        if (m_state == State::Sending) setState(State::Streaming);
        if (!m_messages.isEmpty() && m_messages.last().role == Role::Assistant) {
            m_messages.last().content += text;
            // 流式不每 chunk 写 DB(性能);finished 全量插
        }
        emit chunkReceived(text);
    });

    QObject::connect(reply, &LlmReply::finished, this, [this, reply](const QString &fullText) {
        if (m_state == State::Aborted) return;
        if (!m_messages.isEmpty() && m_messages.last().role == Role::Assistant) {
            m_messages.last().content = fullText;
            persistMessage(m_messages.last());   // finished 插完整 assistant(有 content)
        }
        recordDebug(reply);   // P4:记录请求/响应到调试库
        setState(State::Finished);
        emit finished(fullText);
    });

    QObject::connect(reply, &LlmReply::errorOccurred, this, [this](const QString &err) {
        if (m_state == State::Aborted) return;
        if (!m_messages.isEmpty() && m_messages.last().role == Role::Assistant
            && m_messages.last().content.isEmpty()) {
            m_messages.removeLast();
        }
        setState(State::Error);
        emit errorOccurred(err);
    });

    QObject::connect(reply, &LlmReply::done, this, [this]() {
        if (m_currentReply) { m_currentReply->deleteLater(); m_currentReply = nullptr; }
    });

    QObject::connect(reply, &LlmReply::usageReceived, this, [this](int p, int c, int t) {
        emit tokenReported(p, c, t);
    });
}

void ChatController::setState(State s)
{
    if (m_state == s) return;
    m_state = s;
    logDebug("chat", QStringLiteral("state -> %1").arg(static_cast<int>(s)));
    emit stateChanged(s);
}

} // namespace glm
