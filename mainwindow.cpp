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

    //=== API Key:从环境变量读(不硬编码,防随代码进 git 泄露)===
    //设法:Qt Creator → 左侧 Projects → Run → Run Environment → 添加 GLM_API_KEY=你的key
    //或 Windows 系统环境变量新建 GLM_API_KEY(改后重启 Qt Creator)
    const QByteArray envKey = qgetenv("GLM_API_KEY");
    if (envKey.isEmpty()) {
        m_outputEdit->append(QStringLiteral("[配置错误] 未设置环境变量 GLM_API_KEY"));
        m_outputEdit->append(QStringLiteral("  Qt Creator: Projects → Run → Run Environment → 添加 GLM_API_KEY=你的key"));
        m_outputEdit->append(QStringLiteral("  或 Windows 系统环境变量新建 GLM_API_KEY(改后重启 Qt Creator)"));
        m_sendBtn->setEnabled(false);
    } else {
        m_glm = new GlmClient(QString::fromLocal8Bit(envKey), this);
        connect(m_glm, &GlmClient::replyReceived, this, &MainWindow::onReplyReceived);
        connect(m_glm, &GlmClient::errorOccurred, this, &MainWindow::onErrorOccurred);
    }
    connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::onSendClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSendClicked()
{
    if (!m_glm) return;   //key 未配置时 m_glm 为空,防崩溃
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
