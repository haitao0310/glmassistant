#include "AgentController.h"

#include "ToolRegistry.h"
#include "../infrastructure/Logger.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

namespace glm {

AgentController::AgentController(ILlmProvider *provider, QObject *parent)
    : QObject(parent)
    , m_provider(provider)
{
}

LlmReply *AgentController::run(LlmRequest req)
{
    auto *reply = new LlmReply();
    if (req.tools.isEmpty()) {
        req.tools = ToolRegistry::instance().schemas();   // 默认全部已注册工具
    }
    req.stream = false;   // Agent 非流式(简化 tool_calls 解析)
    runStep(req, reply, 0);
    return reply;
}

// 循环核心:发请求 → 看 tool_calls → 执行工具 → 回填 → 再请求
void AgentController::runStep(LlmRequest req, LlmReply *finalReply, int depth)
{
    if (depth > kMaxDepth) {
        finalReply->emitError(QStringLiteral("agent 超过最大循环深度 %1").arg(kMaxDepth));
        return;
    }
    if (!m_provider) { finalReply->emitError(QStringLiteral("无 provider")); return; }

    LlmReply *step = m_provider->send(req);
    step->setParent(finalReply);   // 跟随 finalReply 生命周期

    QObject::connect(step, &LlmReply::errorOccurred, finalReply, [finalReply](const QString &e){
        finalReply->emitError(e);
    });

    QObject::connect(step, &LlmReply::finished, finalReply,
        [this, req, finalReply, step, depth](const QString &content) {
            const auto toolCalls = parseToolCalls(step->rawResponse());
            if (toolCalls.isEmpty()) {
                finalReply->emitFinished(content);   // 无 tool_calls:最终回复
                return;
            }
            // 有 tool_calls:执行 + 回填 Tool 消息 + 循环
            LlmRequest next = req;
            Message asstMsg;
            asstMsg.role = Role::Assistant;
            asstMsg.content = content;
            asstMsg.timestamp = QDateTime::currentMSecsSinceEpoch();
            next.messages.append(asstMsg);

            for (const ToolCall &tc : toolCalls) {
                ITool *tool = ToolRegistry::instance().tool(tc.name);
                Message toolMsg;
                toolMsg.role = Role::Tool;
                toolMsg.timestamp = QDateTime::currentMSecsSinceEpoch();
                if (tool) {
                    const ToolResult result = tool->invoke(parseArgs(tc.arguments));
                    toolMsg.content = result.content;
                    logInfo("agent", QStringLiteral("tool %1 → %2").arg(tc.name, result.content.left(80)));
                } else {
                    toolMsg.content = QStringLiteral("工具 %1 不存在").arg(tc.name);
                    logWarning("agent", QStringLiteral("unknown tool: %1").arg(tc.name));
                }
                next.messages.append(toolMsg);
            }
            runStep(next, finalReply, depth + 1);   // 循环
        });
}

// 从 GLM 响应解析 tool_calls(非流式 message.tool_calls)
QList<ToolCall> AgentController::parseToolCalls(const QString &rawResponse) const
{
    QList<ToolCall> result;
    if (rawResponse.isEmpty()) return result;
    const QJsonDocument doc = QJsonDocument::fromJson(rawResponse.toUtf8());
    if (!doc.isObject()) return result;
    const QJsonArray choices = doc.object().value("choices").toArray();
    if (choices.isEmpty()) return result;
    const QJsonObject msg = choices.at(0).toObject().value("message").toObject();
    const QJsonArray calls = msg.value("tool_calls").toArray();
    for (const QJsonValue &v : calls) {
        const QJsonObject tc = v.toObject();
        ToolCall call;
        call.id = tc.value("id").toString();
        const QJsonObject fn = tc.value("function").toObject();
        call.name = fn.value("name").toString();
        call.arguments = fn.value("arguments").toString();
        if (!call.name.isEmpty()) result.append(call);
    }
    return result;
}

QJsonObject AgentController::parseArgs(const QString &arguments) const
{
    const QJsonDocument doc = QJsonDocument::fromJson(arguments.toUtf8());
    return doc.object();
}

} // namespace glm
