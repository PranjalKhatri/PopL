#pragma once

#include <memory>
#include <print>
#include <variant>

#include "popl/diagnostics.hpp"
#include "popl/environment.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/Exceptions/run_time_error.hpp"
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
        } catch (const RunTimeError& error) {
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
        assert(false && "Nil statment not defined");
    }
    void operator()(const VarStmt& stmt) {
        PopLObject value{};
        if (!std::holds_alternative<NilExpr>(stmt.initializer->node))
            value = Evaluate(*(stmt.initializer));
        environment->Define(stmt.name, value);
    }
    void operator()(const BlockStmt& stmt) {
        Environment blockEnv(environment);
        ExecuteBlock(stmt.statements, &blockEnv);
    }
    void operator()(const AssignStmt& stmt) {
        PopLObject value{Evaluate(*(stmt.value))};
        environment->Assign(stmt.name, value);
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
        return environment->Get(expr.name);
    }
    PopLObject operator()(const NilExpr& expr) const { return PopLObject{}; }

   private:
    PopLObject Evaluate(const Expr& expr) const {
        return visitExpr(expr, *this);
    }
    void Execute(const Stmt& stmt) { visitStmt(stmt, *this); }

    void CheckNumberOperand(const Token& op, const PopLObject& operand) const {
        if (operand.isNumber()) return;
        throw RunTimeError(op, "Operand must be a number.");
    }
    void CheckNumberOperand(const Token& op, const PopLObject& left,
                            const PopLObject& right) const {
        if (left.isNumber() && right.isNumber()) return;
        throw RunTimeError(op, "Operands must be number");
    }
    void CheckUninitialised(const Token& op, const PopLObject& value) const {
        if (value.isUninitialized())
            throw RunTimeError(op, "Use of Uninitialized value");
    }
    void ExecuteBlock(const std::vector<Stmt>& stmts, Environment* newEnv) {
        Environment* previous = environment;
        try {
            environment = newEnv;
            for (const auto& stmt : stmts) {
                Execute(stmt);
            }
        } catch (...) {
            environment = previous;
            throw;
        }
        environment = previous;
    }

    Token MakeReplReadToken(std::string_view what = "<repl>") const {
        return Token{TokenType::IDENTIFIER, std::string(what), {}, 1};
    }

   private:
    Environment  m_global_environment{};
    Environment* environment{&m_global_environment};
    bool         m_repl_mode{false};
};
};  // namespace popl
