#include "DebugView.h"
#include "ui_debugview.h"

#include "../app/DebugController.h"
#include "../core/LlmReply.h"

#include <QDateTime>
#include <QJsonDocument>

namespace glm {

DebugView::DebugView(DebugController *ctrl, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DebugView)
    , m_ctrl(ctrl)
{
    ui->setupUi(this);

    connect(ui->historyList, &QListWidget::currentRowChanged, this, &DebugView::onSelectionChanged);
    connect(ui->replayBtn, &QPushButton::clicked, this, &DebugView::onReplay);
    connect(m_ctrl, &DebugController::historyChanged, this, [this]{ refresh(); });

    refresh();
}

void DebugView::refresh()
{
    ui->historyList->blockSignals(true);
    ui->historyList->clear();
    m_records = m_ctrl->history();
    for (const auto &r : m_records) {
        const QString ts = QDateTime::fromMSecsSinceEpoch(r.timestamp).toString(QStringLiteral("MM-dd hh:mm:ss"));
        ui->historyList->addItem(QStringLiteral("#%1 %2 %3").arg(r.id).arg(ts, r.model));
    }
    ui->historyList->blockSignals(false);
    if (!m_records.isEmpty()) ui->historyList->setCurrentRow(0);
}

void DebugView::onSelectionChanged(int row)
{
    if (row < 0 || row >= m_records.size()) return;
    const auto &r = m_records.at(row);
    const QJsonDocument reqDoc = QJsonDocument::fromJson(r.rawRequest.toUtf8());
    ui->requestView->setPlainText(reqDoc.isNull()
        ? r.rawRequest
        : QString::fromUtf8(reqDoc.toJson(QJsonDocument::Indented)));
    ui->responseView->setPlainText(r.rawResponse);
}

void DebugView::onReplay()
{
    const int row = ui->historyList->currentRow();
    if (row < 0 || row >= m_records.size()) return;
    RequestRecord r = m_records.at(row);
    r.rawRequest = ui->requestView->toPlainText();
    ui->responseView->setPlainText(tr("[重放中...]"));

    LlmReply *reply = m_ctrl->replay(r);
    if (!reply) return;
    QObject::connect(reply, &LlmReply::finished, this, [this, reply](const QString &fullText){
        ui->responseView->setPlainText(
            QStringLiteral("【回复】") + fullText +
            QStringLiteral("\n\n【原始 SSE】\n") + reply->rawResponse()
        );
    });
    QObject::connect(reply, &LlmReply::errorOccurred, this, [this](const QString &e){
        ui->responseView->setPlainText(tr("[重放错误] ") + e);
    });
    QObject::connect(reply, &LlmReply::done, reply, &QObject::deleteLater);
}

} // namespace glm
