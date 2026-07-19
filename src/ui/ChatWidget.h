#ifndef GLM_CHAT_WIDGET_H
#define GLM_CHAT_WIDGET_H

#include <QWidget>
#include <QString>
#include "../app/ChatController.h"

namespace glm { class SessionManager; class ParamPanel; }
class QListWidget;
class QTextBrowser;
class QTextEdit;
class QPushButton;
class QLabel;

/**
 * 对话区(会话列表 + 消息显示 + 输入 + 按钮 + 参数面板)。
 * 自己 build UI + connect + 处理槽。MainWindow 只 addTab。
 */
class ChatWidget : public QWidget
{
    Q_OBJECT
public:
    ChatWidget(glm::ChatController *controller, glm::SessionManager *sessions, QWidget *parent = nullptr);

private slots:
    void onSendClicked();
    void onStateChanged(glm::ChatController::State s);
    void onErrorOccurred(const QString &error);
    void refreshSessionList();
    void onCurrentSessionChanged(const QString &id);

private:
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
    QString m_lastTokenStr;
    int m_totalTokens = 0;

    void buildUI();   // 只建控件+布局,不 connect
    void rerenderChat();
    void updateButtonByState(glm::ChatController::State s);
};

#endif // GLM_CHAT_WIDGET_H
