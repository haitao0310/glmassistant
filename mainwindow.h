#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "src/app/ChatController.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Controller 由 main 注入(ADR-008),不自建
    explicit MainWindow(glm::ChatController *controller, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSendClicked();                                 // 发送/停止(按状态切换)
    void onChunkReceived(const QString &text);            // 流式增量(打字机)
    void onFinished(const QString &fullText);              // 生成完成
    void onErrorOccurred(const QString &error);
    void onStateChanged(glm::ChatController::State s);     // 状态→按钮态

private:
    Ui::MainWindow *ui;
    glm::ChatController *m_controller;
    class QTextEdit *m_inputEdit;
    class QTextEdit *m_outputEdit;
    class QPushButton *m_sendBtn;   // 发送/停止复用,按状态切文字

    void updateButtonByState(glm::ChatController::State s);
};

#endif // MAINWINDOW_H
