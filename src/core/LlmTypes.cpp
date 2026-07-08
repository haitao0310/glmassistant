#include "LlmTypes.h"

namespace glm {

QString Message::roleName() const
{
    switch (role) {
    case Role::System:    return QStringLiteral("system");
    case Role::User:      return QStringLiteral("user");
    case Role::Assistant: return QStringLiteral("assistant");
    case Role::Tool:      return QStringLiteral("tool");
    }
    return QStringLiteral("user");
}

Role Message::roleFromName(const QString &name)
{
    const QString n = name.toLower();
    if (n == QStringLiteral("system"))    return Role::System;
    if (n == QStringLiteral("assistant")) return Role::Assistant;
    if (n == QStringLiteral("tool"))      return Role::Tool;
    return Role::User;
}

} // namespace glm
