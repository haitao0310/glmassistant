#include "mainwindow.h"

#include "src/infrastructure/Logger.h"
#include "src/infrastructure/Constants.h"
#include "src/network/HttpClient.h"
#include "src/providers/GlmProvider.h"
#include "src/app/ChatController.h"
#include "src/app/SessionManager.h"
#include "src/data/DatabaseManager.h"

#include <QApplication>
#include <QByteArray>
#include <QString>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QString::fromUtf8(glm::constants::APP_NAME));
    a.setApplicationVersion(QString::fromUtf8(glm::constants::APP_VERSION));

    glm::Logger::install();

    // 打开 SQLite(%APPDATA%/GlmAssistant/glmassistant.db)
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString dbPath = dataDir + QStringLiteral("/glmassistant.db");
    if (!glm::DatabaseManager::instance().open(dbPath)) {
        glm::logError("main", QStringLiteral("DB open failed: %1").arg(dbPath));
    }

    // API key 从环境变量(不进代码,防泄露)
    const QByteArray apiKey = qgetenv(glm::constants::GLM_API_ENV_KEY);
    if (apiKey.isEmpty()) {
        glm::logError("main", QStringLiteral("%1 未设置,窗口内会提示")
                          .arg(QString::fromUtf8(glm::constants::GLM_API_ENV_KEY)));
    }

    // 依赖组装(ADR-008)
    auto *http = new glm::HttpClient(&a);
    auto *provider = new glm::GlmProvider(QString::fromLocal8Bit(apiKey), http, &a);
    auto *sessions = new glm::SessionManager(&a);
    auto *controller = new glm::ChatController(provider, sessions, &a);

    MainWindow w(controller, sessions);
    w.show();
    return a.exec();
}
