#ifndef GLM_LLM_TYPES_H
#define GLM_LLM_TYPES_H

#include <QString>
#include <QList>

namespace glm {

// 消息角色(对齐 OpenAI 兼容格式:system/user/assistant)
enum class Role { System, User, Assistant };

// 单条消息
struct Message {
    Role role = Role::User;
    QString content;
    qint64 timestamp = 0;    // 毫秒纪元
    int tokenCount = 0;      // P2 token 管理(0 = 未计算)

    QString roleName() const;              // "system"/"user"/"assistant"(序列化用)
    static Role roleFromName(const QString &name);
};

// 会话(一组消息 + 元数据)
struct ChatSession {
    QString id;             // UUID
    QString title;
    QList<Message> messages;
    qint64 createdTime = 0;
    qint64 updatedTime = 0;
};

// LLM 请求参数(中间件管道的输入,Provider 据此构造 HTTP 请求)
struct LlmRequest {
    QString providerId;     // "glm"(P5 多 provider 选择)
    QString model;          // "glm-4-flash"
    QList<Message> messages;
    qreal temperature = 0.7;
    qreal topP = 1.0;
    int maxTokens = 2048;
    bool stream = false;    // P1 起默认 true(SSE)
};

// 生成参数(UI 层集合,ParamPanel 持,构造 LlmRequest 时填入)
struct GenerationParams {
    QString model = QStringLiteral("glm-4-flash");
    qreal temperature = 0.7;
    qreal topP = 1.0;
    int maxTokens = 2048;
};

// 调试请求记录(P4 调试模式历史库:每次请求的原始 body + 响应 + 参数)
struct RequestRecord {
    int id = 0;
    QString sessionId;
    QString model;
    qreal temperature = 0.7;
    qreal topP = 1.0;
    int maxTokens = 2048;
    QString rawRequest;     // 发出的 OpenAI 兼容 JSON body
    QString rawResponse;    // 收到的原始响应(流式累积或整体)
    qint64 timestamp = 0;
};

} // namespace glm

#endif // GLM_LLM_TYPES_H
