#pragma once

#include <memory>

#include "popl/callables/native_registry.hpp"
#include "popl/environment.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/ast/expr.hpp"
#include "popl/syntax/ast/stmt.hpp"

namespace popl {

class Interpreter {
   public:
    Interpreter()
        : m_global_environment{std::make_shared<Environment>()},
          m_current_environment{m_global_environment} {
        NativeRegistry::RegisterAll(*this);
    }
    void Interpret(std::vector<std::unique_ptr<Stmt>>& statements,
                   bool                                replMode);
    std::shared_ptr<Environment> GetGlobalEnvironment() {
        return m_global_environment;
    }
    void ExecuteBlock(const std::vector<std::unique_ptr<Stmt>>& stmts,
                      std::shared_ptr<Environment>              newEnv);
    // Expr must be guaranteed to be alive when the interpreter visits it in
    // future
    void Resolve(const Expr& expr, int depth);
    /*
     * Statement visitor
     */
    void operator()(const ExpressionStmt& stmt, const Stmt& originalStmt);
    void operator()(const NilStmt& stmt, const Stmt&);
    void operator()(const VarStmt& stmt, const Stmt&);
    void operator()(const BlockStmt& stmt, const Stmt&);
    void operator()(const AssignStmt& stmt, const Stmt&);
    void operator()(IfStmt& stmt, const Stmt&);
    void operator()(WhileStmt& stmt, const Stmt&);
    void operator()(const BreakStmt& stmt, const Stmt&);
    void operator()(const ContinueStmt& stmt, const Stmt&);
    void operator()(const ReturnStmt& stmt, const Stmt&);
    void operator()(FunctionStmt& stmt, const Stmt&);

    /*
     * Expression visitor
     */

    PopLObject operator()(const NilExpr& expr, const Expr& originalExpr) const;
    PopLObject operator()(const LogicalExpr& expr, const Expr&);
    PopLObject operator()(const CallExpr& expr, const Expr&);
    PopLObject operator()(const FunctionExpr& expr, const Expr&);
    PopLObject operator()(const GroupingExpr& expr, const Expr&);
    PopLObject operator()(const TernaryExpr& expr, const Expr&);
    PopLObject operator()(const UnaryExpr& expr, const Expr&);
    PopLObject operator()(const BinaryExpr& expr, const Expr&);
    PopLObject operator()(const VariableExpr& expr, const Expr&) const;
    PopLObject operator()(const LiteralExpr& expr, const Expr&) const;

   private:
    PopLObject Evaluate(const Expr& expr);
    void       Execute(Stmt& stmt);
    void  CheckNumberOperand(const Token& op, const PopLObject& operand) const;
    void  CheckNumberOperand(const Token& op, const PopLObject& left,
                             const PopLObject& right) const;
    void  CheckUninitialised(const Token& op, const PopLObject& value) const;
    Token MakeReplReadToken(std::string_view what = "<repl>") const;
    const PopLObject& LookUpVariable(const Token& name, const Expr& expr) const;

   private:
    std::shared_ptr<Environment>         m_global_environment{};
    std::shared_ptr<Environment>         m_current_environment{};
    std::unordered_map<const Expr*, int> m_locals{};
    bool                                 m_repl_mode{false};
};
};  // namespace popl
