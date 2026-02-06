#pragma once

#include <memory>
#include <variant>

#include "popl/syntax/ast/expr.hpp"

namespace popl {

struct Stmt;

struct ExpressionStmt {
    std::unique_ptr<Expr> expression;
};

struct PrintStmt {
    std::unique_ptr<Expr> expression;
};

struct Stmt {
    using Variant = std::variant<ExpressionStmt, PrintStmt>;

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
