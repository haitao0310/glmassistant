#include "MiddlewarePipeline.h"

#include "../infrastructure/Logger.h"

namespace glm {

void MiddlewarePipeline::add(IMiddleware *m)
{
    if (m) {
        m_chain.append(m);
        logInfo("middleware", QStringLiteral("added %1 (chain=%2)").arg(m->name()).arg(m_chain.size()));
    }
}

LlmRequest MiddlewarePipeline::process(LlmRequest req) const
{
    for (IMiddleware *m : m_chain) {
        req = m->process(std::move(req));
    }
    return req;
}

} // namespace glm
