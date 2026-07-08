#ifndef GLM_VECTOR_STORE_H
#define GLM_VECTOR_STORE_H

#include <QVector>
#include <QList>
#include <QString>

namespace glm {

// 向量检索结果(id + 原文 + 余弦分数)
struct SearchResult {
    QString id;
    QString text;
    float score = 0.0f;
};

// 内存向量存储 + 余弦相似度检索(RAG 简化,P5)。
// 进程内不持久化;后续可扩展 SQLite 向量或专用向量库(Faiss/Milvus)。
class VectorStore
{
public:
    void add(const QString &id, const QString &text, const QVector<float> &vec);
    QList<SearchResult> search(const QVector<float> &queryVec, int k = 3) const;   // top-k
    void clear();
    int size() const;

private:
    struct Entry { QString id; QString text; QVector<float> vec; };
    QList<Entry> m_entries;
    static float cosine(const QVector<float> &a, const QVector<float> &b);
};

} // namespace glm

#endif // GLM_VECTOR_STORE_H
