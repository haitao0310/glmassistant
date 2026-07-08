#ifndef GLM_THEME_MANAGER_H
#define GLM_THEME_MANAGER_H

#include <QObject>

class QApplication;

namespace glm {

// 主题管理(QSS 深色/浅色切换)。
class ThemeManager : public QObject
{
    Q_OBJECT
public:
    enum class Theme { Light, Dark };
    Q_ENUM(Theme)

    static Theme current();
    static void apply(Theme t, QApplication *app);   // 应用 QSS 到全局
};

} // namespace glm

#endif // GLM_THEME_MANAGER_H
