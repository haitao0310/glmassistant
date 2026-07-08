#include "Logger.h"

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>

namespace glm {

namespace {
LogLevel g_level = LogLevel::Debug;
QString g_filePath;   // 非空则同时写文件

const char *levelName(LogLevel l)
{
    switch (l) {
    case LogLevel::Debug:   return "DEBUG";
    case LogLevel::Info:    return "INFO";
    case LogLevel::Warning: return "WARN";
    case LogLevel::Error:   return "ERROR";
    }
    return "?";
}
} // namespace

void Logger::install(const QString &fileDir)
{
    // 默认 %APPDATA%/GlmAssistant(规范存储,不污染 exe 目录)
    QString dir = fileDir.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        : fileDir;
    if (dir.isEmpty()) return;     // 无可写位置则只控制台
    QDir().mkpath(dir);
    g_filePath = dir + QStringLiteral("/GlmAssistant.log");
}

void Logger::setLevel(LogLevel level) { g_level = level; }

void Logger::log(LogLevel level, const QString &category, const QString &message)
{
    if (static_cast<int>(level) < static_cast<int>(g_level)) return;

    const QString line = QStringLiteral("[%1][%2][%3] %4")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss.zzz")),
             QString::fromLatin1(levelName(level)),
             category,
             message);

    // 控制台(Qt Creator 输出面板)
    switch (level) {
    case LogLevel::Error:   qCritical().noquote() << line; break;
    case LogLevel::Warning: qWarning().noquote() << line; break;
    default:                qDebug().noquote() << line; break;
    }

    // 文件(多线程安全)
    if (!g_filePath.isEmpty()) {
        static QMutex mtx;                       // C++11 magic static,线程安全初始化
        QMutexLocker lock(&mtx);
        QFile f(g_filePath);
        if (f.open(QIODevice::Append | QIODevice::Text)) {
            f.write(line.toUtf8());
            f.write("\n");
        }
    }
}

void logDebug(const QString &c, const QString &m)   { Logger::log(LogLevel::Debug, c, m); }
void logInfo(const QString &c, const QString &m)    { Logger::log(LogLevel::Info, c, m); }
void logWarning(const QString &c, const QString &m) { Logger::log(LogLevel::Warning, c, m); }
void logError(const QString &c, const QString &m)   { Logger::log(LogLevel::Error, c, m); }

} // namespace glm
