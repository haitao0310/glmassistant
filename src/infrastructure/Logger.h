#ifndef GLM_LOGGER_H
#define GLM_LOGGER_H

#include <QString>

namespace glm {

// 横切日志(ADR-006):分类 + 可选文件 sink。
// 默认输出 qDebug(Qt Creator 输出面板);install() 后追加文件。
// 用法:logInfo("network") 不适用,改:logInfo("network", "connected");
enum class LogLevel { Debug, Info, Warning, Error };

class Logger
{
public:
    static void install(const QString &fileDir = {});   // 启动时调,设文件 sink
    static void setLevel(LogLevel level);                // 过滤级别
    static void log(LogLevel level, const QString &category, const QString &message);
};

// 便捷函数(category 分类,如 "network"/"sse"/"db")
void logDebug(const QString &category, const QString &msg);
void logInfo(const QString &category, const QString &msg);
void logWarning(const QString &category, const QString &msg);
void logError(const QString &category, const QString &msg);

} // namespace glm

#endif // GLM_LOGGER_H
