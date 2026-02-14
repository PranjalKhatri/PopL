#pragma once

#include <memory>
#include <variant>

#include "popl/lexer/token.hpp"
#include "popl/literal.hpp"

namespace popl {

struct Expr;
struct Stmt;

struct NilExpr {};

struct BinaryExpr {
    std::unique_ptr<Expr> left;
    Token                 op;
    std::unique_ptr<Expr> right;
};

struct TernaryExpr {
    std::unique_ptr<Expr> condition;
    Token                 question;
    std::unique_ptr<Expr> thenBranch;
    Token                 colon;
    std::unique_ptr<Expr> elseBranch;
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

struct CallExpr {
    std::unique_ptr<Expr>              callee;
    Token                              ClosingParen;
    std::vector<std::unique_ptr<Expr>> arguments;
};

struct VariableExpr {
    Token              name;
    std::optional<int> depth;
};

struct LogicalExpr {
    std::unique_ptr<Expr> left;
    Token                 op;
    std::unique_ptr<Expr> right;
};

struct FunctionExpr {
    std::vector<Token>                 params;
    std::vector<std::unique_ptr<Stmt>> body;
};

struct Expr {
    using Variant = std::variant<NilExpr, BinaryExpr, TernaryExpr, GroupingExpr,
                                 LiteralExpr, UnaryExpr, CallExpr, VariableExpr,
                                 LogicalExpr, FunctionExpr>;

    Variant node;
};

template <typename Visitor, typename... Extra>
decltype(auto) visitExprWithArgs(Expr& expr, Visitor&& visitor,
                                 Extra&&... extra) {
    return std::visit(
        [&](auto&& contained) -> decltype(auto) {
            return std::forward<Visitor>(visitor)(
                std::forward<decltype(contained)>(contained),
                std::forward<Extra>(extra)...);
        },
        expr.node);
}

template <typename Visitor, typename... Extra>
decltype(auto) visitExprWithArgs(const Expr& expr, Visitor&& visitor,
                                 Extra&&... extra) {
    return std::visit(
        [&](auto&& contained) -> decltype(auto) {
            return std::forward<Visitor>(visitor)(
                std::forward<decltype(contained)>(contained),
                std::forward<Extra>(extra)...);
        },
        expr.node);
}
}  // namespace popl
