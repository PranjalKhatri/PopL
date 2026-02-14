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

struct PrintStmt {
    std::unique_ptr<Expr> expression;
};

struct VarStmt {
    Token                 name;
    std::unique_ptr<Expr> initializer;
};

struct AssignStmt {
    Token                 name;
    std::unique_ptr<Expr> value;
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

struct Stmt {
    using Variant =
        std::variant<NilStmt, BlockStmt, ExpressionStmt, PrintStmt, VarStmt,
                     AssignStmt, IfStmt, WhileStmt, BreakStmt, ContinueStmt,
                     FunctionStmt, ReturnStmt>;

    Variant node;
};

template <typename Visitor>
decltype(auto) visitStmt(Stmt& stmt, Visitor&& visitor) {
    return std::visit(std::forward<Visitor>(visitor), stmt.node);
}

template <typename Visitor>
decltype(auto) visitStmt(const Stmt& stmt, Visitor&& visitor) {
    return std::visit(std::forward<Visitor>(visitor), stmt.node);
}

template <typename Visitor, typename... Extra>
decltype(auto) visitStmtWithArgs(Stmt& var, Visitor&& visitor,
                                 Extra&&... extra) {
    return std::visit(
        [&](auto&& contained) -> decltype(auto) {
            return std::forward<Visitor>(visitor)(
                std::forward<decltype(contained)>(contained),
                std::forward<Extra>(extra)...);
        },
        var.node);
}
template <typename Visitor, typename... Extra>
decltype(auto) visitStmtWithArgs(const Stmt& var, Visitor&& visitor,
                                 Extra&&... extra) {
    return std::visit(
        [&](auto&& contained) -> decltype(auto) {
            return std::forward<Visitor>(visitor)(
                std::forward<decltype(contained)>(contained),
                std::forward<Extra>(extra)...);
        },
        var.node);
}
}  // namespace popl
