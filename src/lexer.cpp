#include "popl/lexer/lexer.hpp"

#include <cctype>
#include <variant>

#include "popl/diagnostics.hpp"
#include "popl/lexer/token_types.hpp"
#include "popl/literal.hpp"

namespace popl {

bool Lexer::Match(char expected) {
    if (ReachedEnd()) return false;
    if (m_source.at(m_current) != expected) return false;
    m_current++;
    return true;
}

char Lexer::Peek() const {
    if (ReachedEnd()) return '\0';
    return m_source.at(m_current);
}
char Lexer::PeekNext() const {
    if (m_current + 1 >= m_source.length()) return '\0';
    return m_source.at(m_current + 1);
}
void Lexer::ScanStringLiteral() {
    while (Peek() != '\"' && !ReachedEnd()) {
        if (Peek() == '\n') m_line++;
        Advance();
    }
    if (ReachedEnd()) {
        Diagnostics::Error(m_line, "Unterminated String");
        return;
    }
    Advance();  // closing "
    std::string value =
        m_source.substr(m_start + 1, GetCurrentLiteralLength() - 2);
    AddToken(TokenType::STRING, PopLObject{std::move(value)});
}
void Lexer::ScanNumberLiteral() {
    while (std::isdigit(Peek())) Advance();
    // consume .
    if (Peek() == '.' && isdigit(PeekNext())) {
        Advance();
        while (std::isdigit(Peek())) Advance();
    }
    AddToken(TokenType::NUMBER,
             PopLObject{std::stod(GetCurrentLiteralString())});
}
void Lexer::ScanIdentifier() {
    while (IsAlphaNumOrUnderScore(Peek())) Advance();
    std::string_view text{GetCurrentLiteralView()};

    TokenType type = popl::KeywordOrIdentifier(text);
    AddToken(type);
}

void Lexer::SkipBlockComment() {
    bool   Done      = false;
    size_t lineStart = m_line;
    while (!Done && !ReachedEnd()) {
        if (Peek() == '*' && PeekNext() == '/') {
            Advance();
            Done = true;
        } else if (Peek() == '\n') {
            m_line++;
        }
        Advance();
    }
    if (!Done) {
        Diagnostics::Error(lineStart, "Block Comment End Not Found");
    }
}

std::vector<Token> Lexer::ScanTokens() {
    while (!ReachedEnd()) {
        m_start = m_current;
        ScanToken();
    }
    m_tokens.emplace_back(TokenType::END_OF_FILE, "",
                          PopLObject{std::monostate{}}, m_line);
    return m_tokens;
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
        case '?':
            AddToken(TokenType::QUESTION);
            break;
        case ':':
            AddToken(TokenType::COLON);
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
                // Line Comment
                while (Peek() != '\n' && !ReachedEnd()) Advance();
            } else if (Match('*')) {
                // Block Comment
                SkipBlockComment();
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
        case '\"':
            ScanStringLiteral();
            break;
        default:
            if (std::isdigit(c)) {
                ScanNumberLiteral();
            } else if (IsAlphaOrUnderScore(c)) {
                ScanIdentifier();
            } else {
                Diagnostics::Error(m_line, "Unexpected character");
            }
            break;
    }
}

void Lexer::AddToken(TokenType type, PopLObject literal) {
    std::string text{GetCurrentLiteralString()};
    m_tokens.emplace_back(type, std::move(text), std::move(literal), m_line);
}

};  // namespace popl
