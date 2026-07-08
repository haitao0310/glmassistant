#ifndef GLM_ITOOL_H
#define GLM_ITOOL_H

#include <QString>
#include <QJsonObject>

namespace glm {

// Agent 工具调用描述(LLM 返回的 tool_call)
struct ToolCall {
    QString id;            // GLM tool_call id
    QString name;          // 工具名
    QString arguments;     // JSON 字符串参数
};

// 工具执行结果(回填给 LLM,LLM 据此继续生成)
struct ToolResult {
    QString content;
    bool isError = false;
};

// Agent 工具接口(扩展点 3)。
// 加新工具:实现 ITool + 注册 ToolRegistry,LLM 决策调用。
// 示例:计算器/时间/搜索/RAG 检索/代码执行。
class ITool
{
public:
    virtual ~ITool() = default;
    virtual QString name() const = 0;             // 工具名(LLM 识别)
    virtual QString description() const = 0;       // 描述(LLM 知道何时用)
    virtual QJsonObject schema() const = 0;        // 参数 JSON schema(OpenAI function calling)
    virtual ToolResult invoke(const QJsonObject &args) = 0;   // 执行
};

} // namespace glm

#endif // GLM_ITOOL_H
