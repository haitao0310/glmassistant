#include "HttpClient.h"

#include "../infrastructure/Logger.h"
#include "../infrastructure/Constants.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <functional>
#include <memory>

namespace glm {

HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

// POST(含网络层重试 + 指数退避;HTTP 4xx/5xx 不重试——是逻辑错)
HttpResponse* HttpClient::post(const HttpRequest &req)
{
    auto *response = new HttpResponse(this);   // parent=HttpClient,跟随生命周期

    auto attempt = std::make_shared<int>(0);
    auto doRequest = std::make_shared<std::function<void()>>();
    auto *nam = m_nam;
    const int retryCount = m_retryCount;

    *doRequest = [nam, req, response, attempt, doRequest, retryCount]() {
        QNetworkRequest nr(req.url);
        for (auto it = req.headers.constBegin(); it != req.headers.constEnd(); ++it) {
            nr.setRawHeader(it.key(), it.value());
        }
        nr.setTransferTimeout(constants::HTTP_TIMEOUT_MS);

        QNetworkReply *reply = nam->post(nr, req.body);
        response->setAbortHandle([reply]() { if (reply) reply->abort(); });

        // 流式增量
        QObject::connect(reply, &QIODevice::readyRead, response, [reply, response]() {
            const QByteArray chunk = reply->readAll();
            if (!chunk.isEmpty()) response->emitData(chunk);
        });

        // 错误:网络层(httpCode==0,连接/超时)重试;HTTP 4xx/5xx 不重试
        QObject::connect(reply, &QNetworkReply::errorOccurred, response,
                         [req, response, attempt, doRequest, retryCount, reply](QNetworkReply::NetworkError) {

                             const int httpCode =
                                 reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

                             const QString err = reply->errorString();

                             QByteArray body = reply->readAll();

                             logError("network",
                                      QStringLiteral("HTTP %1\nQtError: %2\nServerBody: %3")
                                          .arg(httpCode)
                                          .arg(err)
                                          .arg(QString::fromUtf8(body)));

                             // 网络错(httpCode==0)无响应体,用 errorString;HTTP 错用 body
                             response->emitError(
                                 body.isEmpty() ? err : QString::fromUtf8(body)
                             );
                         });

        // 完成(成功/最终失败)。重试时旧 reply 也 finished,调用方靠 done 标志防重复
        QObject::connect(reply, &QNetworkReply::finished, response, [reply, response]() {
            response->emitFinished();
            reply->deleteLater();
        });
    };

    logInfo("network", QStringLiteral("POST %1 stream=%2 body=%3B")
                .arg(req.url.toString()).arg(req.stream).arg(req.body.size()));
    (*doRequest)();
    return response;
}

} // namespace glm
