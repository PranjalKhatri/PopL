#pragma once

#include <cctype>
#include <string>
#include <vector>

#include "popl/lexer/token_types.hpp"
#include "token.hpp"

namespace popl {

class Lexer {
   public:
    Lexer(std::string source) : m_source(std::move(source)) {}

   private:
    bool   ReachedEnd() const { return m_current > m_source.size(); }
    char   Peek() const;
    char   PeekNext() const;
    size_t GetCurrentLiteralLength() const { return m_current - m_start + 1; }
    bool   IsAlphaOrUnderScore(char c) const {
        return std::isalpha(c) || (c == '_');
    }
    bool IsAlphaNumOrUnderScore(char c) const {
        return IsAlphaOrUnderScore(c) || std::isdigit(c);
    }

    bool Match(char expected);
    void ScanToken();
    void ScanStringLiteral();
    void ScanNumberLiteral();
    void ScanIdentifier();
    void AddToken(TokenType type) { AddToken(type, Literal{}); }
    void AddToken(TokenType type, Literal literal);
    char Advance() { return m_source.at(m_current++); }

   private:
    std::string        m_source;
    std::vector<Token> m_tokens{};

    size_t m_start{};    // first character in lexeme being scanned
    size_t m_current{};  // current character in lexeme being scanned
    size_t m_line{1};    // source line currently scanned
};

};  // namespace popl
