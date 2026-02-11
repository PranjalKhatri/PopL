#pragma once

#include <memory>
#include <print>
#include <variant>

#include "popl/diagnostics.hpp"
#include "popl/environment.hpp"
#include "popl/literal.hpp"
#include "popl/runtime/run_time_error.hpp"
#include "popl/syntax/ast/expr.hpp"
#include "popl/syntax/ast/stmt.hpp"

namespace popl {

// TODO: Reason about environment pointer
class Interpreter {
   public:
    void Interpret(const std::vector<Stmt>& statements, bool replMode) {
        m_repl_mode = replMode;
        try {
            for (const auto& statement : statements) {
                Execute(statement);
            }
        } catch (const runtime::RunTimeError& error) {
            Diagnostics::ReportRunTimeError(error);
        }
    }
    /*
     * Statement visitor
     */
    void operator()(const ExpressionStmt& stmt) {
        PopLObject obj = Evaluate(*(stmt.expression));
        if (m_repl_mode) {
            CheckUninitialised(MakeReplReadToken(), obj);
            std::println("{}", obj.toString());
        }
    }
    void operator()(const PrintStmt& stmt) {
        PopLObject value = Evaluate(*(stmt.expression));
        CheckUninitialised(MakeReplReadToken("Print"), value);
        println("{}", value.toString());
    }
    void operator()(const NilStmt& stmt) {
        // Yeah.. do nothing
    }
    void operator()(const VarStmt& stmt) {
        PopLObject value{};
        if (!std::holds_alternative<NilExpr>(stmt.initializer->node))
            value = Evaluate(*(stmt.initializer));
        m_environment->Define(stmt.name, value);
    }
    void operator()(const BlockStmt& stmt) {
        Environment blockEnv(m_environment);
        ExecuteBlock(stmt.statements, &blockEnv);
    }
    void operator()(const AssignStmt& stmt) {
        PopLObject value{Evaluate(*(stmt.value))};
        m_environment->Assign(stmt.name, value);
    }
    void operator()(const IfStmt& stmt) {
        if (Evaluate(*stmt.condition).isTruthy())
            Execute(*stmt.thenBranch);
        else
            Execute(*stmt.elseBranch);
    }
    void operator()(const WhileStmt& stmt);
    void operator()(const BreakStmt& stmt) {
        if (!m_in_loop)
            throw runtime::RunTimeError(stmt.keyword,
                                        "'break' outside of any loop.");
        m_break_flag = true;
    }
    /*
     * Expression visitor
     */
    PopLObject operator()(const LiteralExpr& expr) const { return expr.value; }

    PopLObject operator()(const GroupingExpr& expr) const {
        return Evaluate(*expr.expression);
    }

    PopLObject operator()(const TernaryExpr& expr) const {
        PopLObject left = Evaluate(*expr.condition);
        CheckUninitialised(expr.question, left);
        if (left.isTruthy()) return Evaluate(*expr.thenBranch);
        return Evaluate(*expr.elseBranch);
    }
    PopLObject operator()(const UnaryExpr& expr) const;
    PopLObject operator()(const BinaryExpr& expr) const;
    PopLObject operator()(const VariableExpr& expr) const {
        return m_environment->Get(expr.name);
    }
    PopLObject operator()(const NilExpr& expr) const {
        return PopLObject{NilValue{}};
    }
    PopLObject operator()(const LogicalExpr& expr) const {
        auto left{Evaluate(*expr.left)};
        if (expr.op.GetType() == TokenType::OR) {
            if (left.isTruthy()) return left;
        } else {
            if (!left.isTruthy()) return left;
        }
        return Evaluate(*expr.right);
    }

   private:
    PopLObject Evaluate(const Expr& expr) const {
        return visitExpr(expr, *this);
    }
    void Execute(const Stmt& stmt) { visitStmt(stmt, *this); }

    void CheckNumberOperand(const Token& op, const PopLObject& operand) const {
        if (operand.isNumber()) return;
        throw runtime::RunTimeError(op, "Operand must be a number.");
    }
    void CheckNumberOperand(const Token& op, const PopLObject& left,
                            const PopLObject& right) const {
        if (left.isNumber() && right.isNumber()) return;
        throw runtime::RunTimeError(op, "Operands must be number");
    }
    void CheckUninitialised(const Token& op, const PopLObject& value) const {
        if (value.isUninitialized())
            throw runtime::RunTimeError(op, "Use of Uninitialized value");
    }
    void ExecuteBlock(const std::vector<Stmt>& stmts, Environment* newEnv) {
        Environment* previous = m_environment;
        try {
            m_environment = newEnv;
            for (const auto& stmt : stmts) {
                Execute(stmt);
            }
        } catch (...) {
            m_environment = previous;
            throw;
        }
        m_environment = previous;
    }

    Token MakeReplReadToken(std::string_view what = "<repl>") const {
        return Token{TokenType::IDENTIFIER, std::string(what), {}, 1};
    }

   private:
    Environment  m_global_environment{};
    Environment* m_environment{&m_global_environment};
};
};  // namespace popl
