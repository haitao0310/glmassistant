#include "RetrievalTool.h"

#include <QJsonArray>

namespace glm {

RetrievalTool::RetrievalTool(IEmbeddingProvider *emb, VectorStore *store)
    : m_emb(emb)
    , m_store(store)
{
}

QString RetrievalTool::name() const
{
    return QStringLiteral("knowledge_search");
}

QString RetrievalTool::description() const
{
    return QStringLiteral("从知识库检索相关知识片段");
}

QJsonObject RetrievalTool::schema() const
{
    return QJsonObject{
        {"type", QStringLiteral("object")},
        {"properties", QJsonObject{
            {"query", QJsonObject{{"type", QStringLiteral("string")}, {"description", QStringLiteral("检索查询")}}}
        }},
        {"required", QJsonArray{QStringLiteral("query")}}
    };
}

ToolResult RetrievalTool::invoke(const QJsonObject &args)
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

} // namespace glm
