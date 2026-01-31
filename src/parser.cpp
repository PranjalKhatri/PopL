#include "popl/syntax/grammar/parser.hpp"

#include <optional>

#include "popl/lexer/token_types.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/ast/expr.hpp"

namespace popl {

std::optional<Expr> Parser::Parse() {
    try {
        return Expression();
    } catch (const ParseError& parseError) {
        return std::nullopt;
    }
}

Token Parser::Advance() {
    if (!IsAtEnd()) m_current++;
    return Previous();
}
bool Parser::Match(std::initializer_list<TokenType> tokenTypes) {
    for (const auto& tokenType : tokenTypes) {
        if (Check(tokenType)) {
            Advance();
            return true;
        }
    }
    return false;
}

void Parser::Synchronize() {
    Advance();
    while (!IsAtEnd()) {
        if (Previous().GetType() == TokenType::SEMICOLON) return;

        switch (Peek().GetType()) {
            case TokenType::CLASS:
            case TokenType::FUN:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
                return;
            default:
                break;
        }
        Advance();
    }
}

Expr Parser::Expression() { return Comma(); }
Expr Parser::Comma() {
    return ParseBinary(&Parser::Ternary, {TokenType::COMMA});
}
Expr Parser::Ternary() {
    Expr expr = Equality();
    if (Match({TokenType::QUESTION})) {
        Token question = Previous();
        Expr  middle   = Expression();
        Token colon =
            Consume(TokenType::COLON, "No matching ':' for '?' found");
        Expr right = Ternary();
        expr.node.emplace<TernaryExpr>(MakeExprPtr(std::move(expr)), question,
                                       MakeExprPtr(std::move(middle)), colon,
                                       MakeExprPtr(std::move(right)));
    }
    return expr;
}
Expr Parser::Equality() {
    return ParseBinary(&Parser::Comparison,
                       {TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL});
}
Expr Parser::Comparison() {
    return ParseBinary(&Parser::Term,
                       {TokenType::GREATER, TokenType::GREATER_EQUAL,
                        TokenType::LESS, TokenType::LESS_EQUAL});
}
Expr Parser::Term() {
    return ParseBinary(&Parser::Factor, {TokenType::MINUS, TokenType::PLUS});
}
Expr Parser::Factor() {
    return ParseBinary(&Parser::Unary, {TokenType::SLASH, TokenType::STAR});
}
Expr Parser::Unary() {
    if (Match({TokenType::BANG, TokenType::MINUS})) {
        return Expr{UnaryExpr{Previous(), MakeExprPtr(Unary())}};
    }
    return Primary();
}
Expr Parser::Primary() {
    if (Match({TokenType::FALSE})) return Expr{LiteralExpr{PopLObject{false}}};
    if (Match({TokenType::TRUE})) return Expr{LiteralExpr{PopLObject{true}}};
    if (Match({TokenType::NIL})) return Expr{LiteralExpr{PopLObject{}}};

    if (Match({TokenType::NUMBER, TokenType::STRING})) {
        return Expr{LiteralExpr(Previous().GetLiteral())};
    }

    if (Match({TokenType::LEFT_PAREN})) {
        Expr expr{Expression()};
        Consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return Expr{GroupingExpr{MakeExprPtr(std::move(expr))}};
    }
    throw Error(Peek(), "Expect expression");
}
};  // namespace popl
