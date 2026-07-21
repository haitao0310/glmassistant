#ifndef GLM_CHAT_CONTROLLER_H
#define GLM_CHAT_CONTROLLER_H

#include <QObject>
#include <QString>
#include <QList>
#include "../core/ILlmProvider.h"
#include "../core/LlmTypes.h"
#include "../core/LlmReply.h"
#include "../core/ProviderRegistry.h"

namespace glm {

class SessionManager;
class DebugController;

// 业务层:协调 UI ↔ Provider,持多轮历史 + 生成参数,管理状态机(ADR-009)。
// P:持 ProviderRegistry(多 Provider 热切换,不硬编码单个 provider)。
class ChatController : public QObject
{
    Q_OBJECT
public:
    enum class State { Idle, Sending, Streaming, Finished, Error, Aborted };
    Q_ENUM(State)

    ChatController(ProviderRegistry *registry, SessionManager *sessions,
                   DebugController *debug = nullptr, QObject *parent = nullptr);

    void send(const QString &userText);
    void stop();
    void clearHistory();
    void setParams(const GenerationParams &p);
    void setSession(const QList<Message> &msgs);

    // Provider 热切换
    void setProviderById(const QString &id);
    QString currentProviderId() const;
    QStringList providerIds() const;

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
    void providerChanged(const QString &id);

private:
    ProviderRegistry *m_registry;
    ILlmProvider *m_provider;
    SessionManager *m_sessions;
    DebugController *m_debug;
    LlmReply *m_currentReply = nullptr;
    State m_state = State::Idle;
    QList<Message> m_messages;
    GenerationParams m_params;
    int m_lastPromptTokens = 0;
    int m_lastCompletionTokens = 0;
    int m_lastTotalTokens = 0;

    void setState(State s);
    void connectReply(LlmReply *reply);
    void persistMessage(const Message &m);
    void recordDebug(LlmReply *reply);
};

} // namespace glm

#endif // GLM_CHAT_CONTROLLER_H
