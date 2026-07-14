#include "GlmProvider.h"

#include "../network/HttpClient.h"
#include "common/ProviderUtils.h"
#include "../infrastructure/Constants.h"
#include "../infrastructure/Logger.h"

#include <QUrl>

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

LlmReply *GlmProvider::send(const LlmRequest &req)
{
    auto *reply = new LlmReply();

    HttpRequest httpReq;
    httpReq.url = QUrl(constants::GLM_API_ENDPOINT);
    httpReq.headers["Authorization"] = "Bearer " + m_apiKey.toUtf8();
    httpReq.headers["Content-Type"] = "application/json";
    httpReq.body = serializeOpenAiRequest(req, constants::GLM_DEFAULT_MODEL);   // 通用序列化(提取)
    httpReq.stream = req.stream;
    reply->setRawRequest(QString::fromUtf8(httpReq.body));

    HttpResponse *resp = m_http->post(httpReq);
    reply->setAbortHandle([resp]() { resp->abort(); });
    setupSseStreaming(resp, reply);   // 通用 SSE setup(提取,消除与 Ollama 90% 重复)

    logInfo("glm", QStringLiteral("send model=%1 stream=%2 msgs=%3")
                .arg(req.model).arg(req.stream).arg(req.messages.size()));
    return reply;
}

} // namespace glm
