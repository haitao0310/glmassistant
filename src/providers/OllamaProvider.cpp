#include "OllamaProvider.h"

#include "../network/HttpClient.h"
#include "common/ProviderUtils.h"
#include "../infrastructure/Logger.h"

#include <QUrl>

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

LlmReply *OllamaProvider::send(const LlmRequest &req)
{
    auto *reply = new LlmReply();

    HttpRequest httpReq;
    httpReq.url = QUrl(QStringLiteral("http://127.0.0.1:11434/v1/chat/completions"));
    httpReq.headers["Content-Type"] = "application/json";   // Ollama 本地无 key
    httpReq.body = serializeOpenAiRequest(req, QStringLiteral("llama3.2"));   // 通用序列化
    httpReq.stream = req.stream;
    reply->setRawRequest(QString::fromUtf8(httpReq.body));

    HttpResponse *resp = m_http->post(httpReq);
    reply->setAbortHandle([resp]() { if (resp) resp->abort(); });
    setupSseStreaming(resp, reply);   // 通用 SSE setup(提取)

    logInfo("ollama", QStringLiteral("send model=%1 stream=%2").arg(req.model).arg(req.stream));
    return reply;
}

} // namespace glm
