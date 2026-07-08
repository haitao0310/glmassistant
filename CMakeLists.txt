cmake_minimum_required(VERSION 3.16)
project(GlmAssistant VERSION 0.1.0 LANGUAGES CXX)

# C++17(Qt 6 标准)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 模块:Widgets(UI) + Network(HTTP/SSE) + Test(单测)
find_package(Qt6 REQUIRED COMPONENTS Widgets Network Test)

# Qt 6 标准项目设置(自动 AUTOMOC/AUTOUIC/AUTORCC)
qt_standard_project_setup()

# 主程序(P1:分层 src/ 目录)
qt_add_executable(GlmAssistant
    main.cpp
    src/app/ChatController.cpp src/app/ChatController.h
    src/app/ChatModel.cpp src/app/ChatModel.h
    src/ui/ParamPanel.cpp src/ui/ParamPanel.h
    src/core/LlmTypes.cpp src/core/LlmTypes.h src/core/LlmReply.h src/core/ILlmProvider.h
    src/network/HttpClient.cpp src/network/HttpClient.h src/network/SseParser.cpp src/network/SseParser.h
    src/providers/GlmProvider.cpp src/providers/GlmProvider.h
    src/infrastructure/Logger.cpp src/infrastructure/Logger.h src/infrastructure/Constants.h src/infrastructure/Error.h
    mainwindow.cpp mainwindow.h mainwindow.ui
)
target_include_directories(GlmAssistant PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(GlmAssistant PRIVATE Qt6::Widgets Qt6::Network)

# 单元测试(独立可执行 + ctest 注册)
enable_testing()
add_executable(tst_sseparser
    tests/tst_sseparser.cpp
    src/network/SseParser.cpp)
target_include_directories(tst_sseparser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(tst_sseparser PRIVATE Qt6::Test Qt6::Core)
add_test(NAME tst_sseparser COMMAND tst_sseparser)
