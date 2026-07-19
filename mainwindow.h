#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "src/app/ChatController.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace glm { class SessionManager; class ParamPanel; class DebugController; class DebugView; }
class QListWidget;
class QTextBrowser;
class QTabWidget;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(glm::ChatController *controller, glm::SessionManager *sessions,
               glm::DebugController *debug, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSendClicked();
    void onStateChanged(glm::ChatController::State s);
    void onErrorOccurred(const QString &error);
    void refreshSessionList();
    void onCurrentSessionChanged(const QString &id);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    glm::ChatController *m_controller;
    glm::SessionManager *m_sessions;
    glm::ParamPanel *m_paramPanel;
    glm::DebugView *m_debugView;
    class QTabWidget *m_tabs;
    class QListWidget *m_sessionList;
    class QTextBrowser *m_chatView;
    class QTextEdit *m_inputEdit;
    class QPushButton *m_sendBtn;
    class QPushButton *m_newSessionBtn;
    class QPushButton *m_delSessionBtn;
    class QPushButton *m_themeBtn;
    class QLabel *m_statusLabel;

    QWidget *buildChatTab();   // 构建对话 tab 内容
    void rerenderChat();
    void updateButtonByState(glm::ChatController::State s);
};

#endif // MAINWINDOW_H
