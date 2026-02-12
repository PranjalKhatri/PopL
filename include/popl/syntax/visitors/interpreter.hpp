#pragma once

#include "popl/environment.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/ast/expr.hpp"
#include "popl/syntax/ast/stmt.hpp"

namespace popl {

// TODO: Reason about environment pointer
class Interpreter {
   public:
    void         Interpret(std::vector<Stmt>& statements, bool replMode);
    Environment* GetGlobalEnvironment() { return &m_global_environment; }
    void         ExecuteBlock(const std::vector<std::unique_ptr<Stmt>>& stmts,
                              Environment*                              newEnv);
    /*
     * Statement visitor
     */
    void         operator()(const ExpressionStmt& stmt);
    void         operator()(const PrintStmt& stmt);
    void         operator()(const NilStmt& stmt);
    void         operator()(const VarStmt& stmt);
    void         operator()(const BlockStmt& stmt);
    void         operator()(const AssignStmt& stmt);
    void         operator()(IfStmt& stmt);
    void         operator()(WhileStmt& stmt);
    void         operator()(const BreakStmt& stmt);
    void         operator()(const ContinueStmt& stmt);
    void         operator()(const ReturnStmt& stmt);
    void         operator()(FunctionStmt& stmt);

    /*
     * Expression visitor
     */
    PopLObject operator()(const LiteralExpr& expr) const;
    PopLObject operator()(const GroupingExpr& expr);
    PopLObject operator()(const TernaryExpr& expr);
    PopLObject operator()(const UnaryExpr& expr);
    PopLObject operator()(const BinaryExpr& expr);
    PopLObject operator()(const VariableExpr& expr) const;
    PopLObject operator()(const NilExpr& expr) const;
    PopLObject operator()(const LogicalExpr& expr);
    PopLObject operator()(const CallExpr& expr);

   private:
    PopLObject Evaluate(const Expr& expr);
    void       Execute(Stmt& stmt);
    void  CheckNumberOperand(const Token& op, const PopLObject& operand) const;
    void  CheckNumberOperand(const Token& op, const PopLObject& left,
                             const PopLObject& right) const;
    void  CheckUninitialised(const Token& op, const PopLObject& value) const;
    Token MakeReplReadToken(std::string_view what = "<repl>") const;

   private:
    Environment  m_global_environment{};
    Environment* environment{&m_global_environment};
    bool         m_repl_mode{false};
};
};  // namespace popl
