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
    void operator()(IfStmt& stmt, Stmt&);
    void operator()(WhileStmt& stmt, Stmt&);
    void operator()(BreakStmt& stmt, Stmt&);
    void operator()(ContinueStmt& stmt, Stmt&);
    void operator()(ReturnStmt& stmt, Stmt&);
    void operator()(FunctionStmt& stmt, Stmt&);
    void operator()(ClassStmt& stmt, Stmt&);

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
    void operator()(AssignExpr& expr, Expr&);
    void operator()(FunctionExpr& expr, Expr&);
    void operator()(GetExpr& expr, Expr&);
    void operator()(SetExpr& expr, Expr&);
    void operator()(ThisExpr& expr, Expr&);

   private:
    enum class FunctionType { NONE, FUNCTION, METHOD };
    enum class ClassType { NONE, CLASS };
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

    template <typename T>
        requires requires(T t) { t.depth; }
    void ResolveLocal(T& expr, const Token& name) {
        for (int i = static_cast<int>(m_scopes.size()) - 1; i >= 0; --i) {
            auto it = m_scopes[i].find(name.GetLexeme());
            if (it != m_scopes[i].end()) {
                it->second.used = true;
                expr.depth      = static_cast<int>(m_scopes.size()) - 1 - i;
                return;
            }
        }
    }
    void ResolveFunction(FunctionExpr& expr, FunctionType type);

    void Resolve(Stmt& statement);
    void Resolve(Expr& expr);

   private:
    Interpreter& m_interpreter;

    std::vector<std::unordered_map<std::string, VariableInfo>> m_scopes{};

    FunctionType m_current_function_type{FunctionType::NONE};
    ClassType    m_current_class_type{ClassType::NONE};
    LoopType     m_current_loop_type{LoopType::NONE};
};

}  // namespace popl
