#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "src/app/SessionManager.h"
#include "src/ui/ParamPanel.h"
#include "src/ui/DebugView.h"
#include "src/infrastructure/ThemeManager.h"

#include <QTabWidget>
#include <QListWidget>
#include <QTextBrowser>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QTextCursor>

MainWindow::MainWindow(glm::ChatController *controller, glm::SessionManager *sessions,
                       glm::DebugController *debug, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_controller(controller)
    , m_sessions(sessions)
{
    ui->setupUi(this);
    setWindowTitle(tr("GlmAssistant - P4 对话 + 调试双模式"));

    m_paramPanel = new glm::ParamPanel(this);
    m_debugView = new glm::DebugView(debug, this);
    m_tabs = new QTabWidget(this);
    m_tabs->addTab(buildChatTab(), tr("对话"));
    m_tabs->addTab(m_debugView, tr("调试"));
    setCentralWidget(m_tabs);

    // Controller → UI
    connect(m_controller, &glm::ChatController::messageAppended, this, [this](const glm::Message &){ rerenderChat(); });
    connect(m_controller, &glm::ChatController::chunkReceived, this, [this](const QString &){ rerenderChat(); });
    connect(m_controller, &glm::ChatController::historyReplaced, this, [this](const QList<glm::Message> &){ rerenderChat(); });
    connect(m_controller, &glm::ChatController::stateChanged, this, &MainWindow::onStateChanged);
    connect(m_controller, &glm::ChatController::errorOccurred, this, &MainWindow::onErrorOccurred);

    connect(m_paramPanel, &glm::ParamPanel::paramsChanged, m_controller, &glm::ChatController::setParams);
    m_controller->setParams(m_paramPanel->params());

    connect(m_sessions, &glm::SessionManager::sessionListChanged, this, &MainWindow::refreshSessionList);
    connect(m_sessions, &glm::SessionManager::currentChanged, this, &MainWindow::onCurrentSessionChanged);
    connect(m_sessions, &glm::SessionManager::messagesLoaded, m_controller, &glm::ChatController::setSession);

    refreshSessionList();
    rerenderChat();
    updateButtonByState(m_controller->state());
}

QWidget *MainWindow::buildChatTab()
{
    m_sessionList = new QListWidget;
    m_chatView = new QTextBrowser;
    m_chatView->setOpenExternalLinks(true);
    m_inputEdit = new QTextEdit;
    m_inputEdit->setMaximumHeight(80);
    m_inputEdit->setPlaceholderText(tr("输入消息..."));
    m_sendBtn = new QPushButton(tr("发送"));
    m_newSessionBtn = new QPushButton(tr("新建会话"));
    m_delSessionBtn = new QPushButton(tr("删除"));
    m_themeBtn = new QPushButton(tr("切换主题"));
    m_statusLabel = new QLabel(tr("就绪"));

    auto *chatWidget = new QWidget;
    auto *outer = new QHBoxLayout(chatWidget);

    auto *left = new QVBoxLayout;
    left->addWidget(new QLabel(tr("会话")));
    left->addWidget(m_sessionList);
    left->addWidget(m_newSessionBtn);
    left->addWidget(m_delSessionBtn);
    outer->addLayout(left);

    auto *center = new QVBoxLayout;
    center->addWidget(m_chatView, 1);
    center->addWidget(m_inputEdit);
    auto *btnRow = new QHBoxLayout;
    btnRow->addWidget(m_themeBtn);
    btnRow->addWidget(m_statusLabel);
    btnRow->addStretch();
    btnRow->addWidget(m_sendBtn);
    center->addLayout(btnRow);
    outer->addLayout(center, 1);

    outer->addWidget(m_paramPanel);

    connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    connect(m_newSessionBtn, &QPushButton::clicked, this, [this]{ m_sessions->newSession(); });
    connect(m_delSessionBtn, &QPushButton::clicked, this, [this]{ m_sessions->deleteCurrent(); });
    connect(m_themeBtn, &QPushButton::clicked, this, [this]{
        const auto next = (glm::ThemeManager::current() == glm::ThemeManager::Theme::Light)
                          ? glm::ThemeManager::Theme::Dark : glm::ThemeManager::Theme::Light;
        glm::ThemeManager::apply(next, qApp);
    });
    connect(m_sessionList, &QListWidget::currentRowChanged, this, [this](int row){
        const auto ss = m_sessions->sessions();
        if (row >= 0 && row < ss.size()) m_sessions->switchTo(ss.at(row).id);
    });

    return chatWidget;
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::onSendClicked()
{
    using S = glm::ChatController::State;
    const auto st = m_controller->state();
    if (st == S::Sending || st == S::Streaming) { m_controller->stop(); return; }
    const QString text = m_inputEdit->toPlainText().trimmed();
    if (text.isEmpty()) return;
    m_inputEdit->clear();
    m_controller->send(text);
}

void MainWindow::onStateChanged(glm::ChatController::State s)
{
    using S = glm::ChatController::State;
    m_inputEdit->setEnabled(!(s == S::Sending || s == S::Streaming));
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
    updateButtonByState(s);
}

void MainWindow::onErrorOccurred(const QString &error)
{
    m_statusLabel->setText(tr("错误: ") + error);
}

void MainWindow::refreshSessionList()
{
    m_sessionList->blockSignals(true);
    m_sessionList->clear();
    const auto ss = m_sessions->sessions();
    const QString cur = m_sessions->currentSessionId();
    int curRow = -1;
    for (int i = 0; i < ss.size(); ++i) {
        m_sessionList->addItem(ss.at(i).title.isEmpty() ? tr("(无标题)") : ss.at(i).title);
        if (ss.at(i).id == cur) curRow = i;
    }
    if (curRow >= 0) m_sessionList->setCurrentRow(curRow);
    m_sessionList->blockSignals(false);
}

void MainWindow::onCurrentSessionChanged(const QString &id)
{
    Q_UNUSED(id);
    refreshSessionList();
}

void MainWindow::rerenderChat()
{
    const auto hist = m_controller->history();
    QString md;
    for (const auto &m : hist) {
        if (m.role == glm::Role::User) {
            md += QStringLiteral("**我:** ") + m.content + QStringLiteral("\n\n");
        } else if (m.role == glm::Role::Assistant) {
            md += QStringLiteral("**GLM:** ") + m.content + QStringLiteral("\n\n");
        }
    }
    m_chatView->setMarkdown(md);
    m_chatView->moveCursor(QTextCursor::End);
}

void MainWindow::updateButtonByState(glm::ChatController::State s)
{
    using S = glm::ChatController::State;
    const bool gen = (s == S::Sending || s == S::Streaming);
    m_sendBtn->setText(gen ? tr("停止") : tr("发送"));
    m_sendBtn->setEnabled(true);
}
