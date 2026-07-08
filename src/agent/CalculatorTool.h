#ifndef GLM_CALCULATOR_TOOL_H
#define GLM_CALCULATOR_TOOL_H

#include "ITool.h"
#include <QJSEngine>
#include <QJSValue>
#include <QJsonArray>

namespace glm {

// 示例工具:计算 JS 数学表达式(QJSEngine 真算,支持 + - * / Math.* 等)
class CalculatorTool : public ITool
{
public:
    QString name() const override { return QStringLiteral("calculator"); }
    QString description() const override { return QStringLiteral("计算数学表达式,如 1+2*3、Math.sqrt(16)、Math.PI*2"); }
    QJsonObject schema() const override
    {
        return QJsonObject{
            {"type", QStringLiteral("object")},
            {"properties", QJsonObject{
                {"expression", QJsonObject{{"type", QStringLiteral("string")}, {"description", QStringLiteral("JS 数学表达式")}}}
            }},
            {"required", QJsonArray{QStringLiteral("expression")}}
        };
    }
    ToolResult invoke(const QJsonObject &args) override
    {
        const QString expr = args.value(QStringLiteral("expression")).toString();
        if (expr.isEmpty()) return ToolResult{QStringLiteral("空表达式"), true};
        QJSEngine engine;
        const QJSValue v = engine.evaluate(expr);
        ToolResult r;
        if (v.isError()) {
            r.isError = true;
            r.content = QStringLiteral("计算错误: %1").arg(expr);
        } else {
            r.content = v.toString();
        }
        return r;
    }
};

} // namespace glm

#endif // GLM_CALCULATOR_TOOL_H
