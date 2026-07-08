#include "mainwindow.h"

#include "src/infrastructure/Logger.h"
#include "src/infrastructure/Constants.h"
#include "src/network/HttpClient.h"
#include "src/providers/GlmProvider.h"
#include "src/providers/OllamaProvider.h"
#include "src/app/ChatController.h"
#include "src/app/SessionManager.h"
#include "src/app/DebugController.h"
#include "src/agent/AgentController.h"
#include "src/agent/ToolRegistry.h"
#include "src/agent/TimeTool.h"
#include "src/agent/CalculatorTool.h"
#include "src/core/ProviderRegistry.h"
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

    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString dbPath = dataDir + QStringLiteral("/glmassistant.db");
    if (!glm::DatabaseManager::instance().open(dbPath)) {
        glm::logError("main", QStringLiteral("DB open failed: %1").arg(dbPath));
    }

    const QByteArray apiKey = qgetenv(glm::constants::GLM_API_ENV_KEY);
    if (apiKey.isEmpty()) {
        glm::logError("main", QStringLiteral("%1 未设置,窗口内会提示")
                          .arg(QString::fromUtf8(glm::constants::GLM_API_ENV_KEY)));
    }

    auto *http = new glm::HttpClient(&a);
    auto *provider = new glm::GlmProvider(QString::fromLocal8Bit(apiKey), http, &a);

    // P5 扩展点注册:工具(Agent)+ Provider(多厂商)
    static glm::TimeTool s_timeTool;
    static glm::CalculatorTool s_calcTool;
    glm::ToolRegistry::instance().registerTool(&s_timeTool);
    glm::ToolRegistry::instance().registerTool(&s_calcTool);
    glm::ProviderRegistry::instance().registerProvider(provider);

    auto *sessions = new glm::SessionManager(&a);
    auto *debug = new glm::DebugController(provider, sessions, &a);
    auto *agent = new glm::AgentController(provider, &a);
    auto *controller = new glm::ChatController(provider, sessions, debug, &a);

    MainWindow w(controller, sessions, debug);
    w.show();
    return a.exec();
}
