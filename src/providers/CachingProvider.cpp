#include "CachingProvider.h"

#include "../core/LlmReply.h"
#include "../infrastructure/Logger.h"

#include <QMetaObject>

namespace glm {

CachingProvider::CachingProvider(ILlmProvider *inner, QObject *parent)
    : QObject(parent)
    , m_inner(inner)
{
}

QString CachingProvider::id() const
{
    return m_inner ? m_inner->id() : QStringLiteral("cache");
}

QString CachingProvider::displayName() const
{
    return m_inner ? (m_inner->displayName() + QStringLiteral(" (cached)")) : QStringLiteral("Cache");
}

QStringList CachingProvider::models() const
{
    return m_inner ? m_inner->models() : QStringList{};
}

LlmReply *CachingProvider::send(const LlmRequest &req)
{
    const QString key = cacheKey(req);
    if (m_cache.contains(key)) {
        auto *reply = new LlmReply();
        const QString cached = m_cache.value(key);
        reply->setRawRequest(QStringLiteral("[cache hit] ") + key);
        // QueuedConnection:延迟到事件循环发,保持异步语义(和真实请求一样不阻塞调用方)
        QMetaObject::invokeMethod(reply, [reply, cached]() {
            reply->emitFinished(cached);
        }, Qt::QueuedConnection);
        logInfo("cache", QStringLiteral("hit: %1").arg(key));
        return reply;
    }
    LlmReply *inner = m_inner ? m_inner->send(req) : nullptr;
    if (inner) {
        QObject::connect(inner, &LlmReply::finished, this, [this, key](const QString &full) {
            m_cache.insert(key, full);
            logInfo("cache", QStringLiteral("stored: %1").arg(key));
        });
    }
    return inner;
}

void CachingProvider::clear()
{
    m_cache.clear();
}

QString CachingProvider::cacheKey(const LlmRequest &req)
{
    QString k = req.model + QStringLiteral("|");
    for (const Message &m : req.messages) {
        k += m.roleName() + QStringLiteral(":") + m.content + QStringLiteral(";");
    }
    k += QStringLiteral("|t%1|p%2|m%3").arg(req.temperature).arg(req.topP).arg(req.maxTokens);
    return QString::number(qHash(k));
}

} // namespace glm
