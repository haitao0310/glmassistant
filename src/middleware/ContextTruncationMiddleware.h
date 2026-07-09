#ifndef GLM_CONTEXT_TRUNCATION_MIDDLEWARE_H
#define GLM_CONTEXT_TRUNCATION_MIDDLEWARE_H

#include "IMiddleware.h"
#include "TokenCounterMiddleware.h"
#include "../infrastructure/Logger.h"

namespace glm {

// 上下文截断中间件:总 token 超 budget 时,删旧消息(留首条 system + 最近对话)。
// 防请求超 model 上下文窗口。
class ContextTruncationMiddleware : public IMiddleware
{
public:
    explicit ContextTruncationMiddleware(int budget = 6000) : m_budget(budget) {}
    QString name() const override { return QStringLiteral("context_truncation"); }

    LlmRequest process(LlmRequest req) override
    {
        int total = totalTokens(req);
        while (total > m_budget && req.messages.size() > 2) {
            // 删第二条(保留 index 0 通常是 system + 末尾 user/assistant)
            req.messages.removeAt(1);
            total = totalTokens(req);
        }
        if (total > m_budget) {
            logWarning("middleware", QStringLiteral("truncated to %1 tokens still > %2 budget")
                .arg(total).arg(m_budget));
        }
        return req;
    }

private:
    int m_budget;
    static int totalTokens(const LlmRequest &req)
    {
        int t = 0;
        for (const Message &m : req.messages) t += TokenCounterMiddleware::estimate(m.content);
        return t;
    }
};

} // namespace glm

#endif // GLM_CONTEXT_TRUNCATION_MIDDLEWARE_H
