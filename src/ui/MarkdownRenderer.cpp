#include "MarkdownRenderer.h"

#include <QTextDocument>

namespace glm {

// Qt 内置 CommonMark 解析(setMarkdown),无需引外部 md 库。
QString MarkdownRenderer::toHtml(const QString &md)
{
    if (md.isEmpty()) return {};
    QTextDocument doc;
    doc.setMarkdown(md);
    return doc.toHtml();
}

} // namespace glm
