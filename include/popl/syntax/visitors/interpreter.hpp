#pragma once

#include <print>

#include "popl/diagnostics.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/Exceptions/run_time_error.hpp"
#include "popl/syntax/ast/expr.hpp"
#include "popl/syntax/ast/stmt.hpp"

namespace popl {

struct Interpreter {
    void Interpret(const std::vector<Stmt>& statements) const {
        try {
            for (const auto& statement : statements) {
                Execute(statement);
            }
        } catch (const RunTimeError& error) {
            Diagnostics::ReportRunTimeError(error);
        }
    }
    /*
     * Statement visitor
     */
    void operator()(const ExpressionStmt& stmt) const {
        Evaluate(*(stmt.expression));
    }
    void operator()(const PrintStmt& stmt) const {
        PopLObject value = Evaluate(*(stmt.expression));
        println("{}", value.toString());
    }
    /*
     * Expresssoin visitor
     */
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
    void Execute(const Stmt& stmt) const { visitStmt(stmt, *this); }

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
