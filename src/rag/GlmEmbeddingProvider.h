#ifndef GLM_GLM_EMBEDDING_PROVIDER_H
#define GLM_GLM_EMBEDDING_PROVIDER_H

#include <QObject>
#include "../rag/IEmbeddingProvider.h"
#include <QString>
#include <QVector>

namespace glm {

class HttpClient;

// GLM embedding-3。C3:加 embedAsync(信号链,去 QEventLoop 嵌套)。
class GlmEmbeddingProvider : public QObject, public IEmbeddingProvider
{
    Q_OBJECT
public:
    GlmEmbeddingProvider(QString apiKey, HttpClient *http, QObject *parent = nullptr);

    QVector<float> embed(const QString &text) override;   // 同步(QEventLoop,RetrievalTool 用)
    void embedAsync(const QString &text);                  // C3 异步(信号 embeddingReady/Failed)

signals:
    void embeddingReady(const QVector<float> &vec);
    void embeddingFailed(const QString &err);

private:
    QString m_apiKey;
    HttpClient *m_http;
    static QVector<float> parseEmbedding(const QByteArray &response);
};

} // namespace glm

#endif // GLM_GLM_EMBEDDING_PROVIDER_H
