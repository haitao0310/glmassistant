#ifndef GLM_LOGGING_MIDDLEWARE_H
#define GLM_LOGGING_MIDDLEWARE_H

#include "IMiddleware.h"
#include "../infrastructure/Logger.h"

namespace glm {

// 日志中间件(扩展点 2 示例):记录请求概要(模型/消息数/参数/工具数)。
// 加 LoggingMiddleware 到 Pipeline 即生效,不动业务层。
class LoggingMiddleware : public IMiddleware
{
public:
    QString name() const override { return QStringLiteral("logging"); }
    LlmRequest process(LlmRequest req) override
    {
        logInfo("middleware", QStringLiteral("request: model=%1 msgs=%2 temp=%3 topP=%4 maxTok=%5 tools=%6")
                    .arg(req.model).arg(req.messages.size())
                    .arg(req.temperature).arg(req.topP)
                    .arg(req.maxTokens).arg(req.tools.size()));
        return req;
    }
};

} // namespace glm

#endif // GLM_LOGGING_MIDDLEWARE_H
