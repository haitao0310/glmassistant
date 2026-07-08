#include "ThemeManager.h"

#include "../infrastructure/Logger.h"

#include <QApplication>

namespace glm {

namespace {
ThemeManager::Theme g_current = ThemeManager::Theme::Light;

const char *LIGHT_QSS = R"(
QWidget { background: #f5f5f5; color: #1a1a1a; font-size: 13px; }
QListView, QTextEdit, QTextBrowser { background: #ffffff; border: 1px solid #d0d0d0; }
QPushButton { background: #e0e0e0; border: 1px solid #b0b0b0; padding: 5px 14px; border-radius: 3px; }
QPushButton:hover { background: #d0d0d0; }
QPushButton:disabled { color: #999; }
QLabel { background: transparent; }
QComboBox, QSpinBox, QDoubleSpinBox { background: #ffffff; border: 1px solid #b0b0b0; padding: 2px; }
)";

const char *DARK_QSS = R"(
QWidget { background: #2b2b2b; color: #d4d4d4; font-size: 13px; }
QListView, QTextEdit, QTextBrowser { background: #1e1e1e; color: #d4d4d4; border: 1px solid #3c3c3c; }
QPushButton { background: #3c3c3c; color: #d4d4d4; border: 1px solid #555; padding: 5px 14px; border-radius: 3px; }
QPushButton:hover { background: #4a4a4a; }
QPushButton:disabled { color: #666; }
QLabel { background: transparent; }
QComboBox, QSpinBox, QDoubleSpinBox { background: #3c3c3c; color: #d4d4d4; border: 1px solid #555; padding: 2px; }
)";
} // namespace

ThemeManager::Theme ThemeManager::current() { return g_current; }

void ThemeManager::apply(Theme t, QApplication *app)
{
    g_current = t;
    app->setStyleSheet(QString::fromLatin1(t == Theme::Dark ? DARK_QSS : LIGHT_QSS));
    logInfo("theme", t == Theme::Dark ? QStringLiteral("dark applied") : QStringLiteral("light applied"));
}

} // namespace glm
