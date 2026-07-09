#ifndef GLM_IMIDDLEWARE_H
#define GLM_IMIDDLEWARE_H

#include <QString>
#include "../core/LlmTypes.h"

namespace glm {

// 中间件接口(扩展点 2):请求发出前可插拔处理。
// 用例:token 计算 / 上下文截断 / 日志 / 重试 / 缓存。
// Pipeline 按顺序调 process(req),返回处理后的 req。
class IMiddleware
{
public:
    virtual ~IMiddleware() = default;
    virtual QString name() const = 0;
    virtual LlmRequest process(LlmRequest req) = 0;   // 处理(可改 req 并返回)
};

} // namespace glm

#endif // GLM_IMIDDLEWARE_H
