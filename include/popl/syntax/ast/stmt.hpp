#pragma once

#include <memory>
#include <variant>

#include "popl/lexer/token.hpp"
#include "popl/syntax/ast/expr.hpp"

namespace popl {

struct Stmt;

struct NilStmt {};

struct BlockStmt {
    std::vector<std::unique_ptr<Stmt>> statements;
};

struct ExpressionStmt {
    std::unique_ptr<Expr> expression;
};

struct VarStmt {
    Token                 name;
    std::unique_ptr<Expr> initializer;
};

struct IfStmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
};

struct WhileStmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
};

struct BreakStmt {
    Token keyword;
};

struct ContinueStmt {
    Token keyword;
};

struct FunctionStmt {
    Token                         name;
    std::unique_ptr<FunctionExpr> func;
};

struct ReturnStmt {
    Token                 keyword;
    std::unique_ptr<Expr> value;
};

struct ClassStmt {
    Token                                      name;
    std::vector<std::unique_ptr<FunctionStmt>> methods;
};

struct Stmt {
    using Variant = std::variant<NilStmt, BlockStmt, ExpressionStmt, VarStmt,
                                 IfStmt, WhileStmt, BreakStmt, ContinueStmt,
                                 FunctionStmt, ReturnStmt, ClassStmt>;

    Variant node;
};

template <typename Visitor, typename... Extra>
decltype(auto) visitStmtWithArgs(Stmt& stmt, Visitor&& visitor,
                                 Extra&&... extra) {
    return std::visit(
        [&](auto&& contained) -> decltype(auto) {
            return std::forward<Visitor>(visitor)(
                std::forward<decltype(contained)>(contained),
                std::forward<Extra>(extra)...);
        },
        stmt.node);
}

template <typename Visitor, typename... Extra>
decltype(auto) visitStmtWithArgs(const Stmt& stmt, Visitor&& visitor,
                                 Extra&&... extra) {
    return std::visit(
        [&](auto&& contained) -> decltype(auto) {
            return std::forward<Visitor>(visitor)(
                std::forward<decltype(contained)>(contained),
                std::forward<Extra>(extra)...);
        },
        stmt.node);
}
}  // namespace popl
