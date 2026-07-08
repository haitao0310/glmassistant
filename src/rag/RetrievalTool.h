#ifndef GLM_RETRIEVAL_TOOL_H
#define GLM_RETRIEVAL_TOOL_H

#include "../agent/ITool.h"
#include "../rag/IEmbeddingProvider.h"
#include "../rag/VectorStore.h"
#include <QJsonArray>

namespace glm {

// RAG 检索工具(P5):Agent 调用 → embed query → VectorStore 余弦检索 → 返回 top-k 知识。
// 把私有知识库接入 Agent(扩展点 3 的 RAG 应用)。
class RetrievalTool : public ITool
{
public:
    RetrievalTool(IEmbeddingProvider *emb, VectorStore *store)
        : m_emb(emb), m_store(store) {}

    QString name() const override { return QStringLiteral("knowledge_search"); }
    QString description() const override { return QStringLiteral("从知识库检索相关知识片段"); }
    QJsonObject schema() const override
    {
        return QJsonObject{
            {"type", QStringLiteral("object")},
            {"properties", QJsonObject{
                {"query", QJsonObject{{"type", QStringLiteral("string")}, {"description", QStringLiteral("检索查询")}}}
            }},
            {"required", QJsonArray{QStringLiteral("query")}}
        };
    }
    ToolResult invoke(const QJsonObject &args) override
    {
        const QString query = args.value(QStringLiteral("query")).toString();
        if (query.isEmpty() || !m_emb || !m_store) return ToolResult{QStringLiteral("无查询/未配置"), true};
        const QVector<float> vec = m_emb->embed(query);
        if (vec.isEmpty()) return ToolResult{QStringLiteral("embedding 失败"), true};
        const QList<SearchResult> results = m_store->search(vec, 3);
        QString content;
        for (const SearchResult &r : results) {
            content += QStringLiteral("[%1] %2\n---\n").arg(r.score, 0, 'f', 2).arg(r.text);
        }
        return ToolResult{content.isEmpty() ? QStringLiteral("无相关知识") : content, false};
    }

private:
    IEmbeddingProvider *m_emb;
    VectorStore *m_store;
};

} // namespace glm

#endif // GLM_RETRIEVAL_TOOL_H
