#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "src/ui/ChatWidget.h"
#include "src/ui/DebugView.h"
#include "src/infrastructure/SettingsManager.h"
#include "src/infrastructure/ThemeManager.h"

#include <QTabWidget>

MainWindow::MainWindow(glm::ChatController *controller, glm::SessionManager *sessions,
                       glm::DebugController *debug, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("GlmAssistant"));

    auto *tabs = new QTabWidget(this);
    tabs->addTab(new ChatWidget(controller, sessions, this), QStringLiteral("对话"));
    tabs->addTab(new glm::DebugView(debug, this), QStringLiteral("调试"));
    setCentralWidget(tabs);

    // 启动恢复:主题 + 窗口几何
    glm::ThemeManager::apply(glm::SettingsManager::instance().theme(), qApp);
    restoreGeometry(glm::SettingsManager::instance().windowGeometry());
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::closeEvent(QCloseEvent *event)
{
    glm::SettingsManager::instance().saveWindowGeometry(saveGeometry());
    QMainWindow::closeEvent(event);
}
