#ifndef GLM_PARAM_PANEL_H
#define GLM_PARAM_PANEL_H

#include <QWidget>
#include "../core/LlmTypes.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ParamPanel; }
QT_END_NAMESPACE

namespace glm {

// 参数面板:模型/temperature/top_p/max_tokens。
// UI 在 forms/parampanel.ui(Designer)。cpp 只做 connect + 逻辑。
class ParamPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ParamPanel(QWidget *parent = nullptr);
    ~ParamPanel();

    GenerationParams params() const;
    void setParams(const GenerationParams &p);

signals:
    void paramsChanged(const glm::GenerationParams &p);

private:
    Ui::ParamPanel *ui;
};

} // namespace glm

#endif // GLM_PARAM_PANEL_H
