#pragma once

#include "popl/syntax/ast/expr.hpp"

namespace popl {
// TODO: Update it for newer statements and expressions
struct AstPrinter {
    std::string Print(const Expr& expr) { return visitExpr(expr, *this); }

    std::string operator()(const LiteralExpr& e) { return e.value.toString(); }

    std::string operator()(const GroupingExpr& e) {
        return "(group " + visitExpr(*e.expression, *this) + ")";
    }

    std::string operator()(const UnaryExpr& e) {
        return std::format("({} {})", e.op.GetLexeme(),
                           visitExpr(*e.right, *this));
    }

    std::string operator()(const BinaryExpr& e) {
        return std::format("({} {} {})", visitExpr(*e.left, *this),
                           e.op.GetLexeme(), visitExpr(*e.right, *this));
    }

    std::string operator()(const TernaryExpr& e) {
        return std::format("({} {} {} {} {})", visitExpr(*e.left, *this),
                           e.opa.GetLexeme(), visitExpr(*e.mid, *this),
                           e.opb.GetLexeme(), visitExpr(*e.right, *this));
    }
};
};  // namespace popl
