#include "ChatController.h"

#include "../infrastructure/Logger.h"

#include <QDateTime>

namespace glm {

ChatController::ChatController(ILlmProvider *provider, QObject *parent)
    : QObject(parent)
    , m_provider(provider)
{
}

void ChatController::send(const QString &userText)
{
    if (m_state != State::Idle) {
        logWarning("chat", QStringLiteral("send ignored: not idle"));
        return;
    }
    if (!m_provider) { emit errorOccurred(QStringLiteral("未配置 Provider")); return; }

    // 用户消息入历史(P2 多轮)
    Message userMsg;
    userMsg.role = Role::User;
    userMsg.content = userText;
    userMsg.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_messages.append(userMsg);
    emit messageAppended(userMsg);

    // 构造请求:完整历史 + 当前参数
    LlmRequest req;
    req.model = m_params.model;
    req.messages = m_messages;
    req.temperature = m_params.temperature;
    req.topP = m_params.topP;
    req.maxTokens = m_params.maxTokens;
    req.stream = true;

    setState(State::Sending);

    // 预占 assistant 消息(流式 chunk 填充)
    Message asstMsg;
    asstMsg.role = Role::Assistant;
    asstMsg.timestamp = QDateTime::currentMSecsSinceEpoch();
    m_messages.append(asstMsg);
    emit messageAppended(asstMsg);

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

void ChatController::connectReply(LlmReply *reply)
{
    QObject::connect(reply, &LlmReply::chunkReceived, this, [this](const QString &text) {
        if (m_state == State::Sending) setState(State::Streaming);
        if (!m_messages.isEmpty() && m_messages.last().role == Role::Assistant) {
            m_messages.last().content += text;   // 累积到历史(下次请求带正确 assistant 回复)
        }
        emit chunkReceived(text);
    });

    QObject::connect(reply, &LlmReply::finished, this, [this](const QString &fullText) {
        if (m_state == State::Aborted) return;
        if (!m_messages.isEmpty() && m_messages.last().role == Role::Assistant) {
            m_messages.last().content = fullText;   // 全量覆盖保历史准
        }
        setState(State::Finished);
        emit finished(fullText);
    });

    QObject::connect(reply, &LlmReply::errorOccurred, this, [this](const QString &err) {
        if (m_state == State::Aborted) return;
        // 错误回滚预占的空 assistant
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
}

void ChatController::setState(State s)
{
    if (m_state == s) return;
    m_state = s;
    logDebug("chat", QStringLiteral("state -> %1").arg(static_cast<int>(s)));
    emit stateChanged(s);
}

} // namespace glm
