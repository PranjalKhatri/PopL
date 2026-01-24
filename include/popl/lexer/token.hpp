#pragma once

#include <string>
#include <string_view>
#include <variant>

#include "token_types.hpp"

namespace popl {
using Literal = std::variant<std::monostate, double, std::string, bool>;
class Token {
   public:
    Token(TokenType type, std::string lexeme, Literal literal,
          unsigned int line)
        : type(type),
          lexeme(std::move(lexeme)),
          literal(std::move(literal)),
          line(line) {}
    friend std::ostream& operator<<(std::ostream& os, const Token& token);
    friend struct std::formatter<Token>;

   private:
    const TokenType    type;
    const std::string  lexeme;
    const Literal      literal;
    const unsigned int line;
};
};  // namespace popl
template <>
struct std::formatter<popl::Literal> : std::formatter<std::string_view> {
    auto format(const popl::Literal& lit, format_context& ctx) const {
        return std::visit(
            [&](const auto& value) {
                using T = std::decay_t<decltype(value)>;

                if constexpr (std::is_same_v<T, std::monostate>) {
                    return std::formatter<std::string_view>::format("nil", ctx);
                } else if constexpr (std::is_same_v<T, bool>) {
                    return std::formatter<std::string_view>::format(
                        value ? "true" : "false", ctx);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    return std::formatter<std::string_view>::format(value, ctx);
                } else {
                    // double
                    return std::format_to(ctx.out(), "{}", value);
                }
            },
            lit);
    }
};
template <>
struct std::formatter<popl::Token> : std::formatter<std::string_view> {
    auto format(const popl::Token& token, format_context& ctx) const {
        return std::formatter<std::string_view>::format(
            std::format("Token(type={}, lexeme=\"{}\", literal={}, line={})",
                        token.type, token.lexeme, token.literal, token.line),
            ctx);
    }
};
