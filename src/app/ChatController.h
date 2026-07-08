#ifndef GLM_CHAT_CONTROLLER_H
#define GLM_CHAT_CONTROLLER_H

#include <QObject>
#include <QString>
#include <QList>
#include "../core/ILlmProvider.h"
#include "../core/LlmTypes.h"
#include "../core/LlmReply.h"

namespace glm {

// 业务层:协调 UI ↔ Provider,持多轮历史 + 生成参数,管理状态机(ADR-009)。
class ChatController : public QObject
{
    Q_OBJECT
public:
    enum class State { Idle, Sending, Streaming, Finished, Error, Aborted };
    Q_ENUM(State)

    explicit ChatController(ILlmProvider *provider, QObject *parent = nullptr);

    void send(const QString &userText);             // 加入历史 + 带完整 messages 请求
    void stop();
    void clearHistory();                             // 清空历史(新对话)
    void setParams(const GenerationParams &p);       // ParamPanel 调参

    State state() const { return m_state; }
    QList<Message> history() const { return m_messages; }
    GenerationParams params() const { return m_params; }

signals:
    void stateChanged(glm::ChatController::State state);
    void chunkReceived(const QString &text);
    void finished(const QString &fullText);
    void errorOccurred(const QString &error);
    void messageAppended(const glm::Message &m);     // 新消息入历史(UI 同步 ChatModel)
    void tokenReported(int promptTokens, int completionTokens, int totalTokens);

private:
    ILlmProvider *m_provider;
    LlmReply *m_currentReply = nullptr;
    State m_state = State::Idle;
    QList<Message> m_messages;                        // 多轮历史
    GenerationParams m_params;                        // 当前生成参数

    void setState(State s);
    void connectReply(LlmReply *reply);
};

} // namespace glm

#endif // GLM_CHAT_CONTROLLER_H
