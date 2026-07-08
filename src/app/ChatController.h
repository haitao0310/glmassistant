#ifndef GLM_CHAT_CONTROLLER_H
#define GLM_CHAT_CONTROLLER_H

#include <QObject>
#include <QString>
#include <QList>
#include "../core/ILlmProvider.h"
#include "../core/LlmTypes.h"
#include "../core/LlmReply.h"

namespace glm {

class SessionManager;
class DebugController;

// 业务层:协调 UI ↔ Provider,持多轮历史 + 生成参数,管理状态机(ADR-009)。
// P3:历史持久化 SQLite;P4:请求完成记入 DebugController(调试历史库)。
class ChatController : public QObject
{
    Q_OBJECT
public:
    enum class State { Idle, Sending, Streaming, Finished, Error, Aborted };
    Q_ENUM(State)

    ChatController(ILlmProvider *provider, SessionManager *sessions,
                   DebugController *debug = nullptr, QObject *parent = nullptr);

    void send(const QString &userText);
    void stop();
    void clearHistory();
    void setParams(const GenerationParams &p);
    void setSession(const QList<Message> &msgs);

    State state() const { return m_state; }
    QList<Message> history() const { return m_messages; }
    GenerationParams params() const { return m_params; }

signals:
    void stateChanged(glm::ChatController::State state);
    void chunkReceived(const QString &text);
    void finished(const QString &fullText);
    void errorOccurred(const QString &error);
    void messageAppended(const glm::Message &m);
    void historyReplaced(const QList<glm::Message> &msgs);
    void tokenReported(int promptTokens, int completionTokens, int totalTokens);

private:
    ILlmProvider *m_provider;
    SessionManager *m_sessions;
    DebugController *m_debug;
    LlmReply *m_currentReply = nullptr;
    State m_state = State::Idle;
    QList<Message> m_messages;
    GenerationParams m_params;

    void setState(State s);
    void connectReply(LlmReply *reply);
    void persistMessage(const Message &m);
    void persistUpdateLast(const QString &content);
    void recordDebug(LlmReply *reply);   // P4:请求完成记录到调试库
};

} // namespace glm

#endif // GLM_CHAT_CONTROLLER_H
