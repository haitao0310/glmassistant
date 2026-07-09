#include "GlmEmbeddingProvider.h"

#include "../network/HttpClient.h"
#include "../infrastructure/Logger.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QEventLoop>

namespace glm {

GlmEmbeddingProvider::GlmEmbeddingProvider(QString apiKey, HttpClient *http, QObject *parent)
    : QObject(parent)
    , m_apiKey(std::move(apiKey))
    , m_http(http)
{
}

// 共享:从响应解析 data[0].embedding → QVector<float>
QVector<float> GlmEmbeddingProvider::parseEmbedding(const QByteArray &response)
{
    const QJsonDocument doc = QJsonDocument::fromJson(response);
    const QJsonArray data = doc.object().value("data").toArray();
    if (data.isEmpty()) return {};
    const QJsonArray embedding = data.at(0).toObject().value("embedding").toArray();
    QVector<float> result;
    result.reserve(embedding.size());
    for (const QJsonValue &v : embedding) result.append(static_cast<float>(v.toDouble()));
    return result;
}

// 同步 embed(QEventLoop;RetrievalTool 工具同步用)
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

    if (errored) { logError("embedding", QStringLiteral("embed failed")); return {}; }
    return parseEmbedding(fullResponse);
}

// C3 异步 embed(信号链,无 QEventLoop 嵌套;调用方 connect embeddingReady/Failed)
void GlmEmbeddingProvider::embedAsync(const QString &text)
{
    if (!m_http || text.isEmpty()) { emit embeddingFailed(QStringLiteral("无文本/http")); return; }
    HttpRequest req;
    req.url = QUrl(QStringLiteral("https://open.bigmodel.cn/api/paas/v4/embeddings"));
    req.headers["Authorization"] = "Bearer " + m_apiKey.toUtf8();
    req.headers["Content-Type"] = "application/json";
    QJsonObject body;
    body["model"] = QStringLiteral("embedding-3");
    body["input"] = text;
    req.body = QJsonDocument(body).toJson(QJsonDocument::Compact);

    HttpResponse *resp = m_http->post(req);
    auto *accumulated = new QByteArray();   // 累积响应(随 resp 生命周期清理)
    QObject::connect(resp, &HttpResponse::dataReceived, resp, [accumulated](const QByteArray &c){ *accumulated += c; });
    QObject::connect(resp, &HttpResponse::finished, resp, [this, accumulated]() {
        const QVector<float> vec = parseEmbedding(*accumulated);
        delete accumulated;
        if (vec.isEmpty()) emit embeddingFailed(QStringLiteral("embedding 解析失败"));
        else emit embeddingReady(vec);
    });
    QObject::connect(resp, &HttpResponse::errorOccurred, resp, [this, accumulated](const QString &e) {
        delete accumulated;
        emit embeddingFailed(e);
    });
}

} // namespace glm
