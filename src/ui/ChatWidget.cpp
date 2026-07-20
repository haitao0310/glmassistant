#include "ChatWidget.h"
#include "ui_chatwidget.h"

#include "../app/SessionManager.h"
#include "../ui/ParamPanel.h"
#include "../infrastructure/ThemeManager.h"
#include "../infrastructure/SettingsManager.h"

#include <QTextCursor>
#include <QApplication>

ChatWidget::ChatWidget(glm::ChatController *controller, glm::SessionManager *sessions, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatWidget)
    , m_controller(controller)
    , m_sessions(sessions)
{
    ui->setupUi(this);

    // Controller → UI
    connect(m_controller, &glm::ChatController::messageAppended, this, [this](const glm::Message &){ rerenderChat(); });
    connect(m_controller, &glm::ChatController::chunkReceived, this, [this](const QString &){ rerenderChat(); });
    connect(m_controller, &glm::ChatController::historyReplaced, this, [this](const QList<glm::Message> &){ rerenderChat(); });
    connect(m_controller, &glm::ChatController::stateChanged, this, &ChatWidget::onStateChanged);
    connect(m_controller, &glm::ChatController::errorOccurred, this, &ChatWidget::onErrorOccurred);
    connect(m_controller, &glm::ChatController::tokenReported, this, [this](int p, int c, int t){
        m_totalTokens += t;
        m_lastTokenStr = QStringLiteral("↑%1 ↓%2 = %3 tokens | 累计 %4")
                             .arg(p).arg(c).arg(t).arg(m_totalTokens);
    });

    // ParamPanel → Controller + QSettings
    connect(ui->paramPanel, &glm::ParamPanel::paramsChanged, m_controller, &glm::ChatController::setParams);
    connect(ui->paramPanel, &glm::ParamPanel::paramsChanged, this, [](const glm::GenerationParams &p){
        glm::SettingsManager::instance().saveParams(p);
    });
    const glm::GenerationParams saved = glm::SettingsManager::instance().params();
    ui->paramPanel->setParams(saved);
    m_controller->setParams(saved);

    // SessionManager → UI
    connect(m_sessions, &glm::SessionManager::sessionListChanged, this, &ChatWidget::refreshSessionList);
    connect(m_sessions, &glm::SessionManager::currentChanged, this, &ChatWidget::onCurrentSessionChanged);
    connect(m_sessions, &glm::SessionManager::messagesLoaded, m_controller, &glm::ChatController::setSession);

    // 按钮
    connect(ui->sendBtn, &QPushButton::clicked, this, &ChatWidget::onSendClicked);
    connect(ui->newSessionBtn, &QPushButton::clicked, this, [this]{ m_sessions->newSession(); });
    connect(ui->delSessionBtn, &QPushButton::clicked, this, [this]{ m_sessions->deleteCurrent(); });
    connect(ui->themeBtn, &QPushButton::clicked, this, [this]{
        const auto next = (glm::ThemeManager::current() == glm::ThemeManager::Theme::Light)
                          ? glm::ThemeManager::Theme::Dark : glm::ThemeManager::Theme::Light;
        glm::ThemeManager::apply(next, qApp);
        glm::SettingsManager::instance().saveTheme(next);
    });
    connect(ui->sessionList, &QListWidget::currentRowChanged, this, [this](int row){
        const auto ss = m_sessions->sessions();
        if (row >= 0 && row < ss.size()) {
            const QString id = ss.at(row).id;
            m_sessions->switchTo(id);
            glm::SettingsManager::instance().saveLastSessionId(id);
        }
    });

    refreshSessionList();
    rerenderChat();
    updateButtonByState(m_controller->state());
}

ChatWidget::~ChatWidget() { delete ui; }

void ChatWidget::onSendClicked()
{
    using S = glm::ChatController::State;
    const auto st = m_controller->state();
    if (st == S::Sending || st == S::Streaming) { m_controller->stop(); return; }
    const QString text = ui->inputEdit->toPlainText().trimmed();
    if (text.isEmpty()) return;
    ui->inputEdit->clear();
    m_controller->send(text);
}

void ChatWidget::onStateChanged(glm::ChatController::State s)
{
    using S = glm::ChatController::State;
    ui->inputEdit->setEnabled(!(s == S::Sending || s == S::Streaming));
    QString status;
    switch (s) {
    case S::Idle:     status = tr("就绪"); break;
    case S::Sending:  status = tr("发送中..."); break;
    case S::Streaming:status = tr("生成中..."); break;
    case S::Finished: status = m_lastTokenStr.isEmpty() ? tr("完成") : m_lastTokenStr; break;
    case S::Error:    status = tr("出错"); break;
    case S::Aborted:  status = tr("已中断"); break;
    }
    ui->statusLabel->setText(status);
    updateButtonByState(s);
}

void ChatWidget::onErrorOccurred(const QString &error)
{
    ui->statusLabel->setText(tr("错误: ") + error);
}

void ChatWidget::refreshSessionList()
{
    ui->sessionList->blockSignals(true);
    ui->sessionList->clear();
    const auto ss = m_sessions->sessions();
    const QString cur = m_sessions->currentSessionId();
    int curRow = -1;
    for (int i = 0; i < ss.size(); ++i) {
        ui->sessionList->addItem(ss.at(i).title.isEmpty() ? tr("(无标题)") : ss.at(i).title);
        if (ss.at(i).id == cur) curRow = i;
    }
    if (curRow >= 0) ui->sessionList->setCurrentRow(curRow);
    ui->sessionList->blockSignals(false);
}

void ChatWidget::onCurrentSessionChanged(const QString &id)
{
    Q_UNUSED(id);
    refreshSessionList();
}

void ChatWidget::rerenderChat()
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
    ui->chatView->setMarkdown(md);
    ui->chatView->moveCursor(QTextCursor::End);
}

void ChatWidget::updateButtonByState(glm::ChatController::State s)
{
    using S = glm::ChatController::State;
    const bool gen = (s == S::Sending || s == S::Streaming);
    ui->sendBtn->setText(gen ? tr("停止") : tr("发送"));
    ui->sendBtn->setEnabled(true);
}
