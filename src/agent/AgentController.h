#ifndef GLM_AGENT_CONTROLLER_H
#define GLM_AGENT_CONTROLLER_H

#include <QObject>
#include "../core/ILlmProvider.h"
#include "../core/LlmTypes.h"
#include "../core/LlmReply.h"
#include "../agent/ITool.h"

namespace glm {

// Agent 控制器(P5):LLM tool_calls 循环(扩展点 3 的驱动者)。
//
// run(req) 发带 tools 的请求;响应有 tool_calls → 查工具执行 → 结果回填 messages
// → 再请求,循环到无 tool_call(或达深度上限)。
// 用非流式(简化 tool_calls 解析),最终文本经 LlmReply::finished 发出。
class AgentController : public QObject
{
    Q_OBJECT
public:
    AgentController(ILlmProvider *provider, QObject *parent = nullptr);

    LlmReply *run(LlmRequest req);   // req.tools 空时用 ToolRegistry 全部

private:
    ILlmProvider *m_provider;

    void runStep(LlmRequest req, LlmReply *finalReply, int depth);
    QList<ToolCall> parseToolCalls(const QString &rawResponse) const;
    QJsonObject parseArgs(const QString &arguments) const;

    static constexpr int kMaxDepth = 8;   // 防无限循环(防玩具 DoD)
};

} // namespace glm

#endif // GLM_AGENT_CONTROLLER_H
