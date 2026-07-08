#ifndef GLM_DEBUG_CONTROLLER_H
#define GLM_DEBUG_CONTROLLER_H

#include <QObject>
#include <QList>
#include "../core/LlmTypes.h"
#include "../core/ILlmProvider.h"
#include "../core/LlmReply.h"

namespace glm {

class SessionManager;

// 调试模式控制器(P4):记录每次请求 raw、重放、多参数对比。
// 是 P4 差异化护城河的核心逻辑层。
class DebugController : public QObject
{
    Q_OBJECT
public:
    DebugController(ILlmProvider *provider, SessionManager *sessions, QObject *parent = nullptr);

    void record(const RequestRecord &r);              // 请求完成后记入 DB
    QList<RequestRecord> history() const;              // 历史请求库
    void deleteRecord(int id);

    LlmReply *replay(const RequestRecord &r);          // 重放:用记录参数重发,返回新 reply
    QList<LlmReply *> compareBatch(const QList<LlmRequest> &reqs);  // 对比:并发多请求

signals:
    void recordAppended(const glm::RequestRecord &r);
    void historyChanged();

private:
    ILlmProvider *m_provider;
    SessionManager *m_sessions;
};

} // namespace glm

#endif // GLM_DEBUG_CONTROLLER_H
