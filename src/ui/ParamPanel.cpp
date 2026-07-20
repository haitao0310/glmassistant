#include "ParamPanel.h"
#include "ui_parampanel.h"

#include "../infrastructure/Constants.h"

namespace glm {

ParamPanel::ParamPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ParamPanel)
{
    ui->setupUi(this);
    ui->modelBox->addItems(constants::GLM_MODELS);

    connect(ui->modelBox, &QComboBox::currentTextChanged, this, [this](const QString &){ emit paramsChanged(params()); });
    connect(ui->tempBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double){ emit paramsChanged(params()); });
    connect(ui->topPBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double){ emit paramsChanged(params()); });
    connect(ui->maxTokBox, qOverload<int>(&QSpinBox::valueChanged), this, [this](int){ emit paramsChanged(params()); });
}

ParamPanel::~ParamPanel() { delete ui; }

GenerationParams ParamPanel::params() const
{
    GenerationParams p;
    p.model = ui->modelBox->currentText();
    p.temperature = ui->tempBox->value();
    p.topP = ui->topPBox->value();
    p.maxTokens = ui->maxTokBox->value();
    return p;
}

void ParamPanel::setParams(const GenerationParams &p)
{
    const int idx = ui->modelBox->findText(p.model);
    if (idx >= 0) ui->modelBox->setCurrentIndex(idx);
    ui->tempBox->setValue(p.temperature);
    ui->topPBox->setValue(p.topP);
    ui->maxTokBox->setValue(p.maxTokens);
}

} // namespace glm
