#ifndef GLM_SSE_PARSER_H
#define GLM_SSE_PARSER_H

#include <QString>
#include <QByteArray>
#include <QList>

namespace glm {

// SSE(Server-Sent Events)帧解析器 —— 纯函数,无状态,可单元测试。
//
// GLM 流式返回格式:每帧 "data: {json}\n\n",结束帧 "data: [DONE]\n\n"
// 本类处理增量字节流,跨 chunk 的不完整帧会在 buffer 拼接。
class SseParser
{
public:
    // 喂入一段字节,返回本段解析出的完整 data 负载列表(每项是一个 JSON 字符串)。
    // 不完整帧留在内部 buffer,下次 feed 拼接(处理 TCP 分包跨帧)。
    // 注意:为支持跨实例的状态隔离,P1 实现改为持有一个 feed(QByteArray) 成员
    //       + reset();静态版本用于无状态单帧解析。最终签名以 .cpp 实现为准。
    QList<QString> feed(const QByteArray &chunk);
    void reset();

    // 从单个 SSE data 负载(JSON)提取增量文本。
    // GLM 格式:choices[0].delta.content;无 content(role 帧/finish 帧)返回空串。
    static QString extractDeltaContent(const QString &dataJson);

    // 是否结束标志 [DONE]
    static bool isDone(const QString &dataLine);

private:
    QByteArray m_buffer;   // 跨 chunk 的不完整帧缓存
};

} // namespace glm

#endif // GLM_SSE_PARSER_H
