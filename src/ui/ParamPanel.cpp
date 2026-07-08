#include "ParamPanel.h"
#include "../infrastructure/Constants.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QFormLayout>

namespace glm {

ParamPanel::ParamPanel(QWidget *parent) : QWidget(parent)
{
    auto *form = new QFormLayout(this);

    m_modelBox = new QComboBox;
    m_modelBox->addItems(constants::GLM_MODELS);

    m_tempBox = new QDoubleSpinBox;
    m_tempBox->setRange(0.0, 2.0);
    m_tempBox->setSingleStep(0.1);
    m_tempBox->setValue(0.7);

    m_topPBox = new QDoubleSpinBox;
    m_topPBox->setRange(0.0, 1.0);
    m_topPBox->setSingleStep(0.1);
    m_topPBox->setValue(1.0);

    m_maxTokBox = new QSpinBox;
    m_maxTokBox->setRange(1, 32768);
    m_maxTokBox->setValue(2048);

    form->addRow(tr("模型"), m_modelBox);
    form->addRow(tr("temperature"), m_tempBox);
    form->addRow(tr("top_p"), m_topPBox);
    form->addRow(tr("max_tokens"), m_maxTokBox);

    // 任一控件变 → 发 paramsChanged
    connect(m_modelBox, &QComboBox::currentTextChanged, this, [this](const QString &){ emitChanged(); });
    connect(m_tempBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double){ emitChanged(); });
    connect(m_topPBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double){ emitChanged(); });
    connect(m_maxTokBox, qOverload<int>(&QSpinBox::valueChanged), this, [this](int){ emitChanged(); });
}

GenerationParams ParamPanel::params() const
{
    GenerationParams p;
    p.model = m_modelBox->currentText();
    p.temperature = m_tempBox->value();
    p.topP = m_topPBox->value();
    p.maxTokens = m_maxTokBox->value();
    return p;
}

void ParamPanel::emitChanged()
{
    emit paramsChanged(params());
}

} // namespace glm
