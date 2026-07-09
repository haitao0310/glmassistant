# GlmAssistant 面试讲解稿

> 求职用:9 个技术点 × (是什么/为什么/怎么做/坑/边界) + 追问答案 + demo 脚本。照此讲,能扛追问。

## 一句话 + 技术栈

面向 AI 开发者的 LLM 桌面工作站(对话 + 调试双模式)。Qt 6.8.3 LTS / C++17 / CMake / MinGW / Qt Network(SSE) / SQLite / Qt Widgets。

## 架构总览(6 层 + 4 扩展点)

```
UI 层(mainwindow/ChatWidget/DebugView)
业务层(ChatController/SessionManager/AgentController)
服务层(ILlmProvider + Glm/Ollama + ProviderRegistry + 中间件管道)
数据层(DatabaseManager SQLite + ChatModel)
网络层(HttpClient + SseParser)
基础设施(Logger/Error/Constants/Theme)
```
4 扩展点:Provider(多厂商)/中间件(请求处理链)/Agent 工具/RAG。9 个 ADR 记关键决策。

---

## 9 技术点

### 1. SSE 流式解析(`SseParser`)
- **是什么**:Server-Sent Events,服务端单向推送(GLM/OpenAI 流式回复用)。
- **为什么**:GLM 流式用 SSE(ADR-002),非 WebSocket——双向用不上且复杂。
- **怎么做**:`feed(chunk)` 按 `"\n\n"` 分帧;`m_buffer` 缓存跨 chunk 不完整帧;`extractDeltaContent` 提 `choices[0].delta.content`;`isDone` 识别 `[DONE]`;1MB buffer 上限防恶意流。
- **坑**:TCP 分包(帧跨 chunk,buffer 拼接);坏 JSON 不崩返空。
- **边界**:单帧/多帧/跨帧/坏 JSON/超限 全覆盖,9 单测。
- **追问**:
  - Q: 为什么 `\n\n` 分帧? → SSE 规范双换行。
  - Q: 1MB 上限怎么定? → 经验值,防无 `\n\n` 的垃圾流。

### 2. 状态机(`ChatController::State`)
- **是什么**:`State{Idle,Sending,Streaming,Finished,Error,Aborted}` 枚举,Q_ENUM。
- **为什么**:显式状态治"隐式散乱"(ADR-009),UI 据状态切发送/停止按钮。
- **怎么做**:`setState()` 发 `stateChanged`;`send()` 守卫 `!Idle` 拒;`stop()` 设 Aborted;`connectReply` 各信号据状态处理。
- **坑**:重复点击(守卫);abort 后 finished 信号可能覆盖(查 Aborted return)。
- **边界**:6 状态转换覆盖。
- **追问**:
  - Q: Aborted 为什么不复用 Error? → 用户主动 vs 系统错,UX 区分。
  - Q: 断网怎么 Streaming→Error? → `LlmReply::errorOccurred` → Controller。

### 3. Provider 抽象(`ILlmProvider` + Registry)
- **是什么**:`ILlmProvider` 接口,`Glm/Ollama` 实现,`ProviderRegistry` 工厂。
- **为什么**:多厂商扩展(ADR-001),业务层不绑死;P5 加 Ollama 零改业务层。
- **怎么做**:`ILlmProvider{send/models/id/displayName}`;各实现;Registry 按 id 取;依赖注入。
- **坑**:API 差异(OpenAI 兼容格式统一);Registry 不持所有权(main 管生命周期)。
- **边界**:GLM + Ollama 两厂商。
- **追问**:
  - Q: `send` 返回 `LlmReply*` 非 `future`? → Qt 信号槽适合流式增量,future 不支持。
  - Q: Registry 不持所有权会悬空? → main 管,Registry 只索引。

