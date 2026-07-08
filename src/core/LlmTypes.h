#ifndef GLM_LLM_TYPES_H
#define GLM_LLM_TYPES_H

#include <QString>
#include <QList>
#include <QJsonArray>

namespace glm {

// 消息角色(P5 加 Tool:function calling 工具结果)
enum class Role { System, User, Assistant, Tool };

struct Message {
    Role role = Role::User;
    QString content;
    qint64 timestamp = 0;
    int tokenCount = 0;

    QString roleName() const;
    static Role roleFromName(const QString &name);
};

struct ChatSession {
    QString id;
    QString title;
    QList<Message> messages;
    qint64 createdTime = 0;
    qint64 updatedTime = 0;
};

struct LlmRequest {
    QString providerId;
    QString model;
    QList<Message> messages;
    qreal temperature = 0.7;
    qreal topP = 1.0;
    int maxTokens = 2048;
    bool stream = false;
    QJsonArray tools;   // P5 function calling schemas(空=不用)
};

struct GenerationParams {
    QString model = QStringLiteral("glm-4-flash");
    qreal temperature = 0.7;
    qreal topP = 1.0;
    int maxTokens = 2048;
};

struct RequestRecord {
    int id = 0;
    QString sessionId;
    QString model;
    qreal temperature = 0.7;
    qreal topP = 1.0;
    int maxTokens = 2048;
    QString rawRequest;
    QString rawResponse;
    qint64 timestamp = 0;
};

} // namespace glm

#endif // GLM_LLM_TYPES_H
