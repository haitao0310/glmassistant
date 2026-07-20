#ifndef GLM_DEBUG_VIEW_H
#define GLM_DEBUG_VIEW_H

#include <QWidget>
#include <QList>
#include "../core/LlmTypes.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DebugView; }
QT_END_NAMESPACE

namespace glm {

class DebugController;

// 调试视图:历史请求库 + 原始请求/响应 JSON 查看 + 重放。
// UI 在 forms/debugview.ui(Designer)。cpp 只做 connect + 逻辑。
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
    Ui::DebugView *ui;
    DebugController *m_ctrl;
    QList<RequestRecord> m_records;
};

} // namespace glm

#endif // GLM_DEBUG_VIEW_H
