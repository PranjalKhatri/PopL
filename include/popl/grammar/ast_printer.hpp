#pragma once

#include "popl/grammar/expr.hpp"

namespace popl {
struct AstPrinter {
    std::string Print(const Expr& expr) { return visitExpr(expr, *this); }

    std::string operator()(const LiteralExpr& e) { return e.value.toString(); }

    std::string operator()(const GroupingExpr& e) {
        return "(group " + popl::visitExpr(*e.expression, *this) + ")";
    }

    std::string operator()(const UnaryExpr& e) {
        return std::format("({} {})", e.op.GetLexeme(),
                           popl::visitExpr(*e.right, *this));
    }

    std::string operator()(const BinaryExpr& e) {
        return std::format("({} {} {})", popl::visitExpr(*e.left, *this),
                           e.op.GetLexeme(), popl::visitExpr(*e.right, *this));
    }
};
};  // namespace popl
