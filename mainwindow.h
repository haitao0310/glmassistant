#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "src/app/ChatController.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace glm { class ChatModel; class ParamPanel; }
class QListView;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(glm::ChatController *controller, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSendClicked();
    void onStateChanged(glm::ChatController::State s);
    void onErrorOccurred(const QString &error);

private:
    Ui::MainWindow *ui;
    glm::ChatController *m_controller;
    glm::ChatModel *m_chatModel;
    glm::ParamPanel *m_paramPanel;
    class QListView *m_messageList;
    class QTextEdit *m_inputEdit;
    class QPushButton *m_sendBtn;
    class QLabel *m_statusLabel;

    void updateButtonByState(glm::ChatController::State s);
};

#endif // MAINWINDOW_H
