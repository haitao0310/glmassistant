cmake_minimum_required(VERSION 3.16)
project(GlmAssistant VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6: Widgets(UI) + Network(HTTP/SSE) + Sql(SQLite) + Qml(QJSEngine,Agent 算表达式) + Test
find_package(Qt6 REQUIRED COMPONENTS Widgets Network Sql Qml Test)
qt_standard_project_setup()

qt_add_executable(GlmAssistant
    main.cpp
    src/app/ChatController.cpp src/app/ChatController.h
    src/app/ChatModel.cpp src/app/ChatModel.h
    src/app/SessionManager.cpp src/app/SessionManager.h
    src/app/DebugController.cpp src/app/DebugController.h
    src/core/LlmTypes.cpp src/core/LlmTypes.h src/core/LlmReply.h src/core/ILlmProvider.h src/core/ProviderRegistry.cpp src/core/ProviderRegistry.h
    src/data/DatabaseManager.cpp src/data/DatabaseManager.h
    src/network/HttpClient.cpp src/network/HttpClient.h src/network/SseParser.cpp src/network/SseParser.h
    src/providers/common/ProviderUtils.cpp src/providers/common/ProviderUtils.h
    src/providers/GlmProvider.cpp src/providers/GlmProvider.h
    src/providers/OllamaProvider.cpp src/providers/OllamaProvider.h
    src/providers/CachingProvider.cpp src/providers/CachingProvider.h
    src/infrastructure/Logger.cpp src/infrastructure/Logger.h src/infrastructure/Constants.h src/infrastructure/Error.h
    src/infrastructure/SettingsManager.cpp src/infrastructure/SettingsManager.h
    src/infrastructure/ThemeManager.cpp src/infrastructure/ThemeManager.h
    src/ui/ChatWidget.cpp src/ui/ChatWidget.h
    forms/chatwidget.ui
    forms/parampanel.ui
    forms/debugview.ui
    src/ui/ParamPanel.cpp src/ui/ParamPanel.h
    src/ui/DebugView.cpp src/ui/DebugView.h
    src/ui/MarkdownRenderer.cpp src/ui/MarkdownRenderer.h
    src/agent/ToolRegistry.cpp src/agent/ToolRegistry.h src/agent/ITool.h src/agent/TimeTool.h src/agent/CalculatorTool.cpp src/agent/CalculatorTool.h
    src/agent/AgentController.cpp src/agent/AgentController.h
    src/rag/VectorStore.cpp src/rag/VectorStore.h src/rag/IEmbeddingProvider.h src/rag/GlmEmbeddingProvider.cpp src/rag/GlmEmbeddingProvider.h src/rag/RetrievalTool.cpp src/rag/RetrievalTool.h
    src/middleware/MiddlewarePipeline.cpp src/middleware/MiddlewarePipeline.h src/middleware/IMiddleware.h src/middleware/TokenCounterMiddleware.h src/middleware/ContextTruncationMiddleware.h src/middleware/LoggingMiddleware.h
    mainwindow.cpp mainwindow.h mainwindow.ui
)
target_include_directories(GlmAssistant PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(GlmAssistant PRIVATE Qt6::Widgets Qt6::Network Qt6::Sql Qt6::Qml)
set_target_properties(GlmAssistant PROPERTIES AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/forms)

# 单元测试
enable_testing()
add_executable(tst_sseparser
    tests/tst_sseparser.cpp
    src/network/SseParser.cpp
    src/infrastructure/Logger.cpp)
target_include_directories(tst_sseparser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(tst_sseparser PRIVATE Qt6::Test Qt6::Core)
add_test(NAME tst_sseparser COMMAND tst_sseparser)
