#ifndef GLM_CONSTANTS_H
#define GLM_CONSTANTS_H

#include <QString>
#include <QStringList>

namespace glm::constants {

// GLM API
inline constexpr auto GLM_API_ENV_KEY = "GLM_API_KEY";   // 读 key 的环境变量名
inline const QString GLM_API_ENDPOINT =
    QStringLiteral("https://open.bigmodel.cn/api/paas/v4/chat/completions");
inline const QString GLM_DEFAULT_MODEL = QStringLiteral("glm-4-flash");
inline const QStringList GLM_MODELS = {
    QStringLiteral("glm-4-flash"),
    QStringLiteral("glm-4-flashx"),
    QStringLiteral("glm-4-plus"),
    QStringLiteral("glm-4-long"),
};

// HTTP / SSE
inline constexpr int HTTP_TIMEOUT_MS = 60000;     // 单次请求超时
inline constexpr int HTTP_RETRY_COUNT = 2;        // 失败重试次数(P4 中间件)
inline constexpr auto SSE_DONE_FLAG = "[DONE]";   // 流式结束标志

// 应用
inline constexpr auto APP_NAME = "GlmAssistant";
inline constexpr auto APP_VERSION = "0.1.0";

} // namespace glm::constants

#endif // GLM_CONSTANTS_H
