#include "LlmTypes.h"

namespace glm {

// 序列化用(发给 GLM API 的 role 字段)
QString Message::roleName() const
{
    switch (role) {
    case Role::System:    return QStringLiteral("system");
    case Role::User:      return QStringLiteral("user");
    case Role::Assistant: return QStringLiteral("assistant");
    }
    return QStringLiteral("user");
}

// 反序列化(宽容:未知角色当 user,不崩)
Role Message::roleFromName(const QString &name)
{
    const QString n = name.toLower();
    if (n == QStringLiteral("system"))    return Role::System;
    if (n == QStringLiteral("assistant")) return Role::Assistant;
    return Role::User;
}

} // namespace glm
