#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "src/app/ChatModel.h"
#include "src/ui/ParamPanel.h"

#include <QListView>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(glm::ChatController *controller, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_controller(controller)
{
    ui->setupUi(this);
    setWindowTitle(tr("GlmAssistant - P2 多轮 + 参数"));

    m_chatModel = new glm::ChatModel(this);
    m_paramPanel = new glm::ParamPanel(this);
    m_messageList = new QListView;
    m_messageList->setModel(m_chatModel);
    m_inputEdit = new QTextEdit;
    m_inputEdit->setPlaceholderText(tr("输入消息..."));
    m_inputEdit->setMaximumHeight(80);
    m_sendBtn = new QPushButton(tr("发送"));
    m_statusLabel = new QLabel(tr("就绪"));

    // 布局:左 ParamPanel | 右(对话列表 + 输入 + 按钮 + 状态)
    auto *central = new QWidget(this);
    auto *hbox = new QHBoxLayout(central);
    hbox->addWidget(m_paramPanel);
    auto *vbox = new QVBoxLayout;
    vbox->addWidget(new QLabel(tr("对话")));
    vbox->addWidget(m_messageList, 1);
    vbox->addWidget(m_inputEdit);
    vbox->addWidget(m_sendBtn);
    vbox->addWidget(m_statusLabel);
    hbox->addLayout(vbox, 1);
    setCentralWidget(central);

    // Controller → ChatModel(数据同步,Model/View)
    connect(m_controller, &glm::ChatController::messageAppended, m_chatModel, &glm::ChatModel::appendMessage);
    connect(m_controller, &glm::ChatController::chunkReceived, m_chatModel, &glm::ChatModel::appendChunkToLast);
    connect(m_controller, &glm::ChatController::stateChanged, this, &MainWindow::onStateChanged);
    connect(m_controller, &glm::ChatController::errorOccurred, this, &MainWindow::onErrorOccurred);
    // ParamPanel → Controller
    connect(m_paramPanel, &glm::ParamPanel::paramsChanged, m_controller, &glm::ChatController::setParams);
    m_controller->setParams(m_paramPanel->params());   // 初始参数

    connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    updateButtonByState(m_controller->state());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSendClicked()
{
    using S = glm::ChatController::State;
    const auto st = m_controller->state();
    if (st == S::Sending || st == S::Streaming) {
        m_controller->stop();   // 生成中:点 = 停止
        return;
    }
    const QString text = m_inputEdit->toPlainText().trimmed();
    if (text.isEmpty()) return;
    m_inputEdit->clear();
    m_controller->send(text);   // Controller 加历史 + messageAppended → ChatModel 自动更新
}

void MainWindow::onStateChanged(glm::ChatController::State s)
{
    using S = glm::ChatController::State;
    const bool generating = (s == S::Sending || s == S::Streaming);
    m_sendBtn->setText(generating ? tr("停止") : tr("发送"));
    m_inputEdit->setEnabled(!generating);

    QString status;
    switch (s) {
    case S::Idle:     status = tr("就绪"); break;
    case S::Sending:  status = tr("发送中..."); break;
    case S::Streaming:status = tr("生成中..."); break;
    case S::Finished: status = tr("完成"); break;
    case S::Error:    status = tr("出错"); break;
    case S::Aborted:  status = tr("已中断"); break;
    }
    m_statusLabel->setText(status);
}

void MainWindow::onErrorOccurred(const QString &error)
{
    m_statusLabel->setText(tr("错误: ") + error);
}

void MainWindow::updateButtonByState(glm::ChatController::State s)
{
    using S = glm::ChatController::State;
    const bool generating = (s == S::Sending || s == S::Streaming);
    m_sendBtn->setText(generating ? tr("停止") : tr("发送"));
    m_sendBtn->setEnabled(true);
}
