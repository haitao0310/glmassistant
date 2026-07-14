#ifndef GLM_PROVIDER_UTILS_H
#define GLM_PROVIDER_UTILS_H

#include <QByteArray>
#include <QString>

namespace glm {

class HttpResponse;
class LlmReply;
struct LlmRequest;

// OpenAI 兼容 Provider 共用工具(providers/common)。
// 放此非 network:依赖 LlmReply(业务抽象),network 只管 HTTP/SSE 协议。

// SSE 流式连接 setup:resp 的 dataReceived/errorOccurred/finished → reply 信号
// + SSE 帧解析 + 友好错误。通用(任何 SSE 流式 Provider 复用,含未来 Claude/Gemini)。
void setupSseStreaming(HttpResponse *resp, LlmReply *reply);

// LlmRequest → OpenAI 兼容 chat/completions body(JSON 序列化)。
// 跳过空 content(服务端拒),含 tools(function calling)。
QByteArray serializeOpenAiRequest(const LlmRequest &req, const QString &defaultModel);

} // namespace glm

#endif // GLM_PROVIDER_UTILS_H
