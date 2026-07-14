#include "DebugView.h"

#include "../app/DebugController.h"
#include "../core/LlmReply.h"

#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QDateTime>
#include <QJsonDocument>

namespace glm {

DebugView::DebugView(DebugController *ctrl, QWidget *parent)
    : QWidget(parent)
    , m_ctrl(ctrl)
{
    m_historyList = new QListWidget;
    m_requestView = new QPlainTextEdit;
    m_requestView->setReadOnly(false);   // 可编辑:改参后重放
    m_responseView = new QPlainTextEdit;
    m_responseView->setReadOnly(true);
    m_replayBtn = new QPushButton(tr("重放选中"));

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("历史请求")));
    layout->addWidget(m_historyList, 1);
    layout->addWidget(new QLabel(tr("请求 body(可编辑后重放)")));
    layout->addWidget(m_requestView);
    layout->addWidget(new QLabel(tr("响应")));
    layout->addWidget(m_responseView);
    layout->addWidget(m_replayBtn);

    connect(m_historyList, &QListWidget::currentRowChanged, this, &DebugView::onSelectionChanged);
    connect(m_replayBtn, &QPushButton::clicked, this, &DebugView::onReplay);
    connect(m_ctrl, &DebugController::historyChanged, this, [this]{ refresh(); });

    refresh();
}

void DebugView::refresh()
{
    m_historyList->blockSignals(true);
    m_historyList->clear();
    m_records = m_ctrl->history();
    for (const auto &r : m_records) {
        const QString ts = QDateTime::fromMSecsSinceEpoch(r.timestamp).toString(QStringLiteral("MM-dd hh:mm:ss"));
        m_historyList->addItem(QStringLiteral("#%1 %2 %3").arg(r.id).arg(ts, r.model));
    }
    m_historyList->blockSignals(false);
    if (!m_records.isEmpty()) m_historyList->setCurrentRow(0);
}

void DebugView::onSelectionChanged(int row)
{
    if (row < 0 || row >= m_records.size()) return;
    const auto &r = m_records.at(row);
    const QJsonDocument reqDoc = QJsonDocument::fromJson(r.rawRequest.toUtf8());
    m_requestView->setPlainText(reqDoc.isNull()
        ? r.rawRequest
        : QString::fromUtf8(reqDoc.toJson(QJsonDocument::Indented)));
    m_responseView->setPlainText(r.rawResponse);
}

void DebugView::onReplay()
{
    const int row = m_historyList->currentRow();
    if (row < 0 || row >= m_records.size()) return;
    RequestRecord r = m_records.at(row);
    r.rawRequest = m_requestView->toPlainText();   // 用编辑版重放
    m_responseView->setPlainText(tr("[重放中...]"));

    LlmReply *reply = m_ctrl->replay(r);
    if (!reply) return;
    QObject::connect(reply, &LlmReply::finished, this, [this, reply](const QString &fullText){
        m_responseView->setPlainText(
            QStringLiteral("【回复】") + fullText +
            QStringLiteral("\n\n【原始 SSE】\n") + reply->rawResponse()
        );
        // 不 refresh():重放请求没记入历史库,refresh 会选中第一条旧记录覆盖重放回复
    });
    QObject::connect(reply, &LlmReply::errorOccurred, this, [this](const QString &e){
        m_responseView->setPlainText(tr("[重放错误] ") + e);
    });
    QObject::connect(reply, &LlmReply::done, reply, &QObject::deleteLater);
}

} // namespace glm
