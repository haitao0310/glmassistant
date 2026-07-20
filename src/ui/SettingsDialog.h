#ifndef GLM_SETTINGS_DIALOG_H
#define GLM_SETTINGS_DIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsDialog; }
QT_END_NAMESPACE

namespace glm {

// 设置对话框:API Key + 主题 + 关于。确定时存 QSettings。
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void onAccepted();

private:
    Ui::SettingsDialog *ui;
};

} // namespace glm

#endif // GLM_SETTINGS_DIALOG_H
