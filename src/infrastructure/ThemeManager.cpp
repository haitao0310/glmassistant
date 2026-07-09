#include "ThemeManager.h"

#include "../infrastructure/Logger.h"

#include <QApplication>

namespace glm {

namespace {
ThemeManager::Theme g_current = ThemeManager::Theme::Light;

// 浅色:主色 #2563eb(蓝) + 背景 #f8f9fa + 文字 #1e293b + 圆角 6px
const char *LIGHT_QSS = R"(
* { font-family: 'Segoe UI','Microsoft YaHei',sans-serif; font-size: 13px; color: #1e293b; }
QMainWindow, QDialog { background: #f8f9fa; }

QListWidget {
    background: #ffffff; border: 1px solid #e2e8f0; border-radius: 6px;
    padding: 4px; outline: none;
}
QListWidget::item { padding: 8px 10px; border-radius: 4px; }
QListWidget::item:hover { background: #f1f5f9; }
QListWidget::item:selected { background: #2563eb; color: #ffffff; }

QTextBrowser, QTextEdit, QPlainTextEdit {
    background: #ffffff; border: 1px solid #e2e8f0; border-radius: 6px;
    padding: 8px; selection-background-color: #bfdbfe;
}
QTextBrowser:focus, QTextEdit:focus, QPlainTextEdit:focus { border-color: #2563eb; }

QPushButton {
    background: #ffffff; border: 1px solid #cbd5e1; border-radius: 6px;
    padding: 6px 16px; color: #1e293b;
}
QPushButton:hover { background: #f1f5f9; border-color: #94a3b8; }
QPushButton:pressed { background: #e2e8f0; }
QPushButton:disabled { color: #94a3b8; background: #f1f5f9; }

/* 主按钮(发送/停止,用 objectName=sendButton 区分) */
QPushButton#sendButton {
    background: #2563eb; border: none; color: #ffffff; font-weight: 500;
}
QPushButton#sendButton:hover { background: #1d4ed8; }
QPushButton#sendButton:pressed { background: #1e40af; }

QComboBox, QSpinBox, QDoubleSpinBox {
    background: #ffffff; border: 1px solid #cbd5e1; border-radius: 6px;
    padding: 4px 8px; min-height: 20px;
}
QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus { border-color: #2563eb; }
QComboBox::drop-down { border: none; width: 20px; }
QComboBox QAbstractItemView {
    background: #ffffff; border: 1px solid #cbd5e1;
    selection-background-color: #2563eb; selection-color: #ffffff;
}

QTabWidget::pane { border: 1px solid #e2e8f0; border-radius: 6px; top: -1px; }
QTabBar::tab {
    background: #f1f5f9; border: 1px solid #e2e8f0; border-bottom: none;
    padding: 6px 16px; border-top-left-radius: 6px; border-top-right-radius: 6px; margin-right: 2px;
}
QTabBar::tab:selected { background: #ffffff; color: #2563eb; }
QTabBar::tab:hover:!selected { background: #e2e8f0; }

QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }
QScrollBar::handle:vertical { background: #cbd5e1; border-radius: 5px; min-height: 30px; }
QScrollBar::handle:vertical:hover { background: #94a3b8; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

QLabel { background: transparent; }
)";

// 深色:主色 #3b82f6 + 背景 #1e293b + 文字 #e2e8f0
const char *DARK_QSS = R"(
* { font-family: 'Segoe UI','Microsoft YaHei',sans-serif; font-size: 13px; color: #e2e8f0; }
QMainWindow, QDialog { background: #1e293b; }

QListWidget {
    background: #0f172a; border: 1px solid #334155; border-radius: 6px;
    padding: 4px; outline: none;
}
QListWidget::item { padding: 8px 10px; border-radius: 4px; }
QListWidget::item:hover { background: #1e293b; }
QListWidget::item:selected { background: #3b82f6; color: #ffffff; }

QTextBrowser, QTextEdit, QPlainTextEdit {
    background: #0f172a; color: #e2e8f0; border: 1px solid #334155; border-radius: 6px;
    padding: 8px; selection-background-color: #1d4ed8;
}
QTextBrowser:focus, QTextEdit:focus, QPlainTextEdit:focus { border-color: #3b82f6; }

QPushButton {
    background: #334155; color: #e2e8f0; border: 1px solid #475569; border-radius: 6px;
    padding: 6px 16px;
}
QPushButton:hover { background: #475569; border-color: #64748b; }
QPushButton:pressed { background: #475569; }
QPushButton:disabled { color: #64748b; background: #1e293b; }

QPushButton#sendButton {
    background: #3b82f6; border: none; color: #ffffff; font-weight: 500;
}
QPushButton#sendButton:hover { background: #2563eb; }
QPushButton#sendButton:pressed { background: #1d4ed8; }

QComboBox, QSpinBox, QDoubleSpinBox {
    background: #0f172a; color: #e2e8f0; border: 1px solid #475569; border-radius: 6px;
    padding: 4px 8px; min-height: 20px;
}
QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus { border-color: #3b82f6; }
QComboBox::drop-down { border: none; width: 20px; }
QComboBox QAbstractItemView {
    background: #0f172a; color: #e2e8f0; border: 1px solid #475569;
    selection-background-color: #3b82f6;
}

QTabWidget::pane { border: 1px solid #334155; border-radius: 6px; top: -1px; }
QTabBar::tab {
    background: #0f172a; color: #94a3b8; border: 1px solid #334155; border-bottom: none;
    padding: 6px 16px; border-top-left-radius: 6px; border-top-right-radius: 6px; margin-right: 2px;
}
QTabBar::tab:selected { background: #1e293b; color: #3b82f6; }
QTabBar::tab:hover:!selected { background: #1e293b; }

QScrollBar:vertical { background: transparent; width: 10px; }
QScrollBar::handle:vertical { background: #475569; border-radius: 5px; min-height: 30px; }
QScrollBar::handle:vertical:hover { background: #64748b; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

QLabel { background: transparent; }
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
