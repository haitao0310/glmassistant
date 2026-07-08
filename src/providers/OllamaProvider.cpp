#include "OllamaProvider.h"

#include "../network/HttpClient.h"
#include "../network/SseParser.h"
#include "../infrastructure/Logger.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <memory>

namespace glm {

OllamaProvider::OllamaProvider(HttpClient *http, QObject *parent)
    : QObject(parent)
    , m_http(http)
{
}

QStringList OllamaProvider::models() const
{
    return {QStringLiteral("llama3.2"), QStringLiteral("qwen2.5"), QStringLiteral("deepseek-r1")};
}

QByteArray OllamaProvider::buildRequestBody(const LlmRequest &req) const
{
    QJsonObject body;
    body["model"] = req.model.isEmpty() ? QStringLiteral("llama3.2") : req.model;
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
    if (req.maxTokens > 0) body["max_tokens"] = req.maxTokens;
    if (!req.tools.isEmpty()) body["tools"] = req.tools;
    return QJsonDocument(body).toJson(QJsonDocument::Compact);
}

LlmReply *OllamaProvider::send(const LlmRequest &req)
{
    auto *reply = new LlmReply();

    HttpRequest httpReq;
    httpReq.url = QUrl(QStringLiteral("http://127.0.0.1:11434/v1/chat/completions"));
    httpReq.headers["Content-Type"] = "application/json";   // Ollama 本地无 key
    httpReq.body = buildRequestBody(req);
    httpReq.stream = req.stream;
    reply->setRawRequest(QString::fromUtf8(httpReq.body));

    HttpResponse *resp = m_http->post(httpReq);
    reply->setAbortHandle([resp]() { if (resp) resp->abort(); });

    auto parser = std::make_shared<SseParser>();
    auto accumulated = std::make_shared<QString>();
    auto done = std::make_shared<bool>(false);

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
            reply->emitError(err);
        });
    QObject::connect(resp, &HttpResponse::finished, reply,
        [done, accumulated, reply]() {
            if (*done) return;
            *done = true;
            reply->emitFinished(*accumulated);
        });

    logInfo("ollama", QStringLiteral("send model=%1 stream=%2").arg(req.model).arg(req.stream));
    return reply;
}

} // namespace glm
