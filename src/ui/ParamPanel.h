#ifndef GLM_PARAM_PANEL_H
#define GLM_PARAM_PANEL_H

#include <QWidget>
#include "../core/LlmTypes.h"

QT_BEGIN_NAMESPACE
class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
QT_END_NAMESPACE

namespace glm {

// 参数面板:temperature / top_p / max_tokens / model。
// UI 层,不碰业务;调参发 paramsChanged 给 Controller。
class ParamPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ParamPanel(QWidget *parent = nullptr);

    GenerationParams params() const;
    void setParams(const GenerationParams &p);   // 从 QSettings 恢复时调

signals:
    void paramsChanged(const glm::GenerationParams &p);

private:
    QComboBox *m_modelBox;
    QDoubleSpinBox *m_tempBox;
    QDoubleSpinBox *m_topPBox;
    QSpinBox *m_maxTokBox;

    void emitChanged();
};

} // namespace glm

#endif // GLM_PARAM_PANEL_H
