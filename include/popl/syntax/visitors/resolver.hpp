#pragma once

#include "popl/syntax/ast/stmt.hpp"
namespace popl {
class Interpreter;
class Resolver {
   public:
    Resolver(Interpreter& interpreter) : m_interpreter{interpreter} {}

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
    void operator()(const ReturnStmt& stmt);
    void operator()(const FunctionStmt& stmt);

    void operator()(const LiteralExpr& expr);
    void operator()(const GroupingExpr& expr);
    void operator()(const TernaryExpr& expr);
    void operator()(const UnaryExpr& expr);
    void operator()(const BinaryExpr& expr);
    void operator()(const VariableExpr& expr);
    void operator()(const NilExpr& expr);
    void operator()(const LogicalExpr& expr);
    void operator()(const CallExpr& expr);
    void operator()(const FunctionExpr& expr);

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
    void Resolve(const std::vector<std::unique_ptr<Stmt>>& statements);
    void Resolve(const Expr& expr);

   private:
    Interpreter&                                       m_interpreter;
    std::vector<std::unordered_map<std::string, bool>> m_scopes{};
};
}  // namespace popl