### 4. 中间件管道(`IMiddleware` + Pipeline + Retry + Cache)
- **是什么**:请求前责任链(中间件)+ 请求级 Retry(HttpClient)+ Cache(CachingProvider)。
- **为什么**:解耦(扩展点 2),加处理不动 Provider/业务层。
- **怎么做**:`Pipeline.process(req)` 按序调 `IMiddleware.process`;实现 `TokenCounter`(估 token)+`ContextTruncation`(超限截旧)+`LoggingMiddleware`(请求日志)。Retry 在 HttpClient(网络错 `httpCode==0` 重试 + 指数退避 1s/2s)。Cache 在 `CachingProvider`(Decorator 包装 ILlmProvider,body hash → 响应缓存,命中跳过请求)。
- **坑**:顺序依赖(TokenCounter 先于 Truncation);传值非引用(函数式无副作用);Retry 只网络错(HTTP 4xx 是逻辑错重试无用);Cache 命中要延迟发 finished(`QMetaObject::invokeMethod` QueuedConnection,保异步感)。
- **边界**:Logging/Token/Truncation 真中间件;Retry + Cache 真实现。
- **追问**:
  - Q: 为什么传值? → 独立处理无副作用。
  - Q: Retry 为什么 HTTP 4xx 不重试? → 401/400 是 key/请求错,重试无用;只重连失败/超时。
  - Q: Cache 命中怎么发响应? → `invokeMethod` QueuedConnection 延迟到事件循环,不阻塞调用方。
  - Q: Retry/Cache 为什么不在 Pipeline? → Pipeline 是请求前同步;Retry/Cache 要感知请求结果(异步),适合 Provider/Http 层。

### 5. Agent tool_calls 循环(`AgentController`)
- **是什么**:LLM 返回 `tool_calls` → 执行工具 → 回填 → 再请求,递归循环。
- **为什么**:让 LLM 调外部函数(计算器/时间/RAG),扩展点 3。
- **怎么做**:`runStep` 检 depth(>kMaxDepth 报错);`parseToolCalls` 提取;`ToolRegistry` 查工具 `invoke`;结果回填 Tool 消息;递归 `runStep(depth+1)`。
- **坑**:无限循环(kMaxDepth=8 防);工具不存在(回错误消息);非流式(简化解析)。
- **边界**:CalculatorTool(QJSEngine)/TimeTool/RetrievalTool。
- **追问**:
  - Q: 为什么非流式? → 简化 tool_calls 解析,Agent 关注最终结果。
  - Q: kMaxDepth 怎么定? → 经验值,实际极少超 3 层。

### 6. RAG(`VectorStore` + Embedding 同步/异步 + RetrievalTool)
- **是什么**:余弦向量检索 + embedding(同步 `embed` + 异步 `embedAsync`)+ 工具,知识库接入 Agent。
- **为什么**:私有知识检索。
- **怎么做**:`embed(text)` 同步(QEventLoop,RetrievalTool 工具同步用);`embedAsync(text)` 异步(信号 `embeddingReady/Failed`,去 QEventLoop 嵌套);`VectorStore.search(queryVec,k)` top-k 余弦;`RetrievalTool` 给 Agent 调。
- **坑**:同步 embed QEventLoop 嵌套(RetrievalTool 同步接口要求);异步 embedAsync 信号链(无嵌套,适合批量);无持久化(进程内)。
- **边界**:基础检索 + 同步/异步 embedding。持久化方案(SQLite 向量/Faiss)设计。
- **追问**:
  - Q: 余弦 vs 欧氏? → 文本 embedding 关注方向,余弦稳。
  - Q: 大知识库? → 接 Faiss/Milvus 或 SQLite 向量扩展。
  - Q: 同步 embed 为什么用 QEventLoop? → RetrievalTool::invoke 是同步接口(ITool),Agent 工具同步;embedAsync 是异步 API(信号),批量/非阻塞场景用。
  - Q: embedAsync 怎么避免 QEventLoop? → HTTP `dataReceived/finished` 信号累积响应,finished 时 parseEmbedding + emit embeddingReady,无嵌套事件循环。

