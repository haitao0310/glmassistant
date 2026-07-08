#include "glmclient.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

GlmClient::GlmClient(const QString &apiKey, QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_apiKey(apiKey)
{
}

void GlmClient::sendMessage(const QString &userMessage)
{
    //GLM OpenAI 兼容接口
    QNetworkRequest req(QUrl("https://open.bigmodel.cn/api/paas/v4/chat/completions"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());

    //构造请求体:OpenAI 兼容格式
    QJsonObject body;
    body["model"] = QStringLiteral("glm-4-flash");   //免费版,开发测试用;正式可换 glm-4-plus
    QJsonArray messages;
    QJsonObject msg;
    msg["role"] = "user";
    msg["content"] = userMessage;
    messages.append(msg);
    body["messages"] = messages;
    body["stream"] = false;                          //第一步非流式;后面改 true 走 SSE
    body["temperature"] = 0.7;

    QNetworkReply *reply = m_nam->post(req, QJsonDocument(body).toJson());

    //异步:完成后解析(QNetworkReply::finished,不阻塞 UI)
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if(reply->error() != QNetworkReply::NoError){
            emit errorOccurred(reply->errorString());
            return;
        }
        //解析响应:choices[0].message.content
        const QJsonObject resp = QJsonDocument::fromJson(reply->readAll()).object();
        const QJsonArray choices = resp.value("choices").toArray();
        if(choices.isEmpty()){
            emit errorOccurred(QStringLiteral("响应无 choices 字段: ") + QString::fromUtf8(QJsonDocument(resp).toJson(QJsonDocument::Compact)));
            return;
        }
        const QString content = choices.at(0).toObject()
                                    .value("message").toObject()
                                    .value("content").toString();
        emit replyReceived(content);
    });
}
