# GlmAssistant 产品需求文档 (PRD)

> v3 · 2026-07-08 · 定位:LLM 开发者工作站

## 1. 产品概述
面向 AI 开发者的**本地桌面 LLM 工作站**。日常当对话客户端(参数透明 / token 统计 / 历史持久化);开发时切调试模式(构造请求 / 看原始 JSON / 调参 / 对比模型)。调智谱 GLM API(OpenAI 兼容),Qt 6 / C++。

**差异化**:不是 ChatGPT clone(红海),是 LLM 开发者工具(蓝海)——对话 + 调试双模式。

## 2. 目标用户
AI 应用开发者:日常用 LLM + 开发时需调试 API / prompt / 参数的人。

## 3. 核心功能

### 对话模式 (P1-P3,先做,保证能用)
- SSE 流式回复 + 中断
- 多轮上下文 + token 管理(超限截断)
- 多会话 + SQLite 持久化
- Markdown 渲染(代码块 / 列表 / 表格)
- 参数面板(temperature / top_p / max_tokens / model)
- token / 延迟统计
- 多 endpoint(GLM + 自定义 OpenAI 兼容)

### 调试模式 (P4,差异化护城河)
- 原始请求 JSON 查看 / 编辑
- 原始响应 JSON(含 SSE 帧展开)
- 请求重放
- 多模型 / 多参数对比
- 历史请求库(搜索 / 标签 / 重放)

### 基础设施(贯穿)
- API key:环境变量(P0)→ 加密存储(P4)
- 设置:主题 / 语言 / endpoint 管理

## 4. 非功能需求
- 性能:首 token <1s,流式不卡,大历史加载 <500ms
- 安全:API key 不进代码、HTTPS 传输
- 平台:Windows 10/11(Qt 6.8.3 MinGW),架构留跨平台余地
- 可靠:断网重试、JSON 异常不崩、流式中断保留已生成部分

## 5. 范围边界
- 做:单机桌面、OpenAI 兼容 API(GLM + 可扩展)
- 不做:服务端、移动端、本地推理(P5 可选 Ollama 对接)、插件系统、多用户

## 6. 技术栈
Qt 6.8.3 LTS / C++17(留 C++20 余地) / CMake / MinGW 64-bit / Qt Network(Schannel HTTPS + SSE) / SQLite(Qt SQL) / Qt Widgets

## 7. 架构概览
分层 + Provider 抽象(详见 `docs/ARCHITECTURE.md`):
- **UI 层**:MainWindow / ChatWidget / SessionList / ParamPanel / DebugView
- **业务层**:ChatController / SessionManager(协调 UI ↔ 数据 ↔ 网络)
- **服务层**:LlmService + ILlmProvider(Glm / Ollama / OpenAI) + 中间件管道(token/截断/日志/重试/缓存)
- **数据层**:ChatSession / Message + DatabaseManager(SQLite) + SettingsManager
- **网络层**:HttpClient + SseParser

## 8. 路线图
- **P0** MVP 单轮对话 ✓
- **P1** SSE 流式 + 中断 + 架构骨架(Provider / SseParser / LlmReply)
- **P2** 多轮上下文 + 参数面板 + Model/View 重构
- **P3** 多会话 + SQLite + Markdown + 主题 / 动画
- **P4** 调试模式(原始 JSON / 重放 / 对比 / 历史库)
- **P5** Agent / 工具 / RAG + 多 Provider + 插件化

## 9. 成功标准
- 每期独立可交付运行
- 过 9 条防玩具 DoD(错误处理 / 分层 / 测试 / 可分发...)
- README + ADR 完整,面试可讲
- 核心约 1 万行(深度自然达到,非堆砌)

## 10. 规模与特性目标
1 万行 C++,覆盖 **Qt 全特性**(Widgets / Model-View / 信号槽跨线程 / 网络 / SQL / JSON / 多线程 / QSS / 动画 / 状态机 / 插件) + **C++ 全特性**(智能指针 / 移动语义 / 模板 / optional / variant / 并发 / RAII)。
