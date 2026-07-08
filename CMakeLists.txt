cmake_minimum_required(VERSION 3.16)

project(GlmAssistant VERSION 1.0.0 LANGUAGES CXX)

# C++17(Qt 6 标准)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 找 Qt6 模块:Widgets(UI) + Network(HTTP/HTTPS)
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# Qt 6 标准项目设置(自动启用 AUTOMOC/AUTOUIC/AUTORCC,Qt 6.3+)
qt_standard_project_setup()

# 可执行文件(源码 + 头 + UI 一起列,AUTOMOC/AUTOUIC 自动处理)
qt_add_executable(GlmAssistant
    main.cpp
    mainwindow.cpp
    glmclient.cpp
    mainwindow.h
    glmclient.h
    mainwindow.ui
)

# 链接 Qt 模块
target_link_libraries(GlmAssistant PRIVATE
    Qt6::Widgets
    Qt6::Network
)
