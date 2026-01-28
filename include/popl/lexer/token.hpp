#pragma once

#include <string>
#include <string_view>

#include "popl/literal.hpp"
#include "token_types.hpp"

namespace popl {
class Token {
   public:
    Token(TokenType type, std::string lexeme, PopLObject literal,
          unsigned int line)
        : type(type),
          lexeme(std::move(lexeme)),
          literal(std::move(literal)),
          line(line) {}
    friend std::ostream& operator<<(std::ostream& os, const Token& token);
    friend struct std::formatter<Token>;
    std::string GetLexeme() const { return lexeme; }
    PopLObject  GetLiteral() const { return literal; }

   private:
    const TokenType    type;
    const std::string  lexeme;
    const PopLObject   literal;
    const unsigned int line;
};
};  // namespace popl
template <>
struct std::formatter<popl::Token> : std::formatter<std::string_view> {
    auto format(const popl::Token& token, format_context& ctx) const {
        return std::formatter<std::string_view>::format(
            std::format("Token(type={}, lexeme=\"{}\", literal={}, line={})",
                        token.type, token.lexeme, token.literal, token.line),
            ctx);
    }
};
