#include "popl/syntax/grammar/parser.hpp"

#include <memory>

#include "popl/lexer/token_types.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/ast/expr.hpp"

namespace popl {
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

Expr Parser::Expression() { return Equality(); }
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
}
};  // namespace popl
