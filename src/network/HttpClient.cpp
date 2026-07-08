#include "HttpClient.h"

#include "../infrastructure/Logger.h"
#include "../infrastructure/Constants.h"

#include <QNetworkRequest>
#include <QNetworkReply>

namespace glm {

HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

HttpResponse *HttpClient::post(const HttpRequest &req)
{
    auto *response = new HttpResponse(this);   // parent=HttpClient,跟随生命周期

    QNetworkRequest nr(req.url);
    for (auto it = req.headers.constBegin(); it != req.headers.constEnd(); ++it) {
        nr.setRawHeader(it.key(), it.value());
    }
    nr.setTransferTimeout(constants::HTTP_TIMEOUT_MS);   // Qt 6 超时(防永久挂起)

    QNetworkReply *reply = m_nam->post(nr, req.body);
    response->setAbortHandle([reply]() { if (reply) reply->abort(); });

    // 流式增量:readyRead 时读可读字节喂给 dataReceived(SSE 喂 SseParser)
    QObject::connect(reply, &QIODevice::readyRead, response, [reply, response]() {
        const QByteArray chunk = reply->readAll();
        if (!chunk.isEmpty()) response->emitData(chunk);
    });

    // 网络错误(含超时):先于 finished 触发
    QObject::connect(reply, &QNetworkReply::errorOccurred, response,
        [reply, response](QNetworkReply::NetworkError) {
            logError("network", reply->errorString());
            response->emitError(reply->errorString());
        });

    // 完成(成功/失败最终态)
    QObject::connect(reply, &QNetworkReply::finished, response, [reply, response]() {
        response->emitFinished();
        reply->deleteLater();
    });

    logInfo("network", QStringLiteral("POST %1 stream=%2 body=%3B")
                .arg(req.url.toString()).arg(req.stream).arg(req.body.size()));

    return response;
}

} // namespace glm
