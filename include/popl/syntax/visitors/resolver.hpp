#pragma once

#include "popl/syntax/ast/stmt.hpp"
namespace popl {
class Interpreter;
class Resolver {
   public:
    Resolver(Interpreter& interpreter) : m_interpreter{interpreter} {}
    void Resolve(const std::vector<std::unique_ptr<Stmt>>& statements);

    void operator()(const ExpressionStmt& stmt, const Stmt& originalStmt);
    void operator()(const PrintStmt& stmt, const Stmt&);
    void operator()(const NilStmt& stmt, const Stmt&);
    void operator()(const VarStmt& stmt, const Stmt&);
    void operator()(const BlockStmt& stmt, const Stmt&);
    void operator()(const AssignStmt& stmt, const Stmt&);
    void operator()(const IfStmt& stmt, const Stmt&);
    void operator()(const WhileStmt& stmt, const Stmt&);
    void operator()(const BreakStmt& stmt, const Stmt&);
    void operator()(const ContinueStmt& stmt, const Stmt&);
    void operator()(const ReturnStmt& stmt, const Stmt&);
    void operator()(const FunctionStmt& stmt, const Stmt&);

    void operator()(const LiteralExpr& expr, const Expr& originalExpr);
    void operator()(const GroupingExpr& expr, const Expr&);
    void operator()(const TernaryExpr& expr, const Expr&);
    void operator()(const UnaryExpr& expr, const Expr&);
    void operator()(const BinaryExpr& expr, const Expr&);
    void operator()(const VariableExpr& expr, const Expr&);
    void operator()(const NilExpr& expr, const Expr&);
    void operator()(const LogicalExpr& expr, const Expr&);
    void operator()(const CallExpr& expr, const Expr&);
    void operator()(const FunctionExpr& expr, const Expr&);

   private:
    class ScopeGuard {
       public:
        ScopeGuard(std::vector<std::unordered_map<std::string, bool>>& scopes_)
            : scopes(scopes_) {
            scopes.emplace_back(std::unordered_map<std::string, bool>{});
        }
        ~ScopeGuard() { scopes.pop_back(); }

       private:
        std::vector<std::unordered_map<std::string, bool>>& scopes;
    };
    void Declare(const Token& name);
    void Define(const Token& name);
    void ResolveLocal(const Expr& expr, const Token& name);
    void ResolveFunction(const FunctionStmt& stmt);
    void Resolve(const Stmt& statement);
    void Resolve(const Expr& expr);

   private:
    Interpreter&                                       m_interpreter;
    std::vector<std::unordered_map<std::string, bool>> m_scopes{};
};
}  // namespace popl
