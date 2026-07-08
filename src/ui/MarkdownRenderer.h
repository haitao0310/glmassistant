#ifndef GLM_MARKDOWN_RENDERER_H
#define GLM_MARKDOWN_RENDERER_H

#include <QString>

namespace glm {

// Markdown → HTML 渲染(用 QTextDocument::setMarkdown,Qt 5.14+)。
// assistant 回复的 md(代码块/列表/标题/加粗)→ HTML → 富文本显示。
class MarkdownRenderer
{
public:
    static QString toHtml(const QString &md);
};

} // namespace glm

#endif // GLM_MARKDOWN_RENDERER_H
