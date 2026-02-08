#pragma once

#include <memory>
#include <variant>

#include "popl/lexer/token.hpp"
#include "popl/syntax/ast/expr.hpp"

namespace popl {

struct Stmt;

struct NilStmt {};

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

struct Stmt {
    using Variant =
        std::variant<NilStmt, ExpressionStmt, PrintStmt, VarStmt, AssignStmt>;

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
}  // namespace popl
