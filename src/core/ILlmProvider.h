#ifndef GLM_ILLM_PROVIDER_H
#define GLM_ILLM_PROVIDER_H

#include <QString>
#include <QStringList>
#include "LlmTypes.h"
#include "LlmReply.h"

namespace glm {

// LLM Provider 抽象接口 —— 多厂商扩展的根(扩展点 1)。
//
// 加新 provider(Ollama/OpenAI/本地模型)只需:
//   1. 实现 ILlmProvider
//   2. 注册到 ProviderRegistry(P5)
// UI/业务层零改动。
class ILlmProvider
{
public:
    virtual ~ILlmProvider() = default;

    virtual QString id() const = 0;             // "glm"
    virtual QString displayName() const = 0;     // "智谱 GLM"
    virtual QStringList models() const = 0;      // 可用模型列表

    // 异步发送请求,返回 LlmReply(流式发 chunkReceived,完成发 finished)。
    // 调用方 connect LlmReply 的信号拿结果;reply 的 parent 由调用方管理。
    virtual LlmReply *send(const LlmRequest &req) = 0;
};

} // namespace glm

#endif // GLM_ILLM_PROVIDER_H
