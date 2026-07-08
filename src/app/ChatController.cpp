#include "ChatController.h"

#include "../app/SessionManager.h"
#include "../data/DatabaseManager.h"
#include "../infrastructure/Logger.h"

#include <QDateTime>

namespace glm {

ChatController::ChatController(ILlmProvider *provider, SessionManager *sessions, QObject *parent)
    : QObject(parent)
    , m_provider(provider)
    , m_sessions(sessions)
{
}

void ChatController::persistMessage(const Message &m)
{
    if (!m_sessions) return;
    const QString sid = m_sessions->currentSessionId();
    if (!sid.isEmpty()) DatabaseManager::instance().appendMessage(sid, m);
}

void ChatController::persistUpdateLast(const QString &content)
{
    if (!m_sessions) return;
    const QString sid = m_sessions->currentSessionId();
    if (!sid.isEmpty()) DatabaseManager::instance().updateLastMessageContent(sid, content);
}

void ChatController::send(const QString &userText)
{
    if (m_state != State::Idle) {
        logWarning("chat", QStringLiteral("send ignored: not idle"));
        return;
    }
    if (!m_provider) { emit errorOccurred(QStringLiteral("未配置 Provider")); return; }

    // 用户消息入历史(内存 + DB)
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

    // 预占 assistant(内存 + DB)
    Message asstMsg;
    asstMsg.role = Role::Assistant;
    asstMsg.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_messages.append(asstMsg);
    emit messageAppended(asstMsg);
    persistMessage(asstMsg);

    m_currentReply = m_provider->send(req);
    m_currentReply->setParent(this);
    connectReply(m_currentReply);
}

void ChatController::stop()
{
    if (m_state != State::Sending && m_state != State::Streaming) return;
    setState(State::Aborted);
    if (m_currentReply) m_currentReply->abort();
}

void ChatController::clearHistory()
{
    if (m_currentReply) m_currentReply->abort();
    m_messages.clear();
    setState(State::Idle);
}

void ChatController::setParams(const GenerationParams &p)
{
    m_params = p;
    logDebug("chat", QStringLiteral("params: model=%1 temp=%2").arg(p.model).arg(p.temperature));
}

void ChatController::setSession(const QList<Message> &msgs)
{
    if (m_currentReply) m_currentReply->abort();
    m_messages = msgs;
    setState(State::Idle);
    emit historyReplaced(msgs);   // UI 重建 ChatModel
}

void ChatController::connectReply(LlmReply *reply)
{
    QObject::connect(reply, &LlmReply::chunkReceived, this, [this](const QString &text) {
        if (m_state == State::Sending) setState(State::Streaming);
        if (!m_messages.isEmpty() && m_messages.last().role == Role::Assistant) {
            m_messages.last().content += text;
            persistUpdateLast(m_messages.last().content);   // DB 流式更新
        }
        emit chunkReceived(text);
    });

    QObject::connect(reply, &LlmReply::finished, this, [this](const QString &fullText) {
        if (m_state == State::Aborted) return;
        if (!m_messages.isEmpty() && m_messages.last().role == Role::Assistant) {
            m_messages.last().content = fullText;
            persistUpdateLast(fullText);   // DB 全量
        }
        setState(State::Finished);
        emit finished(fullText);
    });

    QObject::connect(reply, &LlmReply::errorOccurred, this, [this](const QString &err) {
        if (m_state == State::Aborted) return;
        if (!m_messages.isEmpty() && m_messages.last().role == Role::Assistant
            && m_messages.last().content.isEmpty()) {
            m_messages.removeLast();
            // DB 末条留空 assistant(TODO:清理空行,或改 deleteLastMessage)
        }
        setState(State::Error);
        emit errorOccurred(err);
    });

    QObject::connect(reply, &LlmReply::done, this, [this]() {
        if (m_currentReply) { m_currentReply->deleteLater(); m_currentReply = nullptr; }
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
