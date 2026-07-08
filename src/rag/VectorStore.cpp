#include "VectorStore.h"

#include <algorithm>
#include <cmath>

namespace glm {

float VectorStore::cosine(const QVector<float> &a, const QVector<float> &b)
{
    if (a.size() != b.size() || a.isEmpty()) return 0.0f;
    float dot = 0.0f, na = 0.0f, nb = 0.0f;
    for (int i = 0; i < a.size(); ++i) {
        dot += a[i] * b[i];
        na  += a[i] * a[i];
        nb  += b[i] * b[i];
    }
    if (na == 0.0f || nb == 0.0f) return 0.0f;
    return dot / (std::sqrt(na) * std::sqrt(nb));
}

void VectorStore::add(const QString &id, const QString &text, const QVector<float> &vec)
{
    m_entries.append({id, text, vec});
}

QList<SearchResult> VectorStore::search(const QVector<float> &queryVec, int k) const
{
    QList<SearchResult> results;
    results.reserve(m_entries.size());
    for (const Entry &e : m_entries) {
        results.append({e.id, e.text, cosine(queryVec, e.vec)});
    }
    std::sort(results.begin(), results.end(),
        [](const SearchResult &a, const SearchResult &b) { return a.score > b.score; });
    if (results.size() > k) results = results.mid(0, k);
    return results;
}

void VectorStore::clear() { m_entries.clear(); }
int VectorStore::size() const { return m_entries.size(); }

} // namespace glm
