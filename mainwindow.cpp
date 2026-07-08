#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QTextCursor>

MainWindow::MainWindow(glm::ChatController *controller, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_controller(controller)
{
    ui->setupUi(this);
    setWindowTitle(tr("GlmAssistant - P1 流式"));

    // P1:手写中央 UI(P3 拆到独立 ChatWidget + Model/View)
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    m_inputEdit = new QTextEdit;
    m_inputEdit->setPlaceholderText(tr("输入消息..."));
    m_outputEdit = new QTextEdit;
    m_outputEdit->setPlaceholderText(tr("GLM 回复显示在这里..."));
    m_outputEdit->setReadOnly(true);
    m_sendBtn = new QPushButton(tr("发送"));

    layout->addWidget(new QLabel(tr("输入:")));
    layout->addWidget(m_inputEdit);
    layout->addWidget(m_sendBtn);
    layout->addWidget(new QLabel(tr("回复:")));
    layout->addWidget(m_outputEdit);

    setCentralWidget(central);

    // 连接 Controller 信号(UI 只接 Controller,不碰 Provider)
    connect(m_controller, &glm::ChatController::chunkReceived, this, &MainWindow::onChunkReceived);
    connect(m_controller, &glm::ChatController::finished, this, &MainWindow::onFinished);
    connect(m_controller, &glm::ChatController::errorOccurred, this, &MainWindow::onErrorOccurred);
    connect(m_controller, &glm::ChatController::stateChanged, this, &MainWindow::onStateChanged);

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
    const auto state = m_controller->state();
    if (state == S::Streaming || state == S::Sending) {
        m_controller->stop();   // 生成中:点 = 停止
        return;
    }
    // 空闲/终态:点 = 发送
    const QString text = m_inputEdit->toPlainText().trimmed();
    if (text.isEmpty()) return;

    m_outputEdit->append(tr("我: ") + text);
    m_inputEdit->clear();
    m_outputEdit->append(QStringLiteral("GLM: "));   // 占位行,流式 chunk 追加到此行末
    m_controller->send(text);
}

void MainWindow::onChunkReceived(const QString &text)
{
    // 打字机:增量追加到末尾("GLM: " 行后)
    m_outputEdit->moveCursor(QTextCursor::End);
    m_outputEdit->insertPlainText(text);
}

void MainWindow::onFinished(const QString &fullText)
{
    Q_UNUSED(fullText);
    m_outputEdit->append(QString());   // 回复结束,空行分隔
}

void MainWindow::onErrorOccurred(const QString &error)
{
    m_outputEdit->append(tr("[错误] ") + error);
}

void MainWindow::onStateChanged(glm::ChatController::State s)
{
    updateButtonByState(s);
}

void MainWindow::updateButtonByState(glm::ChatController::State s)
{
    using S = glm::ChatController::State;
    const bool generating = (s == S::Sending || s == S::Streaming);
    m_sendBtn->setText(generating ? tr("停止") : tr("发送"));
    m_sendBtn->setEnabled(true);
    m_inputEdit->setEnabled(!generating);   // 生成中禁输入,防乱
}
