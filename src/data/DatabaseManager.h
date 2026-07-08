#ifndef GLM_DATABASE_MANAGER_H
#define GLM_DATABASE_MANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include "../core/LlmTypes.h"

namespace glm {

// SQLite 持久化层(P3)——会话 + 消息存储,带 schema 版本迁移。
// 单例(应用级一个 DB),线程安全由调用方保证(P3 主线程用)。
class DatabaseManager
{
public:
    static DatabaseManager &instance();

    bool open(const QString &path);   // 打开/创建库(自动 ensureSchema)
    void close();
    bool isOpen() const;

    // === 会话 CRUD ===
    QString createSession(const QString &title);        // 返回新 session UUID
    QList<ChatSession> sessions();                       // 全部会话(列表用)
    bool renameSession(const QString &id, const QString &title);
    bool deleteSession(const QString &id);               // 级联删消息

    // === 消息 CRUD ===
    bool appendMessage(const QString &sessionId, const Message &m);
    QList<Message> messages(const QString &sessionId);   // 加载会话历史
    bool updateLastMessageContent(const QString &sessionId, const QString &content);  // 流式更新末条
    bool clearMessages(const QString &sessionId);

private:
    DatabaseManager();
    bool ensureSchema();   // 建表 + 版本迁移(防玩具 DoD:可演进)
    QSqlDatabase m_db;
    QString m_currentPath;
};

} // namespace glm

#endif // GLM_DATABASE_MANAGER_H
