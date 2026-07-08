#ifndef GLM_GLM_EMBEDDING_PROVIDER_H
#define GLM_GLM_EMBEDDING_PROVIDER_H

#include "../rag/IEmbeddingProvider.h"
#include <QString>

namespace glm {

class HttpClient;

// GLM embedding-3 实现(同步,用 QEventLoop 等待 HTTP 完成)。
class GlmEmbeddingProvider : public IEmbeddingProvider
{
public:
    GlmEmbeddingProvider(QString apiKey, HttpClient *http);
    QVector<float> embed(const QString &text) override;

private:
    QString m_apiKey;
    HttpClient *m_http;
};

} // namespace glm

#endif // GLM_GLM_EMBEDDING_PROVIDER_H
