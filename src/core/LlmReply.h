#ifndef GLM_LLM_REPLY_H
#define GLM_LLM_REPLY_H

#include <QObject>
#include <QString>
#include <functional>

namespace glm {

// 异步请求的统一返回对象(ADR-003)。
//
// Provider 通过 emitXxx 推进度;调用方 connect 信号拿结果。
//   流式: 多次 chunkReceived → 1 次 finished
//   非流式: 1 次 finished(无 chunk)
//   任何阶段: 可 errorOccurred
// done() 在 finished/error 后都发,便于统一清理。
//
// 生命周期:Provider 的 send() 返回 LlmReply(parent 默认 nullptr);
//           调用方(ChatController)接管——持有指针,connect done() → deleteLater。
class LlmReply : public QObject
{
    Q_OBJECT
public:
    explicit LlmReply(QObject *parent = nullptr) : QObject(parent) {}

    // Provider 调用:推进度(内部 emit 信号)
    void emitChunk(const QString &text) { emit chunkReceived(text); }
    void emitFinished(const QString &fullText) { emit finished(fullText); emit done(); }
    void emitError(const QString &error) { emit errorOccurred(error); emit done(); }

    // 中断:Provider 在 send() 时 setAbortHandle(如 [resp]{ resp->abort(); }),
    //      调用方 stop() 时调 abort() 触发底层中断。
    //      中断后 Provider 应回调 emitError/emitFinished(发 done 促清理)。
    void setAbortHandle(std::function<void()> aborter) { m_aborter = std::move(aborter); }
    void abort() { if (m_aborter) m_aborter(); }

signals:
    void chunkReceived(const QString &text);    // 流式增量
    void finished(const QString &fullText);      // 完整文本(流式累积 / 非流式全量)
    void errorOccurred(const QString &error);
    void done();                                 // 终态(finished 或 error 后)

private:
    std::function<void()> m_aborter;
};

} // namespace glm

#endif // GLM_LLM_REPLY_H
