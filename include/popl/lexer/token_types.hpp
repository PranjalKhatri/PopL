#pragma once

#include <array>
#include <cstddef>
#include <ostream>

#include "magic_enum/magic_enum.hpp"

namespace popl {
enum class TokenType {
    // Single-character tokens.
    LEFT_PAREN = 0,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,
    QUESTION,
    COLON,

    // One or two character tokens.
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,

    // Literals.
    IDENTIFIER,
    STRING,
    NUMBER,

    // Keywords.
    AND,
    CLASS,
    ELSE,
    FALSE,
    FUN,
    FOR,
    IF,
    NIL,
    OR,
    RETURN,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,
    BREAK,
    CONTINUE,

    END_OF_FILE,

    TOKEN_COUNT

};
struct Keyword {
    std::string_view text;
    TokenType        type;
};

constexpr std::array<Keyword, 17> Keywords = {{

    {"and", TokenType::AND},
    {"class", TokenType::CLASS},
    {"else", TokenType::ELSE},
    {"false", TokenType::FALSE},
    {"for", TokenType::FOR},
    {"fun", TokenType::FUN},
    {"if", TokenType::IF},
    {"nil", TokenType::NIL},
    {"or", TokenType::OR},
    {"return", TokenType::RETURN},
    {"super", TokenType::SUPER},
    {"this", TokenType::THIS},
    {"true", TokenType::TRUE},
    {"var", TokenType::VAR},
    {"while", TokenType::WHILE},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
}};

constexpr TokenType KeywordOrIdentifier(std::string_view s) {
    for (const auto& [kw, type] : Keywords) {
        if (kw == s) return type;
    }
    return TokenType::IDENTIFIER;
}
inline std::ostream& operator<<(std::ostream&   os,
                                popl::TokenType type) noexcept {
    os << magic_enum::enum_name(type);
    return os;
}
};  // namespace popl

template <>
struct std::formatter<popl::TokenType> : std::formatter<std::string_view> {
    auto format(popl::TokenType type, format_context& ctx) const {
        return std::formatter<std::string_view>::format(
            magic_enum::enum_name(type), ctx);
    }
};
