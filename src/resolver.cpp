#include "popl/syntax/visitors/resolver.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

#include "popl/diagnostics.hpp"
#include "popl/syntax/ast/expr.hpp"
#include "popl/syntax/ast/stmt.hpp"

namespace popl {
Resolver::ScopeGuard::~ScopeGuard() {
    for (const auto& [_, info] : scopes.back()) {
        if (info.defined && !info.used) {
            Diagnostics::Error(
                info.keyword,
                "Unused local variable '" + info.keyword.GetLexeme() + "'.");
        }
    }
    scopes.pop_back();
}
void Resolver::Resolve(Stmt& stmt) {
    visitStmtWithArgs(
        stmt,
        [&stmt, this](auto&& contained, Stmt& originalStmt) {
            return (*this)(contained, originalStmt);
        },
        stmt);
}
void Resolver::Resolve(Expr& expr) {
    visitExprWithArgs(
        expr,
        [&expr, this](auto&& contained, Expr& originalExpr) {
            return (*this)(contained, originalExpr);
        },
        expr);
}
void Resolver::Declare(const Token& name) {
    if (m_scopes.empty()) return;
    if (m_scopes.back().contains(name.GetLexeme()))
        Diagnostics::Error(
            name,
            std::format("Variable with name {} already exists in this scop.",
                        name.GetLexeme()));

    m_scopes.back().insert_or_assign(
        name.GetLexeme(),
        VariableInfo{.defined = false, .used = false, .keyword = name});
}
void Resolver::Define(const Token& name) {
    if (m_scopes.empty()) return;
    m_scopes.back().insert_or_assign(
        name.GetLexeme(),
        VariableInfo{.defined = true, .used = false, .keyword = name});
}

void Resolver::Resolve(std::vector<std::unique_ptr<Stmt>>& statements) {
    for (auto& stmt : statements) Resolve(*stmt);
}

void Resolver::ResolveLocal(VariableExpr& expr, const Token& name) {
    // iterate from innermost scope to outermost
    for (int i = static_cast<int>(m_scopes.size()) - 1; i >= 0; --i) {
        auto it = m_scopes[i].find(name.GetLexeme());
        if (it != m_scopes[i].end()) {
            it->second.used = true;
            expr.depth      = static_cast<int>(m_scopes.size()) - 1 - i;
            return;
        }
    }
}

void Resolver::ResolveFunction(FunctionExpr& expr, FunctionType funcType) {
    ScopeGuard guard(m_scopes);

    FunctionType enclosingFunction = m_current_function_type;
    m_current_function_type        = funcType;

    for (Token& param : expr.params) {
        Declare(param);
        Define(param);
    }
    Resolve(expr.body);
    m_current_function_type = enclosingFunction;
}

void Resolver::operator()(VarStmt& stmt, Stmt&) {
    Declare(stmt.name);
    if (stmt.initializer) Resolve(*stmt.initializer);
    Define(stmt.name);
}
void Resolver::operator()(BlockStmt& stmt, Stmt&) {
    ScopeGuard guard(m_scopes);
    Resolve(stmt.statements);
}
void Resolver::operator()(AssignStmt& stmt, Stmt&) {
    Resolve(*stmt.value);
    if (!m_scopes.empty()) {
        auto& currentScope = m_scopes.back();
        auto  it           = currentScope.find(stmt.name.GetLexeme());
        if (it != currentScope.end() && it->second.defined == false) {
            Diagnostics::Error(
                stmt.name, "Can't read local variable in its own initializer.");
        }
    }
}
void Resolver::operator()(FunctionStmt& stmt, Stmt&) {
    Declare(stmt.name);
    Define(stmt.name);
    ResolveFunction(*stmt.func, FunctionType::FUNCTION);
}

void Resolver::operator()(ExpressionStmt& stmt, Stmt&) {
    Resolve(*stmt.expression);
}
void Resolver::operator()(IfStmt& stmt, Stmt&) {
    Resolve(*stmt.condition);
    Resolve(*stmt.thenBranch);
    if (stmt.elseBranch) Resolve(*stmt.elseBranch);
}
void Resolver::operator()(NilStmt& stmt, Stmt&) {}
void Resolver::operator()(WhileStmt& stmt, Stmt&) {
    LoopType enclosing  = m_current_loop_type;
    m_current_loop_type = LoopType::LOOP;

    Resolve(*stmt.condition);
    Resolve(*stmt.body);

    m_current_loop_type = enclosing;
}
void Resolver::operator()(BreakStmt& stmt, Stmt&) {
    if (m_current_loop_type != LoopType::LOOP)
        Diagnostics::Error(stmt.keyword,
                           "'break' statement not inside a loop.");
}
void Resolver::operator()(ContinueStmt& stmt, Stmt&) {
    if (m_current_loop_type != LoopType::LOOP)
        Diagnostics::Error(stmt.keyword,
                           "'continue' statement not inside a loop.");
}
void Resolver::operator()(ReturnStmt& stmt, Stmt&) {
    if (m_current_function_type == FunctionType::NONE) {
        Diagnostics::Error(stmt.keyword, "Can't return from top-level code.");
    }
    if (stmt.value) Resolve(*stmt.value);
}

void Resolver::operator()(VariableExpr& expr, Expr& originalExpr) {
    if (!m_scopes.empty()) {
        auto& currentScope = m_scopes.back();
        auto  it           = currentScope.find(expr.name.GetLexeme());

        if (it != currentScope.end() && it->second.defined == false) {
            Diagnostics::Error(
                expr.name, "Can't read local variable in its own initializer.");
        }
    }
    ResolveLocal(expr, expr.name);
}
void Resolver::operator()(CallExpr& expr, Expr&) {
    Resolve(*expr.callee);
    for (auto& arg : expr.arguments) Resolve(*arg);
}
void Resolver::operator()(FunctionExpr& expr, Expr&) {
    ResolveFunction(expr, FunctionType::FUNCTION);
}
void Resolver::operator()(LogicalExpr& expr, Expr&) {
    Resolve(*expr.left);
    Resolve(*expr.right);
}
void Resolver::operator()(NilExpr& expr, Expr&) {}
void Resolver::operator()(BinaryExpr& expr, Expr&) {
    Resolve(*expr.left);
    Resolve(*expr.right);
}
void Resolver::operator()(UnaryExpr& expr, Expr&) { Resolve(*expr.right); }
void Resolver::operator()(TernaryExpr& expr, Expr&) {
    Resolve(*expr.condition);
    Resolve(*expr.thenBranch);
    Resolve(*expr.elseBranch);
}
void Resolver::operator()(GroupingExpr& expr, Expr&) {
    Resolve(*expr.expression);
}
void Resolver::operator()(LiteralExpr& expr, Expr&) {}
};  // namespace popl
