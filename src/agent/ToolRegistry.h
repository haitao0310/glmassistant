#ifndef GLM_TOOL_REGISTRY_H
#define GLM_TOOL_REGISTRY_H

#include <QMap>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include "../agent/ITool.h"

namespace glm {

// 工具注册中心(单例)。AgentController 据此查工具 + 给 LLM 提供 schemas。
// 不持有工具所有权(外部管理生命周期),P5 可换 QPluginLoader 动态加载。
class ToolRegistry
{
public:
    static ToolRegistry &instance();

    void registerTool(ITool *tool);                 // 注册(外部管所有权)
    ITool *tool(const QString &name) const;          // 按名查
    QList<ITool *> tools() const;                    // 全部
    QJsonArray schemas() const;              // 全部 schema(OpenAI tools 字段)

private:
    ToolRegistry() = default;
    QMap<QString, ITool *> m_tools;
};

} // namespace glm

#endif // GLM_TOOL_REGISTRY_H
