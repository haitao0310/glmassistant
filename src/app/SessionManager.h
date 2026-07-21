#ifndef GLM_SESSION_MANAGER_H
#define GLM_SESSION_MANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include "../core/LlmTypes.h"

namespace glm {

// 多会话管理(基于 DatabaseManager 单例)。
// 持"当前会话",切换时加载历史(messagesLoaded),供 ChatController 同步。
class SessionManager : public QObject
{
    Q_OBJECT
public:
    explicit SessionManager(QObject *parent = nullptr);

    QString currentSessionId() const { return m_currentId; }
    QList<ChatSession> sessions() const;        // 全部会话(侧栏列表)
    QList<Message> currentMessages() const;      // 当前会话历史

    QString newSession(const QString &title = {});   // 创建 + 设为当前,返回 id
    void switchTo(const QString &id);                 // 切换 + 加载历史
    void deleteCurrent();
    void renameCurrent(const QString &title);
    void rename(const QString &id, const QString &title);

signals:
    void sessionListChanged();                            // 会话列表变(侧栏刷新)
    void currentChanged(const QString &id);               // 当前会话变
    void messagesLoaded(const QList<glm::Message> &msgs); // 当前历史加载完

private:
    QString m_currentId;
};

} // namespace glm

#endif // GLM_SESSION_MANAGER_H
