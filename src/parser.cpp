#include "popl/syntax/grammar/parser.hpp"

#include <memory>
#include <optional>
#include <print>
#include <variant>
#include <vector>

#include "popl/lexer/token_types.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/ast/expr.hpp"

namespace popl {

std::vector<Stmt> Parser::Parse() {
    std::vector<Stmt> statements{};
    while (!IsAtEnd()) statements.emplace_back(Declaration());
    return statements;
}
Stmt Parser::Declaration() {
    try {
        if (Match({TokenType::VAR})) return VarDeclaration();
        return Statement();
    } catch (const ParseError& error) {
        Synchronize();
        return Stmt{NilStmt()};
    }
}

Stmt Parser::VarDeclaration() {
    Token name = Consume(TokenType::IDENTIFIER, "Expect variable name.");

    Expr initializer{NilExpr{}};
    if (Match({TokenType::EQUAL})) {
        Expr rhs = Expression();
        if (!std::holds_alternative<NilExpr>(rhs.node)) {
            std::visit(
                [&](auto&& node) {
                    using T = std::decay_t<decltype(node)>;
                    initializer.node.emplace<T>(std::move(node));
                },
                rhs.node);
        }
    }

    Consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return Stmt{VarStmt{name, std::make_unique<Expr>(std::move(initializer))}};
}

Stmt Parser::Statement() {
    if (Match({TokenType::PRINT})) return PrintStatement();
    if (!IsAtEnd() && PeekNext().GetType() == TokenType::EQUAL &&
        Match({TokenType::IDENTIFIER}))
        return AssignmentStatement();
    if (Match({TokenType::LEFT_BRACE}))
        return Stmt{BlockStmt{BlockStatement()}};
    return ExpressionStatement();
}
std::vector<Stmt> Parser::BlockStatement() {
    std::vector<Stmt> statements;
    while (!Check(TokenType::RIGHT_BRACE) && !IsAtEnd())
        statements.emplace_back(Declaration());
    Consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return statements;
}
Stmt Parser::PrintStatement() {
    auto value = std::make_unique<Expr>(Expression());
    Consume(TokenType::SEMICOLON, "Expect ; after value.");
    return Stmt{PrintStmt{std::move(value)}};
}

Stmt Parser::AssignmentStatement() {
    Token name = Previous();
    Consume(TokenType::EQUAL, "Expects = after an identifier name");
    auto value{std::make_unique<Expr>(Expression())};
    Consume(TokenType::SEMICOLON, "Expects ; after Assignments.");
    return Stmt{AssignStmt{std::move(name), std::move(value)}};
}

Stmt Parser::ExpressionStatement() {
    auto expr = std::make_unique<Expr>(Expression());
    Consume(TokenType::SEMICOLON, "Expect ; after value.");
    return Stmt{ExpressionStmt{std::move(expr)}};
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

    if (Match({TokenType::IDENTIFIER})) {
        return Expr{VariableExpr{Previous()}};
    }

    if (Match({TokenType::LEFT_PAREN})) {
        Expr expr{Expression()};
        Consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return Expr{GroupingExpr{MakeExprPtr(std::move(expr))}};
    }
    throw Error(Peek(), "Expect expression");
}
};  // namespace popl
