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

    // v2: 调试请求历史库(P4)
    if (version < 2) {
        exec(QStringLiteral("CREATE TABLE IF NOT EXISTS requests ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, session_id TEXT, model TEXT, "
            "temperature REAL, top_p REAL, max_tokens INTEGER, "
            "raw_request TEXT, raw_response TEXT, timestamp INTEGER)"));
        exec(QStringLiteral("INSERT INTO schema_version(version) VALUES(2)"));
        logInfo("db", QStringLiteral("schema v2 created (requests table)"));
    }

    // C2 索引(性能优化):按会话查消息/请求
    exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_messages_session ON messages(session_id, timestamp)"));
    exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_requests_session ON requests(session_id)"));

    // v3: requests 表加 token 列(Token 统计持久化)
    if (version < 3) {
        exec(QStringLiteral("ALTER TABLE requests ADD COLUMN prompt_tokens INTEGER DEFAULT 0"));
        exec(QStringLiteral("ALTER TABLE requests ADD COLUMN completion_tokens INTEGER DEFAULT 0"));
        exec(QStringLiteral("ALTER TABLE requests ADD COLUMN total_tokens INTEGER DEFAULT 0"));
        exec(QStringLiteral("INSERT INTO schema_version(version) VALUES(3)"));
        logInfo("db", QStringLiteral("schema v3: requests +token columns"));
    }
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
    if (!m_db.transaction()) return false;   // 事务:INSERT + UPDATE 原子(防数据不一致)
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("INSERT INTO messages(session_id, role, content, timestamp) VALUES(?,?,?,?)"));
    q.addBindValue(sessionId);
    q.addBindValue(m.roleName());
    q.addBindValue(m.content);
    q.addBindValue(m.timestamp);
    if (!q.exec()) { logError("db", "appendMessage insert: " + q.lastError().text()); m_db.rollback(); return false; }
    q.prepare(QStringLiteral("UPDATE sessions SET updated_time=? WHERE id=?"));
    q.addBindValue(QDateTime::currentMSecsSinceEpoch());
    q.addBindValue(sessionId);
    if (!q.exec()) { logError("db", "appendMessage update: " + q.lastError().text()); m_db.rollback(); return false; }
    if (!m_db.commit()) { m_db.rollback(); return false; }
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

int DatabaseManager::createRequest(const RequestRecord &r)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("INSERT INTO requests(session_id, model, temperature, top_p, max_tokens, raw_request, raw_response, timestamp, prompt_tokens, completion_tokens, total_tokens) "
        "VALUES(?,?,?,?,?,?,?,?,?,?,?)"));
    q.addBindValue(r.sessionId);
    q.addBindValue(r.model);
    q.addBindValue(r.temperature);
    q.addBindValue(r.topP);
    q.addBindValue(r.maxTokens);
    q.addBindValue(r.rawRequest);
    q.addBindValue(r.rawResponse);
    q.addBindValue(r.timestamp);
    q.addBindValue(r.promptTokens);
    q.addBindValue(r.completionTokens);
    q.addBindValue(r.totalTokens);
    if (!q.exec()) { logError("db", "createRequest: " + q.lastError().text()); return -1; }
    return q.lastInsertId().toInt();
}

QList<RequestRecord> DatabaseManager::requests(const QString &sessionId)
{
    QList<RequestRecord> result;
    QSqlQuery q(m_db);
    if (sessionId.isEmpty()) {
        q.exec(QStringLiteral("SELECT id, session_id, model, temperature, top_p, max_tokens, raw_request, raw_response, timestamp "
            "FROM requests ORDER BY id DESC"));
    } else {
        q.prepare(QStringLiteral("SELECT id, session_id, model, temperature, top_p, max_tokens, raw_request, raw_response, timestamp "
            "FROM requests WHERE session_id=? ORDER BY id DESC"));
        q.addBindValue(sessionId);
        q.exec();
    }
    while (q.next()) {
        RequestRecord r;
        r.id = q.value(0).toInt();
        r.sessionId = q.value(1).toString();
        r.model = q.value(2).toString();
        r.temperature = q.value(3).toDouble();
        r.topP = q.value(4).toDouble();
        r.maxTokens = q.value(5).toInt();
        r.rawRequest = q.value(6).toString();
        r.rawResponse = q.value(7).toString();
        r.timestamp = q.value(8).toLongLong();
        result.append(r);
    }
    return result;
}

bool DatabaseManager::updateRequestResponse(int id, const QString &rawResponse)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("UPDATE requests SET raw_response=? WHERE id=?"));
    q.addBindValue(rawResponse);
    q.addBindValue(id);
    return q.exec();
}

bool DatabaseManager::deleteRequest(int id)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("DELETE FROM requests WHERE id=?"));
    q.addBindValue(id);
    return q.exec();
}

} // namespace glm
