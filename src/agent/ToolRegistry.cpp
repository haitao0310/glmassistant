#include "ToolRegistry.h"

namespace glm {

ToolRegistry &ToolRegistry::instance()
{
    static ToolRegistry inst;
    return inst;
}

void ToolRegistry::registerTool(ITool *tool)
{
    if (tool) m_tools.insert(tool->name(), tool);
}

ITool *ToolRegistry::tool(const QString &name) const
{
    return m_tools.value(name, nullptr);
}

QList<ITool *> ToolRegistry::tools() const
{
    return m_tools.values();
}

// 转成 OpenAI function calling 的 tools 字段格式(QJsonArray,直接给 LlmRequest.tools)
QJsonArray ToolRegistry::schemas() const
{
    QJsonArray result;
    for (ITool *t : m_tools) {
        QJsonObject obj;
        obj["type"] = QStringLiteral("function");
        QJsonObject fn;
        fn["name"] = t->name();
        fn["description"] = t->description();
        fn["parameters"] = t->schema();
        obj["function"] = fn;
        result.append(obj);
    }
    return result;
}

} // namespace glm
