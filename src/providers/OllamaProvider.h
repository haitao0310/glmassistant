#ifndef GLM_OLLAMA_PROVIDER_H
#define GLM_OLLAMA_PROVIDER_H

#include <QObject>
#include "../core/ILlmProvider.h"
#include "../core/LlmTypes.h"

namespace glm {

class HttpClient;

// Ollama 本地模型 Provider(扩展点 1 第二个实现,演示多 Provider)。
// OpenAI 兼容端点 localhost:11434/v1/chat/completions,无 key。
class OllamaProvider : public QObject, public ILlmProvider
{
    Q_OBJECT
public:
    explicit OllamaProvider(HttpClient *http, QObject *parent = nullptr);

    QString id() const override { return QStringLiteral("ollama"); }
    QString displayName() const override { return QStringLiteral("Ollama (本地)"); }
    QStringList models() const override;

    LlmReply *send(const LlmRequest &req) override;

private:
    HttpClient *m_http;
};

} // namespace glm

#endif // GLM_OLLAMA_PROVIDER_H
