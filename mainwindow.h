#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "src/app/ChatController.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace glm { class SessionManager; class ParamPanel; }
class QListWidget;
class QTextBrowser;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(glm::ChatController *controller, glm::SessionManager *sessions, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSendClicked();
    void onStateChanged(glm::ChatController::State s);
    void onErrorOccurred(const QString &error);
    void refreshSessionList();
    void onCurrentSessionChanged(const QString &id);

private:
    Ui::MainWindow *ui;
    glm::ChatController *m_controller;
    glm::SessionManager *m_sessions;
    glm::ParamPanel *m_paramPanel;
    class QListWidget *m_sessionList;
    class QTextBrowser *m_chatView;
    class QTextEdit *m_inputEdit;
    class QPushButton *m_sendBtn;
    class QPushButton *m_newSessionBtn;
    class QPushButton *m_delSessionBtn;
    class QPushButton *m_themeBtn;
    class QLabel *m_statusLabel;

    void rerenderChat();   // 全量 md 渲染
    void updateButtonByState(glm::ChatController::State s);
};

#endif // MAINWINDOW_H
