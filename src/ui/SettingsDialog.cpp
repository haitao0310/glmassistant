#include "SettingsDialog.h"
#include "ui_settingsdialog.h"

#include "../infrastructure/SettingsManager.h"

namespace glm {

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    // 加载当前配置
    ui->apiKeyEdit->setText(SettingsManager::instance().apiKey());
    ui->themeCombo->setCurrentIndex(static_cast<int>(SettingsManager::instance().theme()));

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccepted);
}

SettingsDialog::~SettingsDialog() { delete ui; }

void SettingsDialog::onAccepted()
{
    SettingsManager::instance().saveApiKey(ui->apiKeyEdit->text());
    const int themeIdx = ui->themeCombo->currentIndex();
    SettingsManager::instance().saveTheme(static_cast<ThemeManager::Theme>(themeIdx));
}

} // namespace glm
