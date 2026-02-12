#pragma once

#include <format>
#include <memory>
#include <string>
#include <variant>

#include "popl/callable.hpp"

namespace popl {
struct UninitializedValue {};
struct NilValue {};

inline bool operator==(const UninitializedValue&, const UninitializedValue&) {
    return false;
}
inline bool operator==(const NilValue&, const NilValue&) { return false; }

class PopLObject {
   public:
    using CallablePtr = std::shared_ptr<callable::PoplCallable>;
    using Value       = std::variant<UninitializedValue, NilValue, double,
                                     std::string, bool, CallablePtr>;

    PopLObject() = default;  // Uninitialized
    explicit PopLObject(NilValue) : m_data{} {}
    explicit PopLObject(double d) : m_data(d) {}
    explicit PopLObject(bool b) : m_data(b) {}
    explicit PopLObject(const std::string& str) : m_data(str) {}
    explicit PopLObject(std::string&& str) : m_data(std::move(str)) {}

    // type checks
    bool isNil() const { return std::holds_alternative<NilValue>(m_data); }
    bool isUninitialized() const {
        return std::holds_alternative<UninitializedValue>(m_data);
    }
    bool isNumber() const { return std::holds_alternative<double>(m_data); }
    bool isString() const {
        return std::holds_alternative<std::string>(m_data);
    }
    bool isBool() const { return std::holds_alternative<bool>(m_data); }
    bool isCallable() const {
        return std::holds_alternative<std::shared_ptr<callable::PoplCallable>>(
            m_data);
    }

    // accessors throws on misuse
    double             asNumber() const { return std::get<double>(m_data); }
    const std::string& asString() const {
        return std::get<std::string>(m_data);
    }
    CallablePtr asCallable() const { return std::get<CallablePtr>(m_data); }
    bool        asBool() const { return std::get<bool>(m_data); }

    bool isTruthy() const {
        if (isNil() || isUninitialized()) return false;
        if (isBool()) return asBool();
        return true;
    }

    std::string toString() const {
        return std::visit(
            [](auto&& v) -> std::string {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, UninitializedValue>) {
                    return "<Uninitialized>";
                } else if constexpr (std::is_same_v<T, NilValue>)
                    return "nil";
                else if constexpr (std::is_same_v<T, bool>)
                    return v ? "true" : "false";
                else if constexpr (std::is_same_v<T, CallablePtr>)
                    return "PopLCallable()";
                else if constexpr (std::is_same_v<T, double>) {
                    std::string s = std::to_string(v);
                    s.erase(s.find_last_not_of('0') + 1);
                    if (s.back() == '.') s.pop_back();
                    return s;
                } else
                    return v;
            },
            m_data);
    }

    friend bool operator==(const PopLObject& a, const PopLObject& b) {
        if (a.m_data.index() != b.m_data.index()) return false;

        return std::visit(
            [](auto&& x, auto&& y) -> bool {
                using X = std::decay_t<decltype(x)>;
                using Y = std::decay_t<decltype(y)>;

                if constexpr (std::is_same_v<X, Y>)
                    return x == y;
                else
                    return false;
            },
            a.m_data, b.m_data);
    }

    friend bool operator!=(const PopLObject& a, const PopLObject& b) {
        return !(a == b);
    }

   private:
    Value m_data{};
};

}  // namespace popl
template <>
struct std::formatter<popl::PopLObject> : std::formatter<std::string_view> {
    template <typename FormatContext>
    auto format(const popl::PopLObject& obj, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "{}", obj.toString());
    }
};
