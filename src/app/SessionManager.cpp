#include "SessionManager.h"

#include "../data/DatabaseManager.h"
#include "../infrastructure/Logger.h"

namespace glm {

SessionManager::SessionManager(QObject *parent) : QObject(parent)
{
    // 启动:加载已有会话,无则新建
    const auto ss = sessions();
    if (ss.isEmpty()) {
        newSession();
    } else {
        switchTo(ss.first().id);
    }
}

QList<ChatSession> SessionManager::sessions() const
{
    return DatabaseManager::instance().sessions();
}

QList<Message> SessionManager::currentMessages() const
{
    if (m_currentId.isEmpty()) return {};
    return DatabaseManager::instance().messages(m_currentId);
}

QString SessionManager::newSession(const QString &title)
{
    const QString id = DatabaseManager::instance().createSession(title);
    if (id.isEmpty()) { logError("session", QStringLiteral("createSession failed")); return {}; }
    m_currentId = id;
    emit sessionListChanged();
    emit currentChanged(id);
    emit messagesLoaded({});
    return id;
}

void SessionManager::switchTo(const QString &id)
{
    if (id.isEmpty() || id == m_currentId) return;
    m_currentId = id;
    emit currentChanged(id);
    emit messagesLoaded(DatabaseManager::instance().messages(id));
    logInfo("session", QStringLiteral("switched to %1").arg(id));
}

void SessionManager::deleteCurrent()
{
    if (m_currentId.isEmpty()) return;
    DatabaseManager::instance().deleteSession(m_currentId);
    m_currentId.clear();
    emit sessionListChanged();
    const auto ss = sessions();
    if (ss.isEmpty()) newSession();
    else switchTo(ss.first().id);
}

void SessionManager::renameCurrent(const QString &title)
{
    if (m_currentId.isEmpty()) return;
    DatabaseManager::instance().renameSession(m_currentId, title);
    emit sessionListChanged();
}

void SessionManager::rename(const QString &id, const QString &title)
{
    DatabaseManager::instance().renameSession(id, title);
    emit sessionListChanged();
}

} // namespace glm
