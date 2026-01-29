#pragma once

#include <memory>
#include <variant>

#include "popl/lexer/token.hpp"
#include "popl/literal.hpp"

namespace popl {

struct Expr;

struct BinaryExpr {
    std::unique_ptr<Expr> left;
    Token                 op;
    std::unique_ptr<Expr> right;
};

struct GroupingExpr {
    std::unique_ptr<Expr> expression;
};

struct LiteralExpr {
    PopLObject value;
};

struct UnaryExpr {
    Token                 op;
    std::unique_ptr<Expr> right;
};

struct Expr {
    using Variant =
        std::variant<BinaryExpr, GroupingExpr, LiteralExpr, UnaryExpr>;

    Variant node;
};

template <typename Visitor>
decltype(auto) visitExpr(Expr& expr, Visitor&& visitor) {
    return std::visit(std::forward<Visitor>(visitor), expr.node);
}

template <typename Visitor>
decltype(auto) visitExpr(const Expr& expr, Visitor&& visitor) {
    return std::visit(std::forward<Visitor>(visitor), expr.node);
}

}  // namespace popl
