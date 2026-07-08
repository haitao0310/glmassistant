#ifndef GLM_CHAT_CONTROLLER_H
#define GLM_CHAT_CONTROLLER_H

#include <QObject>
#include <QString>
#include "../core/ILlmProvider.h"
#include "../core/LlmTypes.h"
#include "../core/LlmReply.h"

namespace glm {

// 业务层:协调 UI ↔ Provider,集中管理生成状态机(ADR-009)。
//
// UI 不直接调 Provider,经 Controller——解耦 + 状态集中 + 便于测试(mock provider 注入)。
// 状态机:Idle → Sending → Streaming → Finished / Error / Aborted
class ChatController : public QObject
{
    Q_OBJECT
public:
    enum class State { Idle, Sending, Streaming, Finished, Error, Aborted };
    Q_ENUM(State)

    // provider 由 main 组装后注入(ADR-008),Controller 不自己 new
    explicit ChatController(ILlmProvider *provider, QObject *parent = nullptr);

    // 发送用户消息(构造 LlmRequest + 调 provider + 流式累积)
    void send(const QString &userText);

    // 中断当前生成 → Aborted(中断链路见反思:LlmReply 需暴露 abort)
    void stop();

    State state() const { return m_state; }

signals:
    void stateChanged(glm::ChatController::State state);
    void chunkReceived(const QString &text);     // 流式增量(UI 打字机用)
    void finished(const QString &fullText);       // 生成完成
    void errorOccurred(const QString &error);

private:
    ILlmProvider *m_provider;
    LlmReply *m_currentReply = nullptr;   // 当前请求(中断 + 生命周期)
    State m_state = State::Idle;

    void setState(State s);
    void connectReply(LlmReply *reply);   // 连接 chunk/finished/error 到本控制器
};

} // namespace glm

#endif // GLM_CHAT_CONTROLLER_H
