#ifndef GLM_TOKEN_COUNTER_MIDDLEWARE_H
#define GLM_TOKEN_COUNTER_MIDDLEWARE_H

#include "IMiddleware.h"
#include "../infrastructure/Logger.h"

namespace glm {

// token 计数中间件:估算 messages 总 token(粗估 char/4),写日志。
// (P2 token 统计 AC,这里补;精确计数可接 tiktoken 移植)
class TokenCounterMiddleware : public IMiddleware
{
public:
    QString name() const override { return QStringLiteral("token_counter"); }
    LlmRequest process(LlmRequest req) override
    {
        int total = 0;
        for (const Message &m : req.messages) total += estimate(m.content);
        logInfo("middleware", QStringLiteral("tokens ~ %1 (msgs=%2)").arg(total).arg(req.messages.size()));
        return req;
    }
    static int estimate(const QString &s) { return s.size() / 4; }   // 粗估(中文按 char)
};

} // namespace glm

#endif // GLM_TOKEN_COUNTER_MIDDLEWARE_H
