#include "popl/syntax/visitors/resolver.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

#include "popl/diagnostics.hpp"
#include "popl/syntax/ast/expr.hpp"
#include "popl/syntax/ast/stmt.hpp"
#include "popl/syntax/visitors/interpreter.hpp"

namespace popl {

void Resolver::Resolve(const Stmt& stmt) {
    visitStmtWithArgs(
        stmt,
        [&stmt, this](auto&& contained, const Stmt& originalStmt) {
            return (*this)(contained, originalStmt);
        },
        stmt);
}
void Resolver::Resolve(const Expr& expr) {
    visitExprWithArgs(
        expr,
        [&expr, this](auto&& contained, const Expr& originalExpr) {
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

void Resolver::ResolveFunction(const FunctionStmt& stmt,
                               FunctionType        funcType) {
    ScopeGuard guard(m_scopes);

    FunctionType enclosingFunction = m_current_function_type;
    m_current_function_type        = funcType;

    for (const Token& param : stmt.func->params) {
        Declare(param);
        Define(param);
    }
    Resolve(stmt.func->body);
    m_current_function_type = enclosingFunction;
}

void Resolver::operator()(const VarStmt& stmt, const Stmt&) {
    Declare(stmt.name);
    if (stmt.initializer) Resolve(*stmt.initializer);
    Define(stmt.name);
}
void Resolver::operator()(const BlockStmt& stmt, const Stmt&) {
    ScopeGuard guard(m_scopes);
    Resolve(stmt.statements);
}
void Resolver::operator()(const AssignStmt& stmt, const Stmt&) {
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
void Resolver::operator()(const FunctionStmt& stmt, const Stmt&) {
    Declare(stmt.name);
    Define(stmt.name);
    ResolveFunction(stmt, FunctionType::FUNCTION);
}

void Resolver::operator()(const ExpressionStmt& stmt, const Stmt&) {
    Resolve(*stmt.expression);
}
void Resolver::operator()(const IfStmt& stmt, const Stmt&) {
    Resolve(*stmt.condition);
    Resolve(*stmt.thenBranch);
    if (stmt.elseBranch) Resolve(*stmt.elseBranch);
}
void Resolver::operator()(const NilStmt& stmt, const Stmt&) {}
void Resolver::operator()(const WhileStmt& stmt, const Stmt&) {
    LoopType enclosing  = m_current_loop_type;
    m_current_loop_type = LoopType::LOOP;

    Resolve(*stmt.condition);
    Resolve(*stmt.body);

    m_current_loop_type = enclosing;
}
void Resolver::operator()(const BreakStmt& stmt, const Stmt&) {
    if (m_current_loop_type != LoopType::LOOP)
        Diagnostics::Error(stmt.keyword,
                           "'break' statement not inside a loop.");
}
void Resolver::operator()(const ContinueStmt& stmt, const Stmt&) {
    if (m_current_loop_type != LoopType::LOOP)
        Diagnostics::Error(stmt.keyword,
                           "'continue' statement not inside a loop.");
}
void Resolver::operator()(const ReturnStmt& stmt, const Stmt&) {
    if (m_current_function_type == FunctionType::NONE) {
        Diagnostics::Error(stmt.keyword, "Can't return from top-level code.");
    }
    if (stmt.value) Resolve(*stmt.value);
}

void Resolver::operator()(const VariableExpr& expr, const Expr& originalExpr) {
    if (!m_scopes.empty()) {
        auto& currentScope = m_scopes.back();
        auto  it           = currentScope.find(expr.name.GetLexeme());

        if (it != currentScope.end() && it->second == false) {
            Diagnostics::Error(
                expr.name, "Can't read local variable in its own initializer.");
        }
    }
    ResolveLocal(originalExpr, expr.name);
}
void Resolver::operator()(const CallExpr& expr, const Expr&) {
    Resolve(*expr.callee);
    for (const auto& arg : expr.arguments) Resolve(*arg);
}
void Resolver::operator()(const FunctionExpr& expr, const Expr&) {}
void Resolver::operator()(const LogicalExpr& expr, const Expr&) {
    Resolve(*expr.left);
    Resolve(*expr.right);
}
void Resolver::operator()(const NilExpr& expr, const Expr&) {}
void Resolver::operator()(const BinaryExpr& expr, const Expr&) {
    Resolve(*expr.left);
    Resolve(*expr.right);
}
void Resolver::operator()(const UnaryExpr& expr, const Expr&) {
    Resolve(*expr.right);
}
void Resolver::operator()(const TernaryExpr& expr, const Expr&) {
    Resolve(*expr.condition);
    Resolve(*expr.thenBranch);
    Resolve(*expr.elseBranch);
}
void Resolver::operator()(const GroupingExpr& expr, const Expr&) {
    Resolve(*expr.expression);
}
void Resolver::operator()(const LiteralExpr& expr, const Expr&) {}
};  // namespace popl
