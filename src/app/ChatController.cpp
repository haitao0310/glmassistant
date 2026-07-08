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
        return;   // 状态机守卫,防重复点
    }
    if (!m_provider) {
        emit errorOccurred(QStringLiteral("未配置 Provider"));
        return;
    }

    // P1:单轮(只发当前 userText);P2 扩多轮 messages 列表
    LlmRequest req;
    req.model = QStringLiteral("glm-4-flash");   // TODO P2 从 Settings 读
    req.stream = true;
    Message m;
    m.role = Role::User;
    m.content = userText;
    m.timestamp = QDateTime::currentMSecsSinceEpoch();
    req.messages.append(m);

    setState(State::Sending);

    m_currentReply = m_provider->send(req);
    m_currentReply->setParent(this);   // 接管:Controller 析构时 reply 跟随
    connectReply(m_currentReply);
}

void ChatController::stop()
{
    if (m_state != State::Sending && m_state != State::Streaming) return;
    setState(State::Aborted);                            // 先切 Aborted(后续 error/finished 据此忽略)
    if (m_currentReply) m_currentReply->abort();         // → resp.error(Canceled)+finished → done
}

void ChatController::connectReply(LlmReply *reply)
{
    QObject::connect(reply, &LlmReply::chunkReceived, this, [this](const QString &text) {
        if (m_state == State::Sending) setState(State::Streaming);   // 首块 → Streaming
        emit chunkReceived(text);
    });

    QObject::connect(reply, &LlmReply::finished, this, [this](const QString &fullText) {
        if (m_state == State::Aborted) return;   // 主动中断,保持 Aborted
        setState(State::Finished);
        emit finished(fullText);
    });

    QObject::connect(reply, &LlmReply::errorOccurred, this, [this](const QString &err) {
        if (m_state == State::Aborted) return;   // abort 触发的 OperationCanceled,忽略
        setState(State::Error);
        emit errorOccurred(err);
    });

    // 终态统一清理(finished/error 后 LlmReply 都发 done)
    QObject::connect(reply, &LlmReply::done, this, [this]() {
        if (m_currentReply) {
            m_currentReply->deleteLater();
            m_currentReply = nullptr;
        }
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
