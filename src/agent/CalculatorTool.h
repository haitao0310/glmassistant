#ifndef GLM_CALCULATOR_TOOL_H
#define GLM_CALCULATOR_TOOL_H

#include "ITool.h"

namespace glm {

/**
 * 计算器工具:QJSEngine 执行 JS 数学表达式。
 * 实现在 CalculatorTool.cpp。
 */
class CalculatorTool : public ITool
{
public:
    QString name() const override;
    QString description() const override;
    QJsonObject schema() const override;
    ToolResult invoke(const QJsonObject &args) override;
};

} // namespace glm

#endif // GLM_CALCULATOR_TOOL_H
