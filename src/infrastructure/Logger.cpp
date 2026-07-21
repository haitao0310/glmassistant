#include "Logger.h"

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDate>
#include <QDateTime>
#include <QStandardPaths>
#include <QMutex>
#include <QMutexLocker>

namespace glm {

namespace {
LogLevel g_level = LogLevel::Debug;
QString g_filePath;
QString g_fileDir;
QMutex g_mutex;

// 今天日志文件名:GlmAssistant_yyyyMMdd.log
QString todayFileName()
{
    return QStringLiteral("GlmAssistant_%1.log")
        .arg(QDate::currentDate().toString(QStringLiteral("yyyyMMdd")));
}

// >5MB 归档(rename 加时间戳)
void rotateIfNeeded()
{
    if (g_filePath.isEmpty()) return;
    const QFileInfo info(g_filePath);
    if (info.size() > 5 * 1024 * 1024) {
        const QString rolled = g_filePath + QStringLiteral(".")
            + QDateTime::currentDateTime().toString(QStringLiteral("hhmmss"));
        QFile::rename(g_filePath, rolled);
    }
}

// 跨天建新文件(进程跨午夜运行时)
void checkDateSwitch()
{
    if (g_fileDir.isEmpty() || g_filePath.isEmpty()) return;
    const QString today = todayFileName();
    if (!g_filePath.contains(today)) {
        g_filePath = g_fileDir + QStringLiteral("/") + today;
    }
}

void writeToFile(const QString &line)
{
    checkDateSwitch();
    rotateIfNeeded();
    QFile f(g_filePath);
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        f.write(line.toUtf8());
        f.write("\n");
    }
}
} // namespace

void Logger::install(const QString &fileDir)
{
    g_fileDir = fileDir.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        : fileDir;
    if (g_fileDir.isEmpty()) return;
    QDir().mkpath(g_fileDir);
    g_filePath = g_fileDir + QStringLiteral("/") + todayFileName();

    // 启动 Banner
    QMutexLocker lock(&g_mutex);
    QFile f(g_filePath);
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        f.write(QStringLiteral("======== GlmAssistant v0.1.0 | %1 ========\n")
                    .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")))
                    .toUtf8());
    }
}

void Logger::setLevel(LogLevel level) { g_level = level; }

void Logger::log(LogLevel level, const QString &category, const QString &message)
{
    if (static_cast<int>(level) < static_cast<int>(g_level)) return;

    const QString levelStr = [level] {
        switch (level) {
        case LogLevel::Debug:   return QStringLiteral("DEBUG");
        case LogLevel::Info:    return QStringLiteral("INFO");
        case LogLevel::Warning: return QStringLiteral("WARN");
        case LogLevel::Error:   return QStringLiteral("ERROR");
        }
        return QStringLiteral("?");
    }();

    const QString line = QStringLiteral("[%1][%2][%3] %4")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss.zzz")),
             levelStr, category, message);

    // 控制台
    switch (level) {
    case LogLevel::Error:   qCritical().noquote() << line; break;
    case LogLevel::Warning: qWarning().noquote() << line; break;
    default:                qDebug().noquote() << line; break;
    }

    // 文件(多线程安全)
    if (!g_filePath.isEmpty()) {
        QMutexLocker lock(&g_mutex);
        writeToFile(line);
    }
}

void logDebug(const QString &c, const QString &m)   { Logger::log(LogLevel::Debug, c, m); }
void logInfo(const QString &c, const QString &m)    { Logger::log(LogLevel::Info, c, m); }
void logWarning(const QString &c, const QString &m) { Logger::log(LogLevel::Warning, c, m); }
void logError(const QString &c, const QString &m)   { Logger::log(LogLevel::Error, c, m); }

} // namespace glm
