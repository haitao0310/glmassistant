#ifndef GLM_GLM_PROVIDER_H
#define GLM_GLM_PROVIDER_H

#include <QObject>
#include "../core/ILlmProvider.h"
#include "../core/LlmTypes.h"

namespace glm {

class HttpClient;

// GLM(智谱)Provider —— QObject + ILlmProvider 多继承。
// QObject 让 main 能 parent 管理生命周期(无泄漏);ILlmProvider 是功能契约。
class GlmProvider : public QObject, public ILlmProvider
{
    Q_OBJECT
public:
    GlmProvider(QString apiKey, HttpClient *http, QObject *parent = nullptr);

    QString id() const override { return QStringLiteral("glm"); }
    QString displayName() const override { return QStringLiteral("智谱 GLM"); }
    QStringList models() const override;

    LlmReply *send(const LlmRequest &req) override;

private:
    QByteArray buildRequestBody(const LlmRequest &req) const;   // OpenAI 兼容 JSON
    QString m_apiKey;
    HttpClient *m_http;
};

} // namespace glm

#endif // GLM_GLM_PROVIDER_H
