#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace glm { class ChatController; class SessionManager; class DebugController; }

// 主窗口:纯容器(组装 Tab + 窗口持久化)。对话区在 ChatWidget,调试在 DebugView。
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(glm::ChatController *controller, glm::SessionManager *sessions,
               glm::DebugController *debug, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
