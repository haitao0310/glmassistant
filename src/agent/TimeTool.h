#ifndef GLM_TIME_TOOL_H
#define GLM_TIME_TOOL_H

#include "ITool.h"
#include <QDateTime>
#include <QJsonArray>

namespace glm {

// 示例工具:获取当前时间(无外部依赖,演示 function calling 流程)
class TimeTool : public ITool
{
public:
    QString name() const override { return QStringLiteral("current_time"); }
    QString description() const override { return QStringLiteral("获取当前日期和时间"); }
    QJsonObject schema() const override
    {
        return QJsonObject{
            {"type", QStringLiteral("object")},
            {"properties", QJsonObject{}}
        };
    }
    ToolResult invoke(const QJsonObject &) override
    {
        return ToolResult{
            QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")),
            false
        };
    }
};

} // namespace glm

#endif // GLM_TIME_TOOL_H
