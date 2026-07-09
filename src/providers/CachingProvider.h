#ifndef GLM_CACHING_PROVIDER_H
#define GLM_CACHING_PROVIDER_H

#include <QObject>
#include "../core/ILlmProvider.h"
#include "../core/LlmReply.h"
#include "../infrastructure/Logger.h"
#include <QHash>
#include <QString>

namespace glm {

// 缓存 Provider(Decorator 模式):包装真实 Provider,请求→响应缓存。
// 命中:跳过请求,延迟发 finished(异步感);未命中:真实请求 + finished 时存。
// C1 Cache 补强(扩展点 1 的包装层 + 中间件缓存的请求侧)。
class CachingProvider : public QObject, public ILlmProvider
{
    Q_OBJECT
public:
    CachingProvider(ILlmProvider *inner, QObject *parent = nullptr)
        : QObject(parent), m_inner(inner) {}

    QString id() const override { return m_inner ? m_inner->id() : QStringLiteral("cache"); }
    QString displayName() const override {
        return m_inner ? (m_inner->displayName() + QStringLiteral(" (cached)")) : QStringLiteral("Cache");
    }
    QStringList models() const override { return m_inner ? m_inner->models() : QStringList{}; }

    LlmReply *send(const LlmRequest &req) override
    {
        const QString key = cacheKey(req);
        // 命中:延迟发 finished(异步感,不阻塞调用方)
        if (m_cache.contains(key)) {
            auto *reply = new LlmReply();
            const QString cached = m_cache.value(key);
            reply->setRawRequest(QStringLiteral("[cache hit] ") + key);
            QMetaObject::invokeMethod(reply, [reply, cached]() {
                reply->emitFinished(cached);
            }, Qt::QueuedConnection);
            logInfo("cache", QStringLiteral("hit: %1").arg(key));
            return reply;
        }
        // 未命中:真实请求 + finished 时缓存
        LlmReply *inner = m_inner ? m_inner->send(req) : nullptr;
        if (inner) {
            QObject::connect(inner, &LlmReply::finished, this, [this, key](const QString &full) {
                m_cache.insert(key, full);
                logInfo("cache", QStringLiteral("stored: %1").arg(key));
            });
        }
        return inner;
    }

    void clear() { m_cache.clear(); }

private:
    ILlmProvider *m_inner;
    QHash<QString, QString> m_cache;   // key → 响应全文

    static QString cacheKey(const LlmRequest &req)
    {
        // key:model + messages 内容 + 关键参数(stream/tools 不进 key,纯语义)
        QString k = req.model + QStringLiteral("|");
        for (const Message &m : req.messages) {
            k += m.roleName() + QStringLiteral(":") + m.content + QStringLiteral(";");
        }
        k += QStringLiteral("|t%1|p%2|m%3").arg(req.temperature).arg(req.topP).arg(req.maxTokens);
        return QString::number(qHash(k));
    }
};

} // namespace glm

#endif // GLM_CACHING_PROVIDER_H
