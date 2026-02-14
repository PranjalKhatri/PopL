#include "popl/syntax/visitors/resolver.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

#include "popl/diagnostics.hpp"
#include "popl/syntax/ast/expr.hpp"
#include "popl/syntax/ast/stmt.hpp"
#include "popl/syntax/visitors/interpreter.hpp"

namespace popl {

void Resolver::Resolve(const Stmt& stmt) { visitStmt(stmt, *this); }
void Resolver::Resolve(const Expr& expr) { visitExpr(expr, *this); }
void Resolver::Declare(const Token& name) {
    if (m_scopes.empty()) return;
    m_scopes.back().insert_or_assign(name.GetLexeme(), false);
}
void Resolver::Define(const Token& name) {
    if (m_scopes.empty()) return;
    m_scopes.back().insert_or_assign(name.GetLexeme(), true);
}

void Resolver::Resolve(const std::vector<std::unique_ptr<Stmt>>& statements) {
    for (const auto& stmt : statements) Resolve(*stmt);
}

void Resolver::ResolveLocal(const Expr& expr, const Token& name) {
    // iterate from innermost scope to outermost
    for (int i = static_cast<int>(m_scopes.size()) - 1; i >= 0; --i) {
        auto& scope = m_scopes[i];
        if (scope.find(name.GetLexeme()) != scope.end()) {
            m_interpreter.Resolve(expr,
                                  static_cast<int>(m_scopes.size()) - 1 - i);
            return;
        }
    }
}

void Resolver::ResolveFunction(const FunctionStmt& stmt) {
    ScopeGuard guard(m_scopes);
    for (const Token& param : stmt.func->params) {
        Declare(param);
        Define(param);
    }
    Resolve(stmt.func->body);
}

void Resolver::operator()(const VarStmt& stmt) {
    Declare(stmt.name);
    if (stmt.initializer) Resolve(*stmt.initializer);
    Define(stmt.name);
}
void Resolver::operator()(const BlockStmt& stmt) {
    ScopeGuard guard(m_scopes);
    Resolve(stmt.statements);
}
void Resolver::operator()(const AssignStmt& stmt) {
    Resolve(*stmt.value);
    if (!m_scopes.empty()) {
        auto& currentScope = m_scopes.back();
        auto  it           = currentScope.find(stmt.name.GetLexeme());
        if (it != currentScope.end() && it->second == false) {
            Diagnostics::Error(
                stmt.name, "Can't read local variable in its own initializer.");
        }
    }
}
void Resolver::operator()(const FunctionStmt& stmt) {
    Declare(stmt.name);
    Define(stmt.name);
    ResolveFunction(stmt);
}

void Resolver::operator()(const ExpressionStmt& stmt) {
    Resolve(*stmt.expression);
}
void Resolver::operator()(const PrintStmt& stmt) { Resolve(*stmt.expression); }
void Resolver::operator()(const IfStmt& stmt) {
    Resolve(*stmt.condition);
    Resolve(*stmt.thenBranch);
    if (stmt.elseBranch) Resolve(*stmt.elseBranch);
}
void Resolver::operator()(const NilStmt& stmt) {}
void Resolver::operator()(const WhileStmt& stmt) {
    Resolve(*stmt.condition);
    Resolve(*stmt.body);
}
void Resolver::operator()(const BreakStmt& stmt) {}
void Resolver::operator()(const ContinueStmt& stmt) {}
void Resolver::operator()(const ReturnStmt& stmt) {
    if (stmt.value) Resolve(*stmt.value);
}

void Resolver::operator()(const VariableExpr& expr) {
    if (!m_scopes.empty()) {
        auto& currentScope = m_scopes.back();
        auto  it           = currentScope.find(expr.name.GetLexeme());

        if (it != currentScope.end() && it->second == false) {
            Diagnostics::Error(
                expr.name, "Can't read local variable in its own initializer.");
        }
    }
    ResolveLocal(Expr{expr}, expr.name);
}
void Resolver::operator()(const CallExpr& expr) {
    Resolve(*expr.callee);
    for (const auto& arg : expr.arguments) Resolve(*arg);
}
void Resolver::operator()(const FunctionExpr& expr) {}
void Resolver::operator()(const LogicalExpr& expr) {
    Resolve(*expr.left);
    Resolve(*expr.right);
}
void Resolver::operator()(const NilExpr& expr) {}
void Resolver::operator()(const BinaryExpr& expr) {
    Resolve(*expr.left);
    Resolve(*expr.right);
}
void Resolver::operator()(const UnaryExpr& expr) { Resolve(*expr.right); }
void Resolver::operator()(const TernaryExpr& expr) {
    Resolve(*expr.condition);
    Resolve(*expr.thenBranch);
    Resolve(*expr.elseBranch);
}
void Resolver::operator()(const GroupingExpr& expr) {
    Resolve(*expr.expression);
}
void Resolver::operator()(const LiteralExpr& expr) {}
};  // namespace popl
