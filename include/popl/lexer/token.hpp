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
        : m_type(type),
          m_lexeme(std::move(lexeme)),
          m_literal(std::move(literal)),
          m_line(line) {}
    TokenType    GetType() const { return m_type; }
    std::string  GetLexeme() const { return m_lexeme; }
    PopLObject   GetLiteral() const { return m_literal; }
    unsigned int GetLine() const { return m_line; }

    friend struct std::formatter<Token>;

   private:
    TokenType    m_type;
    std::string  m_lexeme;
    PopLObject   m_literal;
    unsigned int m_line;
};
};  // namespace popl
template <>
struct std::formatter<popl::Token> : std::formatter<std::string_view> {
    auto format(const popl::Token& token, format_context& ctx) const {
        return std::formatter<std::string_view>::format(
            std::format("Token(type={}, lexeme=\"{}\", literal={}, line={})",
                        token.m_type, token.m_lexeme, token.m_literal,
                        token.m_line),
            ctx);
    }
};
