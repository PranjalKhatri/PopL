#include "popl/lexer/lexer.hpp"

#include "popl/diagnostics.hpp"
#include "popl/lexer/token_types.hpp"

namespace popl {

bool Lexer::Match(char expected) {
    if (ReachedEnd()) return false;
    if (m_source.at(m_current) != expected) return false;
    m_current++;
    return true;
}

char Lexer::Peek() {
    if (ReachedEnd()) return '\0';
    return m_source.at(m_current);
}

void Lexer::ScanToken() {
    char c = Advance();
    switch (c) {
        case '(':
            AddToken(TokenType::LEFT_PAREN);
            break;
        case ')':
            AddToken(TokenType::RIGHT_PAREN);
            break;
        case '{':
            AddToken(TokenType::LEFT_BRACE);
            break;
        case '}':
            AddToken(TokenType::RIGHT_BRACE);
            break;
        case ',':
            AddToken(TokenType::COMMA);
            break;
        case '.':
            AddToken(TokenType::DOT);
            break;
        case '-':
            AddToken(TokenType::MINUS);
            break;
        case '+':
            AddToken(TokenType::PLUS);
            break;
        case ';':
            AddToken(TokenType::SEMICOLON);
            break;
        case '*':
            AddToken(TokenType::STAR);
            break;
        case '!':
            AddToken(Match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;
        case '=':
            AddToken(Match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            AddToken(Match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
        case '>':
            AddToken(Match('=') ? TokenType::GREATER_EQUAL
                                : TokenType::GREATER);
            break;
        case '/':
            if (Match('/')) {
                // A comment goes until the end of the line.
                while (Peek() != '\n' && !ReachedEnd()) Advance();
            } else {
                AddToken(TokenType::SLASH);
            }
            break;
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            m_line++;
            break;
        default:
            Diagnostics::Error(m_line, "Unexpected character");
            break;
    }
}

void Lexer::AddToken(TokenType type, Literal literal) {
    std::string text = m_source.substr(m_start, m_current);
    m_tokens.emplace_back(type, std::move(text), std::move(literal), m_line);
}

};  // namespace popl
