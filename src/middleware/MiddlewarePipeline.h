#ifndef GLM_MIDDLEWARE_PIPELINE_H
#define GLM_MIDDLEWARE_PIPELINE_H

#include <QList>
#include <QString>
#include "../middleware/IMiddleware.h"

namespace glm {

// 中间件责任链:按添加顺序依次 process。
// 加新中间件:add(m),不动调用方(ChatController/LlmService)。
class MiddlewarePipeline
{
public:
    void add(IMiddleware *m);
    LlmRequest process(LlmRequest req) const;
    QList<IMiddleware *> middlewares() const { return m_chain; }

private:
    QList<IMiddleware *> m_chain;
};

} // namespace glm

#endif // GLM_MIDDLEWARE_PIPELINE_H
