#ifndef GLM_HTTP_CLIENT_H
#define GLM_HTTP_CLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QMap>
#include <QByteArray>
#include <functional>

namespace glm {

// HTTP 请求描述
struct HttpRequest {
    QUrl url;
    QMap<QByteArray, QByteArray> headers;
    QByteArray body;
    bool stream = false;   // true: SSE 流式(readyRead 增量)
};

// HTTP 响应(异步,流式发 dataReceived)。
// 信号是 protected,外部不能直接 emit,故提供 emit 辅助方法给 HttpClient。
class HttpResponse : public QObject
{
    Q_OBJECT
public:
    explicit HttpResponse(QObject *parent = nullptr) : QObject(parent) {}

    // 中断:HttpClient 注入(调底层 QNetworkReply::abort)
    void setAbortHandle(std::function<void()> h) { m_aborter = std::move(h); }
    void abort() { if (m_aborter) m_aborter(); }

    // HttpClient 调用推进度
    void emitData(const QByteArray &chunk) { emit dataReceived(chunk); }
    void emitError(const QString &e) { emit errorOccurred(e); }
    void emitFinished() { emit finished(); }

signals:
    void dataReceived(const QByteArray &chunk);
    void finished();                            // 终态(成功/失败都发;errorOccurred 会先于 finished)
    void errorOccurred(const QString &error);

private:
    std::function<void()> m_aborter;
};

// HTTP 客户端:封装 QNetworkAccessManager(连接复用)。
// Provider 通过它发请求,网络层只此一处依赖 Qt 网络。
class HttpClient : public QObject
{
    Q_OBJECT
public:
    explicit HttpClient(QObject *parent = nullptr);
    HttpResponse *post(const HttpRequest &req);

private:
    QNetworkAccessManager *m_nam;
};

} // namespace glm

#endif // GLM_HTTP_CLIENT_H
