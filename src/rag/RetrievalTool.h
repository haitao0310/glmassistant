#ifndef GLM_RETRIEVAL_TOOL_H
#define GLM_RETRIEVAL_TOOL_H

#include "../agent/ITool.h"
#include "../rag/IEmbeddingProvider.h"
#include "../rag/VectorStore.h"

namespace glm {

/**
 * RAG 检索工具:Agent 调用 → embed query → VectorStore 余弦检索 → 返回 top-k。
 * 实现在 RetrievalTool.cpp。
 */
class RetrievalTool : public ITool
{
public:
    RetrievalTool(IEmbeddingProvider *emb, VectorStore *store);

    QString name() const override;
    QString description() const override;
    QJsonObject schema() const override;
    ToolResult invoke(const QJsonObject &args) override;

private:
    IEmbeddingProvider *m_emb;
    VectorStore *m_store;
};

} // namespace glm

#endif // GLM_RETRIEVAL_TOOL_H
