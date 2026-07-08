#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("GlmAssistant - 第一步 调通 API"));

    //第一步:手写中央 UI(后面模块化时拆到独立 ChatWidget)
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    m_inputEdit = new QTextEdit;
    m_inputEdit->setPlaceholderText(QStringLiteral("输入消息..."));
    m_outputEdit = new QTextEdit;
    m_outputEdit->setPlaceholderText(QStringLiteral("GLM 回复显示在这里..."));
    m_outputEdit->setReadOnly(true);
    m_sendBtn = new QPushButton(QStringLiteral("发送"));

    layout->addWidget(new QLabel(QStringLiteral("输入:")));
    layout->addWidget(m_inputEdit);
    layout->addWidget(m_sendBtn);
    layout->addWidget(new QLabel(QStringLiteral("回复:")));
    layout->addWidget(m_outputEdit);

    setCentralWidget(central);

    //=== 填你的 GLM API Key ===
    //把下面字符串换成 bigmodel.cn 控制台复制的真实 key
    //(后面做"API Key 加密存储"模块时改用 QSettings + QCryptographicHash,不硬编码)
    m_glm = new GlmClient(QStringLiteral("1983eca8588b4ab0a997d07f8e13f490.JAASIAAmWs2pJIKR"), this);

    connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    connect(m_glm, &GlmClient::replyReceived, this, &MainWindow::onReplyReceived);
    connect(m_glm, &GlmClient::errorOccurred, this, &MainWindow::onErrorOccurred);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSendClicked()
{
    const QString text = m_inputEdit->toPlainText().trimmed();
    if(text.isEmpty()) return;

    m_outputEdit->append(QStringLiteral("我: ") + text);
    m_inputEdit->clear();
    m_sendBtn->setEnabled(false);   //等回复期间禁用,防重复点
    m_glm->sendMessage(text);
}

void MainWindow::onReplyReceived(const QString &content)
{
    m_outputEdit->append(QStringLiteral("GLM: ") + content + QStringLiteral("\n"));
    m_sendBtn->setEnabled(true);
}

void MainWindow::onErrorOccurred(const QString &error)
{
    m_outputEdit->append(QStringLiteral("[错误] ") + error);
    m_sendBtn->setEnabled(true);
}
