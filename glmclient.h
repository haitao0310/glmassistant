#ifndef GLMCLIENT_H
#define GLMCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>

//GLM API 客户端:HTTP POST 调智谱 GLM,异步返回回复
//第一步:非流式(一次性返回);后续步骤改 SSE 流式
//对应产品级 7 模块的"网络层"
class GlmClient : public QObject
{
    Q_OBJECT
public:
    explicit GlmClient(const QString &apiKey, QObject *parent = nullptr);

    //发用户消息(第一步:单轮,不带历史;后续加 messages 历史做上下文)
    void sendMessage(const QString &userMessage);

signals:
    void replyReceived(const QString &content);   //收到 GLM 回复
    void errorOccurred(const QString &error);     //网络/解析出错

private:
    QNetworkAccessManager *m_nam;
    QString m_apiKey;
};

#endif // GLMCLIENT_H
