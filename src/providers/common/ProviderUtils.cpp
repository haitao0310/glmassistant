#include "ProviderUtils.h"

#include "../../core/LlmReply.h"
#include "../../core/LlmTypes.h"
#include "../../network/HttpClient.h"
#include "../../network/SseParser.h"
#include "../../infrastructure/Logger.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>

namespace glm {

// 提取自 GlmProvider/OllamaProvider 的重复 SSE setup(原 90% 重复)
void setupSseStreaming(HttpResponse *resp, LlmReply *reply)
{
    auto parser = std::make_shared<SseParser>();
    auto accumulated = std::make_shared<QString>();
    auto done = std::make_shared<bool>(false);   // 防 error+finished 重复

    QObject::connect(resp, &HttpResponse::dataReceived, reply,
        [parser, accumulated, reply](const QByteArray &chunk) {
            reply->appendRawResponse(chunk);
            const QList<QString> payloads = parser->feed(chunk);
            for (const QString &p : payloads) {
                if (SseParser::isDone(p)) continue;
                const QString text = SseParser::extractDeltaContent(p);
                if (!text.isEmpty()) {
                    *accumulated += text;
                    reply->emitChunk(text);
                }
            }
        });

    QObject::connect(resp, &HttpResponse::errorOccurred, reply,
        [done, reply](const QString &err) {
            if (*done) return;
            *done = true;
            // 友好错误(防玩具 DoD):识别常见场景,给人话 + 解决建议
            QString friendly = err;
            const QString low = err.toLower();
            if (low.contains("authentication") || low.contains("401")) {
                friendly = QStringLiteral("API key 错误(401):检查环境变量/控制台 key 是否有效");
            } else if (low.contains("timeout") || low.contains("timed out")) {
                friendly = QStringLiteral("请求超时:检查网络连接后重试");
            } else if (low.contains("host") && (low.contains("not found") || low.contains("reach"))) {
                friendly = QStringLiteral("无法连接服务器:检查网络/代理");
            } else if (low.contains("rate") && low.contains("limit")) {
                friendly = QStringLiteral("请求过频(限流):稍后重试");
            }
            reply->emitError(friendly);
        });

    QObject::connect(resp, &HttpResponse::finished, reply,
        [done, accumulated, reply]() {
            if (*done) return;
            *done = true;
            reply->emitFinished(*accumulated);
        });
}

QByteArray serializeOpenAiRequest(const LlmRequest &req, const QString &defaultModel)
{
    QJsonObject body;
    body["model"] = req.model.isEmpty() ? defaultModel : req.model;

    QJsonArray messages;
    for (const Message &m : req.messages) {
        if (m.content.isEmpty()) {
            logWarning("provider", QStringLiteral("跳过空 content 消息(role=%1)").arg(m.roleName()));
            continue;   // 防御:空 content 服务端 400
        }
        QJsonObject mo;
        mo["role"] = m.roleName();
        mo["content"] = m.content;
        messages.append(mo);
    }
    body["messages"] = messages;
    body["stream"] = req.stream;
    body["temperature"] = req.temperature;
    body["top_p"] = req.topP;
    if (req.maxTokens > 0) body["max_tokens"] = req.maxTokens;
    if (!req.tools.isEmpty()) body["tools"] = req.tools;
    return QJsonDocument(body).toJson(QJsonDocument::Compact);
}

} // namespace glm
