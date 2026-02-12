#include "popl/syntax/grammar/parser.hpp"

#include <memory>
#include <vector>

#include "popl/lexer/token_types.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/ast/expr.hpp"
#include "popl/syntax/ast/stmt.hpp"

namespace popl {

std::vector<Stmt> Parser::Parse() {
    std::vector<Stmt> statements{};
    while (!IsAtEnd()) statements.emplace_back(Declaration());
    return statements;
}
Stmt Parser::Declaration() {
    try {
        if (Match({TokenType::VAR})) return VarDeclaration();
        if (Match({TokenType::FUN})) return FunctionDeclaration("function");
        return Statement();
    } catch (const ParseError& error) {
        Synchronize();
        return Stmt{NilStmt()};
    }
}

Stmt Parser::FunctionDeclaration(std::string_view kind) {
    Token name =
        Consume(TokenType::IDENTIFIER, std::format("Expect {} name.", kind));
    Consume(TokenType::LEFT_PAREN,
            std::format("Expect '(' after {} name.", kind));
    std::vector<Token> parameters;
    if (!Check(TokenType::RIGHT_PAREN)) {
        do {
            parameters.emplace_back(
                Consume(TokenType::IDENTIFIER, "Expect parameter name."));
        } while (Match({TokenType::COMMA}));
    }
    Consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
    Consume(TokenType::LEFT_BRACE,
            std::format("Expect '{{' before {} body.", kind));

    DepthGuard guard{m_function_depth};

    auto body{BlockStatement()};
    return Stmt{
        FunctionStmt{std::move(name), std::move(parameters), std::move(body)}};
}

Stmt Parser::VarDeclaration() {
    Token name = Consume(TokenType::IDENTIFIER, "Expect variable name.");

    std::unique_ptr<Expr> initializer =
        Match({TokenType::EQUAL}) ? MakeExprPtr(Expression()) : nullptr;

    Consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return Stmt{VarStmt{std::move(name), std::move(initializer)}};
}

Stmt Parser::Statement() {
    if (Match({TokenType::WHILE})) return WhileStatement();
    if (Match({TokenType::FOR})) return ForStatement();
    if (Match({TokenType::IF})) return IfStatement();
    if (Match({TokenType::PRINT})) return PrintStatement();
    if (Match({TokenType::BREAK})) return BreakStatement();
    if (Match({TokenType::CONTINUE})) return ContinueStatement();
    if (Match({TokenType::RETURN})) return ReturnStatement();
    if (!IsAtEnd() && PeekNext().GetType() == TokenType::EQUAL &&
        Match({TokenType::IDENTIFIER}))
        return AssignmentStatement();
    if (Match({TokenType::LEFT_BRACE}))
        return Stmt{BlockStmt{BlockStatement()}};
    return ExpressionStatement();
}
std::vector<std::unique_ptr<Stmt>> Parser::BlockStatement() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!Check(TokenType::RIGHT_BRACE) && !IsAtEnd())
        statements.emplace_back(MakeStmtPtr(Declaration()));
    Consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return statements;
}
Stmt Parser::BreakStatement() {
    if (m_loop_depth == 0) {
        throw Error(Previous(), "Cannot use 'break' outside of a loop.");
    }
    Consume(TokenType::SEMICOLON, "Expect ; after 'break'.");
    return Stmt{BreakStmt{Previous()}};
}
Stmt Parser::ContinueStatement() {
    if (m_loop_depth == 0)
        throw Error(Previous(), "Cannot use 'continue' outside of a loop.");

    Consume(TokenType::SEMICOLON, "Expect ; after 'continue'.");
    return Stmt{ContinueStmt{Previous()}};
}
Stmt Parser::ReturnStatement() {
    if (m_function_depth == 0)
        throw Error(Previous(), "Cannot return from top-level code.");

    Token keyword = Previous();
    Expr  value = Check(TokenType::SEMICOLON) ? Expr{NilExpr{}} : Expression();
    Consume(TokenType::SEMICOLON, "Expect ';' after return vaule.");
    return Stmt{ReturnStmt{std::move(keyword), MakeExprPtr(std::move(value))}};
}
Stmt Parser::WhileStatement() {
    Consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    Expr condition = Expression();
    Consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

    DepthGuard guard(m_loop_depth);

    Stmt body = Statement();
    return Stmt{WhileStmt{MakeExprPtr(std::move(condition)),
                          MakeStmtPtr(std::move(body))}};
}
Stmt Parser::ForStatement() {
    Consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    std::unique_ptr<Stmt> initializer;
    if (!Match({TokenType::SEMICOLON})) {
        if (Match({TokenType::VAR}))
            initializer = MakeStmtPtr(VarDeclaration());
        else
            initializer = MakeStmtPtr(ExpressionStatement());
    }

    std::unique_ptr<Expr> condition;
    if (!Check(TokenType::SEMICOLON)) {
        condition = MakeExprPtr(Expression());
    } else {
        condition = nullptr;
    }
    Consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

    std::unique_ptr<Expr> increment;
    if (!Check(TokenType::RIGHT_PAREN)) increment = MakeExprPtr(Expression());

    Consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

    DepthGuard guard(m_loop_depth);

    Stmt body = Statement();

    // attach increment
    if (increment) {
        std::vector<std::unique_ptr<Stmt>> stmts;
        stmts.push_back(MakeStmtPtr(std::move(body)));
        stmts.push_back(
            MakeStmtPtr(Stmt{ExpressionStmt{std::move(increment)}}));
        body.node = BlockStmt{std::move(stmts)};
    }

    if (!condition) {
        condition = MakeExprPtr(Expr{LiteralExpr{PopLObject{true}}});
    }

    body.node = WhileStmt{std::move(condition), MakeStmtPtr(std::move(body))};

    // attach initializer
    if (initializer) {
        std::vector<std::unique_ptr<Stmt>> stmts;
        stmts.push_back(std::move(initializer));
        stmts.push_back(MakeStmtPtr(std::move(body)));
        body.node = BlockStmt{std::move(stmts)};
    }

    return body;
}
Stmt Parser::IfStatement() {
    Consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    Expr condition = Expression();
    Consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

    Stmt thenBranch = Statement();

    Stmt elseBranch = Match({TokenType::ELSE}) ? Statement() : Stmt{NilStmt{}};

    return Stmt{IfStmt{MakeExprPtr(std::move(condition)),
                       MakeStmtPtr(std::move(thenBranch)),
                       MakeStmtPtr(std::move(elseBranch))}};
}
Stmt Parser::PrintStatement() {
    auto value = MakeExprPtr(Expression());
    Consume(TokenType::SEMICOLON, "Expect ; after value.");
    return Stmt{PrintStmt{std::move(value)}};
}

