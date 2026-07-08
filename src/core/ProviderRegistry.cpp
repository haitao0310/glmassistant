#include "ProviderRegistry.h"

namespace glm {

ProviderRegistry &ProviderRegistry::instance()
{
    static ProviderRegistry inst;
    return inst;
}

void ProviderRegistry::registerProvider(ILlmProvider *p)
{
    if (p) m_providers.insert(p->id(), p);
}

ILlmProvider *ProviderRegistry::provider(const QString &id) const
{
    return m_providers.value(id, nullptr);
}

QList<ILlmProvider *> ProviderRegistry::providers() const
{
    return m_providers.values();
}

QStringList ProviderRegistry::ids() const
{
    return m_providers.keys();
}

} // namespace glm
