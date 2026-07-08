#ifndef GLM_DEBUG_VIEW_H
#define GLM_DEBUG_VIEW_H

#include <QWidget>
#include <QList>
#include "../core/LlmTypes.h"

QT_BEGIN_NAMESPACE
class QListWidget;
class QPlainTextEdit;
class QPushButton;
QT_END_NAMESPACE

namespace glm {

class DebugController;

// 调试视图(P4):历史请求库 + 原始请求/响应 JSON 查看(可编辑) + 重放。
class DebugView : public QWidget
{
    Q_OBJECT
public:
    DebugView(DebugController *ctrl, QWidget *parent = nullptr);
    void refresh();

private slots:
    void onSelectionChanged(int row);
    void onReplay();

private:
    DebugController *m_ctrl;
    QListWidget *m_historyList;
    QPlainTextEdit *m_requestView;
    QPlainTextEdit *m_responseView;
    QPushButton *m_replayBtn;
    QList<RequestRecord> m_records;
};

} // namespace glm

#endif // GLM_DEBUG_VIEW_H
