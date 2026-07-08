#ifndef GLM_PROVIDER_REGISTRY_H
#define GLM_PROVIDER_REGISTRY_H

#include <QMap>
#include <QList>
#include <QStringList>
#include "../core/ILlmProvider.h"

namespace glm {

// Provider 注册中心(扩展点 1 的工厂)。多 Provider 按 id 选。
// P5:GlmProvider + OllamaProvider + OpenAiProvider 各注册到此,业务层按 providerId 取。
class ProviderRegistry
{
public:
    static ProviderRegistry &instance();

    void registerProvider(ILlmProvider *p);     // 注册(外部管所有权)
    ILlmProvider *provider(const QString &id) const;
    QList<ILlmProvider *> providers() const;
    QStringList ids() const;

private:
    ProviderRegistry() = default;
    QMap<QString, ILlmProvider *> m_providers;
};

} // namespace glm

#endif // GLM_PROVIDER_REGISTRY_H
