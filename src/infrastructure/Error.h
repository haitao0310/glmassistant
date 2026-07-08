#ifndef GLM_ERROR_H
#define GLM_ERROR_H

#include <QString>
#include <utility>

namespace glm {

// 统一错误类型(同步操作经 Result<T> 返回)
struct Error {
    enum class Code {
        Network,    // 网络失败 / 超时
        Auth,       // 401 / API key 无效
        Parse,      // JSON / SSE 解析失败
        Timeout,
        Config,     // 配置缺失(如 key 未设)
        Unknown,
    };
    Code code = Code::Unknown;
    QString message;

    static Error network(const QString &m) { return {Code::Network, m}; }
    static Error auth(const QString &m)    { return {Code::Auth, m}; }
    static Error parse(const QString &m)   { return {Code::Parse, m}; }
    static Error config(const QString &m)  { return {Code::Config, m}; }
};

// Result<T> = 成功值 | 错误(ADR-005)。
// 强制调用方检查 isOk(),避免"吞错误"。
// 用法:
//   Result<int> r = Result<int>::success(42);
//   if (r.isOk()) use(r.value()); else handle(r.error());
template<typename T>
class Result
{
public:
    static Result success(T v) { Result r; r.ok_ = true; r.value_ = std::move(v); return r; }
    static Result failure(Error e) { Result r; r.ok_ = false; r.error_ = std::move(e); return r; }

    bool isOk() const { return ok_; }
    bool isErr() const { return !ok_; }
    const T &value() const { return value_; }
    const Error &error() const { return error_; }

private:
    bool ok_ = false;
    T value_{};
    Error error_;
};

} // namespace glm

#endif // GLM_ERROR_H
