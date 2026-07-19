#ifndef GLM_SETTINGS_MANAGER_H
#define GLM_SETTINGS_MANAGER_H

#include <QByteArray>
#include <QString>
#include "../core/LlmTypes.h"
#include "../infrastructure/ThemeManager.h"

namespace glm {

/**
 * 配置持久化(封装 QSettings)。
 * 自动存 %APPDATA%/GlmAssistant/GlmAssistant.ini,跨平台。
 * 重启后恢复 model/params/theme/window/session。
 */
class SettingsManager
{
public:
    static SettingsManager &instance();

    // 生成参数(model/temperature/topP/maxTokens)
    GenerationParams params() const;
    void saveParams(const GenerationParams &p);

    // 主题
    ThemeManager::Theme theme() const;
    void saveTheme(ThemeManager::Theme t);

    // 窗口几何(大小+位置)
    QByteArray windowGeometry() const;
    void saveWindowGeometry(const QByteArray &geo);

    // 最后使用的会话
    QString lastSessionId() const;
    void saveLastSessionId(const QString &id);

private:
    SettingsManager() = default;
};

} // namespace glm

#endif // GLM_SETTINGS_MANAGER_H
