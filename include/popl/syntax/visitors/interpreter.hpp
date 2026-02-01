#pragma once

#include <print>

#include "popl/diagnostics.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/Exceptions/run_time_error.hpp"
#include "popl/syntax/ast/expr.hpp"

namespace popl {

struct Interpreter {
    void Interpret(const Expr expression) {
        try {
            PopLObject value = Evaluate(expression);
            std::println("{}", value.toString());
        } catch (const RunTimeError& error) {
            Diagnostics::ReportRunTimeError(error);
        }
    }

    PopLObject operator()(const LiteralExpr& expr) const { return expr.value; }

    PopLObject operator()(const GroupingExpr& expr) const {
        return Evaluate(*expr.expression);
    }

    PopLObject operator()(const TernaryExpr& expr) const {
        PopLObject left = Evaluate(*expr.right);
        if (left.isTruthy()) return Evaluate(*expr.mid);
        return Evaluate(*expr.right);
    }
    PopLObject operator()(const UnaryExpr& expr) const;
    PopLObject operator()(const BinaryExpr& expr) const;

   private:
    PopLObject Evaluate(const Expr& expr) const {
        return visitExpr(expr, *this);
    }
    void CheckNumberOperand(const Token& op, const PopLObject& operand) const {
        if (operand.isNumber()) return;
        throw RunTimeError(op, "Operand must be a number.");
    }
    void CheckNumberOperand(const Token& op, const PopLObject& left,
                            const PopLObject& right) const {
        if (left.isNumber() && right.isNumber()) return;
        throw RunTimeError(op, "Operands must be number");
    }
};
};  // namespace popl
