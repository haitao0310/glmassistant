#ifndef GLM_LOGGER_H
#define GLM_LOGGER_H

#include <QString>

namespace glm {

enum class LogLevel { Debug, Info, Warning, Error };

// 日志系统:按日期分文件 + 大小滚动 + 启动 Banner + 多线程安全。
class Logger
{
public:
    static void install(const QString &fileDir = {});
    static void setLevel(LogLevel level);
    static void log(LogLevel level, const QString &category, const QString &message);
};

void logDebug(const QString &category, const QString &msg);
void logInfo(const QString &category, const QString &msg);
void logWarning(const QString &category, const QString &msg);
void logError(const QString &category, const QString &msg);

} // namespace glm

// 宏(简化调用)
#define LOG_INFO(cat, msg)  glm::logInfo(cat, msg)
#define LOG_WARN(cat, msg)  glm::logWarning(cat, msg)
#define LOG_ERROR(cat, msg) glm::logError(cat, msg)

#endif // GLM_LOGGER_H