Stmt Parser::AssignmentStatement() {
    Token name = Previous();
    Consume(TokenType::EQUAL, "Expects = after an identifier name");
    auto value{MakeExprPtr(Expression())};
    Consume(TokenType::SEMICOLON, "Expects ; after Assignments.");
    return Stmt{AssignStmt{std::move(name), std::move(value)}};
}

Stmt Parser::ExpressionStatement() {
    auto expr = MakeExprPtr(Expression());
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
Expr Parser::ArgumentExpression() { return Ternary(); }

Expr Parser::Ternary() {
    Expr expr = OrExpression();
    if (Match({TokenType::QUESTION})) {
        Token question = Previous();
        Expr  middle   = Expression();
        Token colon =
            Consume(TokenType::COLON, "No matching ':' for '?' found");
        Expr right = Ternary();
        expr.node.emplace<TernaryExpr>(
            MakeExprPtr(std::move(expr)), std::move(question),
            MakeExprPtr(std::move(middle)), std::move(colon),
            MakeExprPtr(std::move(right)));
    }
    return expr;
}
Expr Parser::OrExpression() {
    return ParseBinary<LogicalExpr>(&Parser::AndExpression, {TokenType::OR});
}
Expr Parser::AndExpression() {
    return ParseBinary<LogicalExpr>(&Parser::Equality, {TokenType::AND});
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
    return CallExpression();
}
Expr Parser::CallExpression() {
    Expr expr{Primary()};
    while (true) {
        if (Match({TokenType::LEFT_PAREN})) {
            expr.node.emplace<CallExpr>(FinishCall(std::move(expr)));
        } else {
            break;
        }
    }
    return expr;
}
CallExpr Parser::FinishCall(Expr callee) {
    std::vector<std::unique_ptr<Expr>> arguments;
    if (!Check(TokenType::RIGHT_PAREN)) {
        do {
            arguments.emplace_back(MakeExprPtr(ArgumentExpression()));
        } while (Match({TokenType::COMMA}));
    }
    Token paren =
        Consume(TokenType::RIGHT_PAREN, "Expects ')' after arguments.");
    return CallExpr{MakeExprPtr(std::move(callee)), paren,
                    std::move(arguments)};
}
Expr Parser::Primary() {
    if (Match({TokenType::FALSE})) return Expr{LiteralExpr{PopLObject{false}}};
    if (Match({TokenType::TRUE})) return Expr{LiteralExpr{PopLObject{true}}};
    if (Match({TokenType::NIL}))
        return Expr{LiteralExpr{PopLObject{NilValue{}}}};

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
