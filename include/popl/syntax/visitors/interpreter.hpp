#pragma once

#include <print>
#include <variant>

#include "popl/diagnostics.hpp"
#include "popl/environment.hpp"
#include "popl/literal.hpp"
#include "popl/runtime/control_flow.hpp"
#include "popl/runtime/run_time_error.hpp"
#include "popl/syntax/ast/expr.hpp"
#include "popl/syntax/ast/stmt.hpp"

namespace popl {

// TODO: Reason about environment pointer
class Interpreter {
   public:
    void Interpret(const std::vector<Stmt>& statements, bool replMode);

    /*
     * Statement visitor
     */
    void operator()(const ExpressionStmt& stmt);
    void operator()(const PrintStmt& stmt);
    void operator()(const NilStmt& stmt);
    void operator()(const VarStmt& stmt);
    void operator()(const BlockStmt& stmt);
    void operator()(const AssignStmt& stmt);
    void operator()(const IfStmt& stmt);
    void operator()(const WhileStmt& stmt);
    void operator()(const BreakStmt& stmt);
    void operator()(const ContinueStmt& stmt);

    /*
     * Expression visitor
     */
    PopLObject operator()(const LiteralExpr& expr) const;

    PopLObject operator()(const GroupingExpr& expr) const;

    PopLObject operator()(const TernaryExpr& expr) const;
    PopLObject operator()(const UnaryExpr& expr) const;
    PopLObject operator()(const BinaryExpr& expr) const;
    PopLObject operator()(const VariableExpr& expr) const;
    PopLObject operator()(const NilExpr& expr) const;
    PopLObject operator()(const LogicalExpr& expr) const;

   private:
    /// RAII class for loop nesting counter
    class LoopGuard {
       public:
        explicit LoopGuard(int& depth) : m_depth(depth) { ++m_depth; }

        ~LoopGuard() { --m_depth; }

       private:
        int& m_depth;
    };
    PopLObject Evaluate(const Expr& expr) const;
    void       Execute(const Stmt& stmt);
    void  CheckNumberOperand(const Token& op, const PopLObject& operand) const;
    void  CheckNumberOperand(const Token& op, const PopLObject& left,
                             const PopLObject& right) const;
    void  CheckUninitialised(const Token& op, const PopLObject& value) const;
    void  ExecuteBlock(const std::vector<Stmt>& stmts, Environment* newEnv);
    Token MakeReplReadToken(std::string_view what = "<repl>") const;

   private:
    Environment  m_global_environment{};
    Environment* environment{&m_global_environment};
    bool         m_repl_mode{false};
    int          m_loop_depth{0};
};
};  // namespace popl
