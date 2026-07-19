#include "SettingsManager.h"

#include <QSettings>
#include "../infrastructure/Constants.h"

namespace glm {

static const char *kKeyModel = "params/model";
static const char *kKeyTemp = "params/temperature";
static const char *kKeyTopP = "params/topP";
static const char *kKeyMaxTok = "params/maxTokens";
static const char *kKeyTheme = "ui/theme";
static const char *kKeyWindow = "ui/windowGeometry";
static const char *kKeySession = "session/lastId";

SettingsManager &SettingsManager::instance()
{
    static SettingsManager inst;
    return inst;
}

GenerationParams SettingsManager::params() const
{
    QSettings s;
    GenerationParams p;
    p.model = s.value(kKeyModel, constants::GLM_DEFAULT_MODEL).toString();
    p.temperature = s.value(kKeyTemp, 0.7).toDouble();
    p.topP = s.value(kKeyTopP, 1.0).toDouble();
    p.maxTokens = s.value(kKeyMaxTok, 2048).toInt();
    return p;
}

void SettingsManager::saveParams(const GenerationParams &p)
{
    QSettings s;
    s.setValue(kKeyModel, p.model);
    s.setValue(kKeyTemp, p.temperature);
    s.setValue(kKeyTopP, p.topP);
    s.setValue(kKeyMaxTok, p.maxTokens);
}

ThemeManager::Theme SettingsManager::theme() const
{
    QSettings s;
    return static_cast<ThemeManager::Theme>(s.value(kKeyTheme, static_cast<int>(ThemeManager::Theme::Light)).toInt());
}

void SettingsManager::saveTheme(ThemeManager::Theme t)
{
    QSettings s;
    s.setValue(kKeyTheme, static_cast<int>(t));
}

QByteArray SettingsManager::windowGeometry() const
{
    QSettings s;
    return s.value(kKeyWindow).toByteArray();
}

void SettingsManager::saveWindowGeometry(const QByteArray &geo)
{
    QSettings s;
    s.setValue(kKeyWindow, geo);
}

QString SettingsManager::lastSessionId() const
{
    QSettings s;
    return s.value(kKeySession).toString();
}

void SettingsManager::saveLastSessionId(const QString &id)
{
    QSettings s;
    s.setValue(kKeySession, id);
}

} // namespace glm
