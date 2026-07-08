# GlmAssistant 架构设计

> v1 · 2026-07-08 · 对应 PRD v3

## 设计原则
1. **接口先行**:.h 契约先定,实现后补,实现不跑偏契约
2. **分层解耦**:UI / 业务 / 服务 / 数据 / 网络各司其职,**UI 不写业务逻辑**
3. **拓展性优先**:每个核心抽象都是扩展点(Provider / 中间件 / 工具 / 视图)
4. **依赖单向**:上层依赖下层,下层不反向依赖(避免环)

## 分层
```
UI 层          MainWindow · ChatWidget · SessionList · ParamPanel · DebugView(P4)
业务层         ChatController · SessionManager(P3) · Middlewares(P2)
服务层         LlmService · ILlmProvider · ProviderRegistry(P5)
数据层         ChatSession/Message · DatabaseManager(P3) · SettingsManager
网络层         HttpClient · SseParser
基础设施层     Logger · Result/Error · Constants · Config · Utils(横切关注点,所有层依赖)
```

## 四个扩展点(拓展性的根)

### 扩展点 1:Provider(ILlmProvider)
加新 LLM 厂商 → 实现 `ILlmProvider` + 注册 `ProviderRegistry` → **UI/业务零改动**。
- P1:GlmProvider
- P5:OllamaProvider / OpenAiProvider / 本地模型

### 扩展点 2:中间件管道(请求处理责任链)
请求发出前经过可插拔中间件:`token 计算 → 上下文截断 → 日志 → 重试 → 缓存`。
加新中间件 → 实现 `IMiddleware` + 插入管道 → 不动 Provider。
- P2:TokenCounter / ContextTruncator
- P4:RetryMiddleware / CacheMiddleware

### 扩展点 3:Agent 工具(P5)
LLM 可调外部工具 → 实现 `ITool` 注册 `ToolRegistry` → LLM 决策调用。
- 示例:计算器 / 搜索 / RAG 检索

### 扩展点 4:视图(UI 可扩展)
UI 用 Model/View,加新视图(对话 / 调试 / 对比)→ 独立 widget,接入 ChatController 信号。

## 数据流(流式请求)
```
用户输入
  → ChatController.buildRequest()
  → [中间件管道: token计算 → 截断 → 日志 → 重试]
  → LlmService.send() → ILlmProvider(GlmProvider).send()
  → HttpClient.post(SSE)
  → SseParser.feed(增量字节) → extractDeltaContent
  → LlmReply.chunkReceived
  → ChatController / UI 打字机显示
  → [DONE] → LlmReply.finished → 落库(P3)
```

## 目录结构
```
src/
├── core/            ILlmProvider.h, LlmTypes.h, LlmReply.h, ProviderRegistry(P5), IMiddleware(P2)
├── network/         HttpClient, SseParser
├── infrastructure/  Logger, Result/Error, Constants, Config, Utils(横切关注点)
├── providers/ GlmProvider(P1), OllamaProvider(P5), OpenAiProvider(P5)
├── data/      ChatSession, DatabaseManager(P3), SettingsManager
├── app/       ChatController(P2), SessionManager(P3), Middlewares(P2)
└── ui/        mainwindow, ChatWidget, SessionList, ParamPanel, DebugView(P4)
tests/         tst_sseparser(P1), tst_glmprovider(mock), ...
```

## ADR(架构决策记录)

### ADR-001 为什么用 ILlmProvider 抽象而非直接调 GLM
**决策**:所有 LLM 调用走 `ILlmProvider` 接口。
**理由**:多 provider 扩展(Ollama/OpenAI/本地)、测试可 mock、业务层不绑死厂商。
**代价**:多一层抽象。值得——P5 加 Ollama 零改业务层。

### ADR-002 为什么用 SSE 而非 WebSocket
**决策**:流式回复用 SSE(HTTP + `data:` 帧)。
**理由**:GLM/OpenAI API 原生 SSE(单向推送够用);WebSocket 双向,LLM 场景用不上且复杂。
**代价**:SSE 单向,但 LLM 流式正是单向推送场景。

### ADR-003 为什么 LlmReply 用信号而非回调/std::future
**决策**:`LlmReply` 继承 QObject,发 `chunkReceived/finished/error` 信号。
**理由**:Qt 信号槽天然异步 + 跨线程安全 + UI 绑定方便;回调易生命周期失控,future 不适合流式增量。
**代价**:QObject 开销。可接受。

### ADR-004 为什么 namespace glm
**决策**:所有代码 `namespace glm {}`。
**理由**:避免命名冲突(Message/Role 等通用名),为库化做准备。

### ADR-005 错误处理:异步信号 + 同步 Result 双轨
**决策**:异步操作(LLM 请求)用 `LlmReply::errorOccurred` 信号;同步操作(解析/DB/配置)用 `Result<T, Error>`。
**理由**:异步信号契合 Qt 事件循环;同步 Result 强制调用方处理错误,避免异常和"吞错误"(防玩具 DoD)。
**代价**:两套机制,但场景清晰不混淆。

### ADR-006 基础设施层独立
**决策**:横切关注点(日志/错误/配置/工具)独立成 `infrastructure/` 层,所有层可依赖。
**理由**:避免日志/错误处理散落各处;统一管理,不污染业务层。

### ADR-007 国际化:tr() 从 P1 起
**决策**:所有用户可见文本用 `tr()` 包裹,不用 `QStringLiteral`。
**理由**:主流桌面应用标配;后期加 `.ts/.qm` 翻译文件即可多语言,零重构。
**代价**:几乎无。

### ADR-008 应用组装在 main,依赖注入
**决策**:`main.cpp`(或 Application 类)组装各层(HttpClient → GlmProvider → ChatController → MainWindow),依赖注入,组件不自己 new 依赖。
**理由**:组装集中可见、组件解耦、便于测试(mock 注入);避免 MainWindow 强耦合 Provider。
**代价**:main 稍复杂。

### ADR-009 生成状态机显式(Q_ENUM + 信号)
**决策**:`ChatController` 持有 `State` 枚举(Q_ENUM),状态变更发 `stateChanged` 信号。
**理由**:显式状态治"隐式散乱";UI 据信号切按钮态(发送 ↔ 停止),逻辑集中;中断(Aborted)明确,防重复点。
**代价**:多一个枚举管理。值得。

## P1-P5 演进(每期加什么)
- **P1**:`core/`(ILlmProvider, LlmTypes, LlmReply) + `network/`(HttpClient, SseParser) + `providers/GlmProvider` + `tests/tst_sseparser`。架构骨架 + SSE。
- **P2**:`app/ChatController` + Middlewares(TokenCounter, ContextTruncator) + 参数面板 + Model/View 重构。
- **P3**:`data/DatabaseManager`(SQLite) + SessionManager + Markdown + 主题/动画。
- **P4**:`ui/DebugView` + 历史请求库 + 重放/对比。
- **P5**:`providers/`(Ollama, OpenAI) + Agent/Tool 框架 + RAG + 插件(QPluginLoader)。
