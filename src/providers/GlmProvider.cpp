#include "GlmProvider.h"

#include "../network/HttpClient.h"
#include "../network/SseParser.h"
#include "../infrastructure/Constants.h"
#include "../infrastructure/Logger.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <memory>

namespace glm {

GlmProvider::GlmProvider(QString apiKey, HttpClient *http, QObject *parent)
    : QObject(parent)
    , m_apiKey(std::move(apiKey))
    , m_http(http)
{
}

QStringList GlmProvider::models() const
{
    return constants::GLM_MODELS;
}

// OpenAI 兼容请求体
QByteArray GlmProvider::buildRequestBody(const LlmRequest &req) const
{
    QJsonObject body;
    body["model"] = req.model.isEmpty() ? constants::GLM_DEFAULT_MODEL : req.model;

    QJsonArray messages;
    for (const Message &m : req.messages) {
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

    return QJsonDocument(body).toJson(QJsonDocument::Compact);
}

LlmReply *GlmProvider::send(const LlmRequest &req)
{
    auto *reply = new LlmReply();   // 调用方(ChatController)接管生命周期

    HttpRequest httpReq;
    httpReq.url = QUrl(constants::GLM_API_ENDPOINT);
    httpReq.headers["Authorization"] = "Bearer " + m_apiKey.toUtf8();
    httpReq.headers["Content-Type"] = "application/json";
    httpReq.body = buildRequestBody(req);
    httpReq.stream = req.stream;

    HttpResponse *resp = m_http->post(httpReq);
    reply->setAbortHandle([resp]() { resp->abort(); });   // LlmReply.abort → resp.abort → QNetworkReply::abort

    // 解析状态用 shared_ptr 管理(随 reply 的 connect 释放,避免局部引用悬空)
    auto parser = std::make_shared<SseParser>();
    auto accumulated = std::make_shared<QString>();
    auto done = std::make_shared<bool>(false);   // 防 error+finished 重复处理

    // 流式增量:SSE 帧 → 增量文本 → chunkReceived
    QObject::connect(resp, &HttpResponse::dataReceived, reply,
        [parser, accumulated, reply](const QByteArray &chunk) {
            const QList<QString> payloads = parser->feed(chunk);
            for (const QString &p : payloads) {
                if (SseParser::isDone(p)) continue;          // 结束标志,等 finished
                const QString text = SseParser::extractDeltaContent(p);
                if (!text.isEmpty()) {
                    *accumulated += text;
                    reply->emitChunk(text);
                }
            }
        });

    // 错误(含超时/abort)。abort 触发 OperationCanceledError,这里统一 emitError,
    // ChatController.stop() 主动中断时已知是 Aborted,不把这条显示给用户。
    QObject::connect(resp, &HttpResponse::errorOccurred, reply,
        [done, reply](const QString &err) {
            if (*done) return;
            *done = true;
            logError("glm", QStringLiteral("request error: %1").arg(err));
            reply->emitError(err);
        });

    // 完成(成功路径)
    QObject::connect(resp, &HttpResponse::finished, reply,
        [done, accumulated, reply]() {
            if (*done) return;   // error 已处理,跳过(防重复 emitDone)
            *done = true;
            reply->emitFinished(*accumulated);
        });

    logInfo("glm", QStringLiteral("send model=%1 stream=%2 msgs=%3")
                .arg(req.model).arg(req.stream).arg(req.messages.size()));

    return reply;
}

} // namespace glm
