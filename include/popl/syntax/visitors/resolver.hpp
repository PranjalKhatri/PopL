#pragma once

#include "popl/syntax/ast/stmt.hpp"

namespace popl {

class Interpreter;

class Resolver {
   public:
    Resolver(Interpreter& interpreter) : m_interpreter{interpreter} {}

    void Resolve(std::vector<std::unique_ptr<Stmt>>& statements);

    //  Statement visitors
    void operator()(ExpressionStmt& stmt, Stmt& originalStmt);
    void operator()(NilStmt& stmt, Stmt&);
    void operator()(VarStmt& stmt, Stmt&);
    void operator()(BlockStmt& stmt, Stmt&);
    void operator()(AssignStmt& stmt, Stmt&);
    void operator()(IfStmt& stmt, Stmt&);
    void operator()(WhileStmt& stmt, Stmt&);
    void operator()(BreakStmt& stmt, Stmt&);
    void operator()(ContinueStmt& stmt, Stmt&);
    void operator()(ReturnStmt& stmt, Stmt&);
    void operator()(FunctionStmt& stmt, Stmt&);

    //  Expression visitors
    void operator()(LiteralExpr& expr, Expr& originalExpr);
    void operator()(GroupingExpr& expr, Expr&);
    void operator()(TernaryExpr& expr, Expr&);
    void operator()(UnaryExpr& expr, Expr&);
    void operator()(BinaryExpr& expr, Expr&);
    void operator()(VariableExpr& expr, Expr&);
    void operator()(NilExpr& expr, Expr&);
    void operator()(LogicalExpr& expr, Expr&);
    void operator()(CallExpr& expr, Expr&);
    void operator()(FunctionExpr& expr, Expr&);

   private:
    enum class FunctionType { NONE, FUNCTION };
    enum class LoopType { NONE, LOOP };
    struct VariableInfo {
        bool  defined = false;
        bool  used    = false;
        Token keyword;
    };
    class ScopeGuard {
       public:
        ScopeGuard(
            std::vector<std::unordered_map<std::string, VariableInfo>>& scopes_)
            : scopes(scopes_) {
            scopes.emplace_back();
        }
        ~ScopeGuard();

       private:
        std::vector<std::unordered_map<std::string, VariableInfo>>& scopes;
    };

    void Declare(const Token& name);
    void Define(const Token& name);

    void ResolveLocal(VariableExpr& expr, const Token& name);
    void ResolveFunction(FunctionExpr& expr, FunctionType type);

    void Resolve(Stmt& statement);
    void Resolve(Expr& expr);

   private:
    Interpreter& m_interpreter;

    std::vector<std::unordered_map<std::string, VariableInfo>> m_scopes{};

    FunctionType m_current_function_type{FunctionType::NONE};
    LoopType     m_current_loop_type{LoopType::NONE};
};

}  // namespace popl
