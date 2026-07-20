#ifndef GLM_CHAT_WIDGET_H
#define GLM_CHAT_WIDGET_H

#include <QWidget>
#include <QString>
#include "../app/ChatController.h"

namespace glm { class SessionManager; }
QT_BEGIN_NAMESPACE
namespace Ui { class ChatWidget; }
QT_END_NAMESPACE

/**
 * 对话区(会话列表 + 消息 + 输入 + 按钮 + 参数面板)。
 * UI 在 forms/chatwidget.ui(Designer)。cpp 只做 connect + 逻辑。
 */
class ChatWidget : public QWidget
{
    Q_OBJECT
public:
    ChatWidget(glm::ChatController *controller, glm::SessionManager *sessions, QWidget *parent = nullptr);
    ~ChatWidget();

private slots:
    void onSendClicked();
    void onStateChanged(glm::ChatController::State s);
    void onErrorOccurred(const QString &error);
    void refreshSessionList();
    void onCurrentSessionChanged(const QString &id);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    Ui::ChatWidget *ui;
    glm::ChatController *m_controller;
    glm::SessionManager *m_sessions;
    QString m_lastTokenStr;
    int m_totalTokens = 0;

    void rerenderChat();
    void updateButtonByState(glm::ChatController::State s);
};

#endif // GLM_CHAT_WIDGET_H
