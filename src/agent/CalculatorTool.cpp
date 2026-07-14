#include "CalculatorTool.h"

#include <QJSEngine>
#include <QJSValue>
#include <QJsonArray>

namespace glm {

QString CalculatorTool::name() const
{
    return QStringLiteral("calculator");
}

QString CalculatorTool::description() const
{
    return QStringLiteral("计算数学表达式,如 1+2*3、Math.sqrt(16)、Math.PI*2");
}

QJsonObject CalculatorTool::schema() const
{
    return QJsonObject{
        {"type", QStringLiteral("object")},
        {"properties", QJsonObject{
            {"expression", QJsonObject{{"type", QStringLiteral("string")}, {"description", QStringLiteral("JS 数学表达式")}}}
        }},
        {"required", QJsonArray{QStringLiteral("expression")}}
    };
}

ToolResult CalculatorTool::invoke(const QJsonObject &args)
{
    const QString expr = args.value(QStringLiteral("expression")).toString();
    if (expr.isEmpty()) return ToolResult{QStringLiteral("空表达式"), true};
    QJSEngine engine;
    const QJSValue v = engine.evaluate(expr);
    if (v.isError()) return ToolResult{QStringLiteral("计算错误: %1").arg(expr), true};
    return ToolResult{v.toString(), false};
}

} // namespace glm
