#ifndef GLM_CACHING_PROVIDER_H
#define GLM_CACHING_PROVIDER_H

#include <QObject>
#include <QHash>
#include "../core/ILlmProvider.h"

namespace glm {

/**
 * 缓存 Provider(Decorator)。
 * 为 LLM Provider 增加响应缓存,命中跳过请求。
 */
class CachingProvider : public QObject, public ILlmProvider
{
    Q_OBJECT
public:
    // inner 不拥有,外部管理生命周期
    CachingProvider(ILlmProvider *inner, QObject *parent = nullptr);

    QString id() const override;
    QString displayName() const override;
    QStringList models() const override;

    /// 带缓存发送:优先返回缓存,未命中委托 inner 并缓存结果
    LlmReply *send(const LlmRequest &req) override;

    void clear();

private:
    ILlmProvider *m_inner;              // 不拥有
    QHash<QString, QString> m_cache;

    // stream/tools 不参与 key:相同语义请求共享缓存
    static QString cacheKey(const LlmRequest &req);
};

} // namespace glm

#endif // GLM_CACHING_PROVIDER_H
