#ifndef GLM_IEMBEDDING_PROVIDER_H
#define GLM_IEMBEDDING_PROVIDER_H

#include <QVector>
#include <QString>

namespace glm {

// Embedding 接口(RAG 用,扩展点)。text → 向量。
// 实现:GlmEmbeddingProvider(embedding-3 API)、Ollama embedding 等。
// P5 用同步签名(简化);异步可参考 LlmReply 信号模式。
class IEmbeddingProvider
{
public:
    virtual ~IEmbeddingProvider() = default;
    virtual QVector<float> embed(const QString &text) = 0;
};

} // namespace glm

#endif // GLM_IEMBEDDING_PROVIDER_H