### 7. SQLite 持久化(`DatabaseManager`)
- **是什么**:单例,会话/消息/请求 三表,schema 版本迁移 + 索引 + 事务。
- **为什么**:本地持久化(重启不丢),版本迁移防玩具 DoD(可演进);索引提速;事务保原子。
- **怎么做**:`ensureSchema` 检 `schema_version`,按版本建表(v1 sessions/messages,v2 requests)+ 建索引(`idx_messages_session` session_id+timestamp,`idx_requests_session`);CRUD;`appendMessage` 事务(transaction + INSERT + UPDATE + commit,失败 rollback)。
- **坑**:NOT NULL(assistant 预占 content 空 → 改 finished 插);事务(INSERT+UPDATE 原子,防半成功);索引(按 session_id 查消息快)。
- **边界**:v1/v2 迁移 + 索引 + 事务。
- **追问**:
  - Q: assistant 为什么 finished 插? → 预占 content 空 NOT NULL 失败。
  - Q: 事务保证什么? → INSERT 消息 + UPDATE 会话时间 原子,防不一致。
  - Q: 为什么 session_id+timestamp 索引? → 按会话查消息(常见)按时间排,复合索引加速。
  - Q: schema 迁移怎么演进? → `schema_version` 表记版本,`if(version<N)` 建表/改表,不丢老数据。

### 8. 分层架构 + ADR(`ARCHITECTURE.md`)
- **是什么**:6 层 + 9 ADR(决策记录)。
- **为什么**:解耦(依赖单向)+ 决策可追溯。
- **怎么做**:`src/` 按层分包;ADR-001~009 记决策(Provider/SSE/信号/namespace/错误/基础设施/i18n/组装/状态机)。
- **坑**:循环依赖(避免);Q_OBJECT moc 坑(.h 要在 SOURCES 让 AUTOMOC 扫)。
- **边界**:9 ADR 覆盖关键决策。
- **追问**:
  - Q: Logger 为什么基础设施层? → 横切关注点,所有层用。
  - Q: 保证依赖单向? → 分层规范 + 代码审查。

### 9. 异步/生命周期(`LlmReply` + shared_ptr + connect context)
- **是什么**:`LlmReply`(QObject 信号)+ `shared_ptr` + `QObject::connect` 第 3 参 context 管生命周期。
- **为什么**:Qt 信号槽异步 + 跨线程安全(ADR-003);流式增量适合信号非回调。
- **怎么做**:`LlmReply` 发 `chunkReceived/finished/error`;GlmProvider lambda 捕 `shared_ptr`(parser/accumulated/done);`connect(sig, reply, lambda)` context=reply,reply 销毁 connect 自动断。
- **坑**:局部引用悬空(send 返回后局部销毁 → shared_ptr);abort 后信号(context=reply 断)。
- **边界**:流式/非流式/错误/中断。
- **追问**:
  - Q: 为什么 shared_ptr 非成员? → lambda 捕获管生命周期。
  - Q: reply 提前删 connect lambda 崩? → connect context=reply,销毁自动断。

---

## Demo 脚本(5-10 分钟)

1. **启动** → 多会话侧栏(新建/切换/删除)
2. **对话**:输入"你好" → SSE 流式打字机(逐字出现)
3. **调参**:temperature 滑块 → 影响下次生成
4. **中断**:生成中点"停止" → Aborted 态
5. **调试 tab**:看原始请求/响应 JSON → 重放改参
6. (选讲)**Agent/RAG**:工具调用循环

## 高频预备问题

- **为什么 Qt6 不 Qt5?** LTS + Schannel 原生 TLS(免 OpenSSL DLL)
- **为什么 CMake 不 qmake?** Qt6 主推 + 现代工程标准
- **LGPL 商用?** 动态链接可商用(库可替换,满足 LGPL)
- **怎么调试 SSE?** `logError` + `rawResponse` 累积(P4 调试模式看原始)
- **错误怎么处理?** 友好提示(401→检查 key / 超时→重试 / 断网→检查网络),非 raw 日志
- **多轮上下文怎么传?** 完整 messages 数组(累积历史),超 token 中间件截断
- **Qt 信号槽 vs 回调?** 信号槽异步 + 跨线程安全 + UI 绑定方便;回调易生命周期失控

## 讲解节奏建议

- **项目总览**(2 分钟):定位 + 架构 6 层 + 4 扩展点
- **深入 2-3 点**(各 3-5 分钟):挑 SSE / 状态机 / Provider(最扛追问)
- **demo**(5 分钟):跑给面试官看
- **Q&A**:用追问答案应对
