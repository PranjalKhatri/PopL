#pragma once

#include <string>
#include <vector>

#include "popl/lexer/token_types.hpp"
#include "token.hpp"

namespace popl {

class Lexer {
   public:
    Lexer(std::string source) : m_source(std::move(source)) {}

   private:
    bool ReachedEnd() const { return m_current > m_source.size(); }
    char Peek() const;
    bool Match(char expected);
    void ScanToken();
    void AddToken(TokenType type) { AddToken(type, Literal{}); }
    char Advance() { return m_source.at(m_current++); }
    void AddToken(TokenType type, Literal literal);

   private:
    std::string        m_source;
    std::vector<Token> m_tokens{};

    unsigned int m_start{};    // first character in lexeme being scanned
    unsigned int m_current{};  // current character in lexeme being scanned
    unsigned int m_line{1};    // source line currently scanned
};

};  // namespace popl
