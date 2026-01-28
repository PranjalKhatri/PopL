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

struct ExprVisitor {
    virtual ~ExprVisitor()                      = default;
    virtual void operator()(BinaryExpr& expr)   = 0;
    virtual void operator()(GroupingExpr& expr) = 0;
    virtual void operator()(LiteralExpr& expr)  = 0;
    virtual void operator()(UnaryExpr& expr)    = 0;
};

inline void visitExpr(Expr& expr, ExprVisitor& visitor) {
    std::visit(visitor, expr.node);
}

}  // namespace popl
