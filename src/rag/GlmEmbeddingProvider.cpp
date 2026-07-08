#include "GlmEmbeddingProvider.h"

#include "../network/HttpClient.h"
#include "../infrastructure/Logger.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QEventLoop>

namespace glm {

GlmEmbeddingProvider::GlmEmbeddingProvider(QString apiKey, HttpClient *http)
    : m_apiKey(std::move(apiKey))
    , m_http(http)
{
}

// 同步 embed:POST embedding API + QEventLoop 等待 + 解析 data[0].embedding
QVector<float> GlmEmbeddingProvider::embed(const QString &text)
{
    if (!m_http || text.isEmpty()) return {};

    HttpRequest req;
    req.url = QUrl(QStringLiteral("https://open.bigmodel.cn/api/paas/v4/embeddings"));
    req.headers["Authorization"] = "Bearer " + m_apiKey.toUtf8();
    req.headers["Content-Type"] = "application/json";
    QJsonObject body;
    body["model"] = QStringLiteral("embedding-3");
    body["input"] = text;
    req.body = QJsonDocument(body).toJson(QJsonDocument::Compact);

    HttpResponse *resp = m_http->post(req);
    QEventLoop loop;
    QByteArray fullResponse;
    bool errored = false;
    QObject::connect(resp, &HttpResponse::dataReceived, &loop, [&fullResponse](const QByteArray &c){ fullResponse += c; });
    QObject::connect(resp, &HttpResponse::errorOccurred, &loop, [&loop, &errored](const QString &){ errored = true; loop.quit(); });
    QObject::connect(resp, &HttpResponse::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (errored) { logError("embedding", QStringLiteral("embed request failed")); return {}; }

    const QJsonDocument doc = QJsonDocument::fromJson(fullResponse);
    const QJsonArray data = doc.object().value("data").toArray();
    if (data.isEmpty()) return {};
    const QJsonArray embedding = data.at(0).toObject().value("embedding").toArray();
    QVector<float> result;
    result.reserve(embedding.size());
    for (const QJsonValue &v : embedding) result.append(static_cast<float>(v.toDouble()));
    return result;
}

} // namespace glm
