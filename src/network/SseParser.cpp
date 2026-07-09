#include "SseParser.h"
#include "../infrastructure/Logger.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace glm {

// feed: 状态机解析 SSE 字节流。
// SSE 帧格式:"data: <payload>\n\n"(双换行分帧)。
// 跨 chunk 的不完整帧留在 m_buffer,下次 feed 拼接(处理 TCP 分包)。
QList<QString> SseParser::feed(const QByteArray &chunk)
{
    QList<QString> payloads;
    m_buffer += chunk;

    // 防御:m_buffer 上限 1MB,防服务端发垃圾(无 \n\n)导致内存涨
    constexpr int kMaxBuffer = 1 * 1024 * 1024;
    if (m_buffer.size() > kMaxBuffer) {
        logError("sse", QStringLiteral("buffer 溢出(>1MB 无 \\n\\n),疑似异常流,已丢弃"));
        m_buffer.clear();
        return payloads;
    }

    // 按双换行切帧
    int idx;
    while ((idx = m_buffer.indexOf("\n\n")) != -1) {
        const QByteArray frame = m_buffer.left(idx);
        m_buffer = m_buffer.mid(idx + 2);

        // 帧内可能多行,取 "data:" 开头的行(GLM 单行 data,多行按 SSE 规范应拼接)
        for (const QByteArray &line : frame.split('\n')) {
            const QByteArray trimmed = line.trimmed();
            if (trimmed.startsWith("data:")) {
                const QByteArray payload = trimmed.mid(5).trimmed();  // 去 "data:" 前缀
                payloads.append(QString::fromUtf8(payload));
            }
        }
    }
    return payloads;
}

void SseParser::reset()
{
    m_buffer.clear();
}

// GLM 格式: {"choices":[{"delta":{"content":"增量文本"}}]}
// 无 content(role 帧/finish 帧)返回空串(不报错——这些帧本就无文本)。
QString SseParser::extractDeltaContent(const QString &dataJson)
{
    if (dataJson.isEmpty()) return {};
    const QJsonDocument doc = QJsonDocument::fromJson(dataJson.toUtf8());
    if (!doc.isObject()) return {};
    const QJsonArray choices = doc.object().value("choices").toArray();
    if (choices.isEmpty()) return {};
    const QJsonObject delta = choices.at(0).toObject().value("delta").toObject();
    return delta.value("content").toString();
}

bool SseParser::isDone(const QString &dataLine)
{
    return dataLine.trimmed() == QStringLiteral("[DONE]");
}

} // namespace glm
