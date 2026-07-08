#include "DebugController.h"

#include "../data/DatabaseManager.h"
#include "../app/SessionManager.h"
#include "../infrastructure/Logger.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace glm {

DebugController::DebugController(ILlmProvider *provider, SessionManager *sessions, QObject *parent)
    : QObject(parent)
    , m_provider(provider)
    , m_sessions(sessions)
{
}

void DebugController::record(const RequestRecord &r)
{
    DatabaseManager::instance().createRequest(r);
    emit recordAppended(r);
    emit historyChanged();
}

QList<RequestRecord> DebugController::history() const
{
    // 默认当前会话的请求;无会话则全部
    if (m_sessions && !m_sessions->currentSessionId().isEmpty()) {
        return DatabaseManager::instance().requests(m_sessions->currentSessionId());
    }
    return DatabaseManager::instance().requests();
}

void DebugController::deleteRecord(int id)
{
    DatabaseManager::instance().deleteRequest(id);
    emit historyChanged();
}

// 重放:从记录的 rawRequest 反解析 messages + 参数,重新发起
LlmReply *DebugController::replay(const RequestRecord &r)
{
    if (!m_provider) return nullptr;

    LlmRequest req;
    req.model = r.model;
    req.temperature = r.temperature;
    req.topP = r.topP;
    req.maxTokens = r.maxTokens;
    req.stream = true;

    const QJsonDocument doc = QJsonDocument::fromJson(r.rawRequest.toUtf8());
    if (doc.isObject()) {
        const QJsonArray msgs = doc.object().value("messages").toArray();
        for (const QJsonValue &v : msgs) {
            Message m;
            m.role = Message::roleFromName(v.toObject().value("role").toString());
            m.content = v.toObject().value("content").toString();
            req.messages.append(m);
        }
    }
    logInfo("debug", QStringLiteral("replay request id=%1").arg(r.id));
    return m_provider->send(req);
}

// 对比:并发发起多个不同参数的请求,调用方收集各 reply 结果并排显示
QList<LlmReply *> DebugController::compareBatch(const QList<LlmRequest> &reqs)
{
    QList<LlmReply *> replies;
    if (!m_provider) return replies;
    for (const LlmRequest &req : reqs) {
        replies.append(m_provider->send(req));
    }
    logInfo("debug", QStringLiteral("compare batch: %1 requests").arg(reqs.size()));
    return replies;
}

} // namespace glm
