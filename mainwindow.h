#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "glmclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSendClicked();                         //点发送按钮
    void onReplyReceived(const QString &content); //收到 GLM 回复
    void onErrorOccurred(const QString &error);   //网络/解析出错

private:
    Ui::MainWindow *ui;
    GlmClient *m_glm = nullptr;
    //第一步手写控件(后面模块化时拆到独立 ChatWidget)
    class QTextEdit *m_inputEdit;
    class QTextEdit *m_outputEdit;
    class QPushButton *m_sendBtn;
};

#endif // MAINWINDOW_H
