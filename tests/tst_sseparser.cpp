#include <QtTest/QtTest>
#include <QStringList>
#include "../src/network/SseParser.h"

// SseParser 单元测试(P1 地基,SSE 解析是流式核心,必须先测稳)
class TestSseParser : public QObject
{
    Q_OBJECT

private slots:
    void testSingleFrame();               // 单帧正常解析
    void testMultipleFramesInOneChunk();  // 一个 chunk 多帧
    void testCrossChunkFrame();           // 帧跨 chunk(分包)
    void testExtractDeltaContent();       // 提取增量文本
    void testExtractDeltaContent_noContent();  // role 帧(无 content)
    void testExtractDeltaContent_badJson();    // 坏 JSON 不崩
    void testIsDone();                    // 结束标志
};

void TestSseParser::testSingleFrame()
{
    glm::SseParser p;
    const QList<QString> r = p.feed("data: {\"choices\":[]}\n\n");
    QCOMPARE(r.size(), 1);
    QCOMPARE(r[0], QStringLiteral("{\"choices\":[]}"));
}

void TestSseParser::testMultipleFramesInOneChunk()
{
    glm::SseParser p;
    const QList<QString> r = p.feed("data: a\n\ndata: b\n\n");
    QCOMPARE(r.size(), 2);
    QCOMPARE(r[0], QStringLiteral("a"));
    QCOMPARE(r[1], QStringLiteral("b"));
}

void TestSseParser::testCrossChunkFrame()
{
    glm::SseParser p;
    const QList<QString> r1 = p.feed("data: {\"ch");   // 不完整,无完整帧
    QCOMPARE(r1.size(), 0);
    const QList<QString> r2 = p.feed("oices\":[]}\n\n"); // 补全,出 1 帧
    QCOMPARE(r2.size(), 1);
    QCOMPARE(r2[0], QStringLiteral("{\"choices\":[]}"));
}

void TestSseParser::testExtractDeltaContent()
{
    const QString json = "{\"choices\":[{\"delta\":{\"content\":\"你好\"}}]}";
    QCOMPARE(glm::SseParser::extractDeltaContent(json), QStringLiteral("你好"));
}

void TestSseParser::testExtractDeltaContent_noContent()
{
    // role 帧(首帧常只带 role,无 content)
    const QString json = "{\"choices\":[{\"delta\":{\"role\":\"assistant\"}}]}";
    QCOMPARE(glm::SseParser::extractDeltaContent(json), QStringLiteral(""));
}

void TestSseParser::testExtractDeltaContent_badJson()
{
    // 坏 JSON / 空串,不崩,返回空
    QCOMPARE(glm::SseParser::extractDeltaContent(QStringLiteral("not json")), QStringLiteral(""));
    QCOMPARE(glm::SseParser::extractDeltaContent(QStringLiteral("")), QStringLiteral(""));
}

void TestSseParser::testIsDone()
{
    QVERIFY(glm::SseParser::isDone(QStringLiteral("[DONE]")));
    QVERIFY(glm::SseParser::isDone(QStringLiteral("  [DONE]  ")));
    QVERIFY(!glm::SseParser::isDone(QStringLiteral("{\"choices\":[]}")));
}

QTEST_GUILESS_MAIN(TestSseParser)
#include "tst_sseparser.moc"
