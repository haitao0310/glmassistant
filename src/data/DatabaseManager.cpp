#include "DatabaseManager.h"

#include "../infrastructure/Logger.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QUuid>

namespace glm {

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager inst;   // C++11 magic static,线程安全
    return inst;
}

DatabaseManager::DatabaseManager()
{
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("glmassistant"));
}

bool DatabaseManager::open(const QString &path)
{
    if (m_db.isOpen()) return true;
    m_currentPath = path;
    m_db.setDatabaseName(path);
    if (!m_db.open()) {
        logError("db", QStringLiteral("open failed: %1").arg(m_db.lastError().text()));
        return false;
    }
    return ensureSchema();
}

void DatabaseManager::close()
{
    if (m_db.isOpen()) m_db.close();
}

bool DatabaseManager::isOpen() const { return m_db.isOpen(); }

// schema 版本化迁移(防玩具 DoD:可演进,不丢老数据)
bool DatabaseManager::ensureSchema()
{
    QSqlQuery q(m_db);
    auto exec = [&](const QString &sql) -> bool {
        if (!q.exec(sql)) {
            logError("db", QStringLiteral("schema: %1 | %2").arg(q.lastError().text(), sql));
            return false;
        }
        return true;
    };

    exec(QStringLiteral("CREATE TABLE IF NOT EXISTS schema_version (version INTEGER PRIMARY KEY)"));
    int version = 0;
    if (q.exec(QStringLiteral("SELECT MAX(version) FROM schema_version")) && q.next()) {
        version = q.value(0).toInt();
    }

    if (version < 1) {
        exec(QStringLiteral("CREATE TABLE IF NOT EXISTS sessions ("
            "id TEXT PRIMARY KEY, title TEXT NOT NULL, created_time INTEGER, updated_time INTEGER)"));
        exec(QStringLiteral("CREATE TABLE IF NOT EXISTS messages ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, session_id TEXT NOT NULL, "
            "role TEXT NOT NULL, content TEXT NOT NULL, timestamp INTEGER, "
            "FOREIGN KEY(session_id) REFERENCES sessions(id) ON DELETE CASCADE)"));
        exec(QStringLiteral("INSERT INTO schema_version(version) VALUES(1)"));
        logInfo("db", QStringLiteral("schema v1 created"));
    }
    // 后续 v2/v3 迁移:if (version < 2) { ... }
    return true;
}

QString DatabaseManager::createSession(const QString &title)
{
    const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("INSERT INTO sessions(id, title, created_time, updated_time) VALUES(?,?,?,?)"));
    q.addBindValue(id);
    q.addBindValue(title.isEmpty() ? QStringLiteral("新会话") : title);
    q.addBindValue(now);
    q.addBindValue(now);
    if (!q.exec()) { logError("db", "createSession: " + q.lastError().text()); return {}; }
    return id;
}

QList<ChatSession> DatabaseManager::sessions()
{
    QList<ChatSession> result;
    QSqlQuery q(m_db);
    if (!q.exec(QStringLiteral("SELECT id, title, created_time, updated_time FROM sessions ORDER BY updated_time DESC"))) {
        logError("db", "sessions: " + q.lastError().text());
        return result;
    }
    while (q.next()) {
        ChatSession s;
        s.id = q.value(0).toString();
        s.title = q.value(1).toString();
        s.createdTime = q.value(2).toLongLong();
        s.updatedTime = q.value(3).toLongLong();
        result.append(s);
    }
    return result;
}

bool DatabaseManager::renameSession(const QString &id, const QString &title)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("UPDATE sessions SET title=? WHERE id=?"));
    q.addBindValue(title);
    q.addBindValue(id);
    return q.exec();
}

bool DatabaseManager::deleteSession(const QString &id)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("DELETE FROM messages WHERE session_id=?"));
    q.addBindValue(id);
    q.exec();
    q.prepare(QStringLiteral("DELETE FROM sessions WHERE id=?"));
    q.addBindValue(id);
    return q.exec();
}

bool DatabaseManager::appendMessage(const QString &sessionId, const Message &m)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("INSERT INTO messages(session_id, role, content, timestamp) VALUES(?,?,?,?)"));
    q.addBindValue(sessionId);
    q.addBindValue(m.roleName());
    q.addBindValue(m.content);
    q.addBindValue(m.timestamp);
    if (!q.exec()) { logError("db", "appendMessage: " + q.lastError().text()); return false; }
    q.prepare(QStringLiteral("UPDATE sessions SET updated_time=? WHERE id=?"));
    q.addBindValue(QDateTime::currentMSecsSinceEpoch());
    q.addBindValue(sessionId);
    q.exec();
    return true;
}

QList<Message> DatabaseManager::messages(const QString &sessionId)
{
    QList<Message> result;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT role, content, timestamp FROM messages WHERE session_id=? ORDER BY id ASC"));
    q.addBindValue(sessionId);
    if (!q.exec()) { logError("db", "messages: " + q.lastError().text()); return result; }
    while (q.next()) {
        Message m;
        m.role = Message::roleFromName(q.value(0).toString());
        m.content = q.value(1).toString();
        m.timestamp = q.value(2).toLongLong();
        result.append(m);
    }
    return result;
}

bool DatabaseManager::updateLastMessageContent(const QString &sessionId, const QString &content)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("UPDATE messages SET content=? WHERE session_id=? "
        "AND id=(SELECT MAX(id) FROM messages WHERE session_id=?)"));
    q.addBindValue(content);
    q.addBindValue(sessionId);
    q.addBindValue(sessionId);
    return q.exec();
}

bool DatabaseManager::clearMessages(const QString &sessionId)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("DELETE FROM messages WHERE session_id=?"));
    q.addBindValue(sessionId);
    return q.exec();
}

} // namespace glm
