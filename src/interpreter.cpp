#include "popl/syntax/visitors/interpreter.hpp"

#include <format>
#include <memory>
#include <print>

#include "popl/callables/callable.hpp"
#include "popl/callables/popl_function.hpp"
#include "popl/diagnostics.hpp"
#include "popl/lexer/token_types.hpp"
#include "popl/literal.hpp"
#include "popl/runtime/control_flow.hpp"
#include "popl/runtime/popl_class.hpp"
#include "popl/runtime/run_time_error.hpp"
#include "popl/syntax/ast/expr.hpp"
#include "popl/syntax/ast/stmt.hpp"

namespace popl {

void Interpreter::Interpret(std::vector<std::unique_ptr<Stmt>>& statements,
                            bool                                replMode) {
    m_repl_mode = replMode;
    try {
        for (auto& statement : statements) {
            if (replMode) {
                // Move into persistent storage
                m_persistent_statements.emplace_back(std::move(statement));
                // execute stored one
                Execute(*m_persistent_statements.back());
            } else {  // normal execution
                Execute(*statement);
            }
        }
    } catch (const runtime::RunTimeError& error) {
        Diagnostics::ReportRunTimeError(error);
    }
}
void Interpreter::Resolve(const Expr& expr, int depth) {
    m_locals[&expr] = depth;
}

void Interpreter::operator()(const ExpressionStmt& stmt, const Stmt&) {
    PopLObject obj = Evaluate(*(stmt.expression));
    if (m_repl_mode) {
        CheckUninitialised(MakeReplReadToken(), obj);
        if (!obj.isNil()) std::println("{}", obj.toString());
    }
}
void Interpreter::operator()(const NilStmt& stmt, const Stmt&) {
    // Yeah.. do nothing
}
void Interpreter::operator()(const VarStmt& stmt, const Stmt&) {
    PopLObject value{UninitializedValue{}};
    if (stmt.initializer) value = Evaluate(*(stmt.initializer));
    m_current_environment->Define(stmt.name, value);
}
void Interpreter::operator()(const BlockStmt& stmt, const Stmt&) {
    auto blockEnv = std::make_shared<Environment>(m_current_environment);
    ExecuteBlock(stmt.statements, blockEnv);
}
void Interpreter::operator()(const AssignStmt& stmt, const Stmt&) {
    PopLObject value{Evaluate(*(stmt.value))};
    m_current_environment->Assign(stmt.name, value);
}
void Interpreter::operator()(IfStmt& stmt, const Stmt&) {
    if (Evaluate(*stmt.condition).isTruthy())
        Execute(*stmt.thenBranch);
    else
        Execute(*stmt.elseBranch);
}
void Interpreter::operator()(const BreakStmt& stmt, const Stmt&) {
    throw runtime::control_flow::BreakSignal{};
}
void Interpreter::operator()(const ContinueStmt& stmt, const Stmt&) {
    throw runtime::control_flow::ContinueSignal{};
}
void Interpreter::operator()(const ReturnStmt& stmt, const Stmt&) {
    auto value = stmt.value ? Evaluate(*stmt.value) : PopLObject{NilValue{}};
    throw runtime::control_flow::ReturnSignal{std::move(value)};
}

void Interpreter::operator()(WhileStmt& stmt, const Stmt&) {
    while (Evaluate(*stmt.condition).isTruthy()) {
        try {
            Execute(*stmt.body);
        } catch (const runtime::control_flow::ContinueSignal&) {
            continue;
        } catch (const runtime::control_flow::BreakSignal&) {
            break;
        }
    }
}
void Interpreter::operator()(FunctionStmt& stmt, const Stmt&) {
    Token name = stmt.name;
    auto  func = std::make_shared<callable::PoplFunction>(
        stmt.func.get(), m_current_environment, stmt.name.GetLexeme());
    m_current_environment->Define(name, PopLObject{func});
}
void Interpreter::operator()(ClassStmt& stmt, const Stmt&) {
    m_current_environment->Define(stmt.name, PopLObject(NilValue{}));
    auto klass = std::make_shared<runtime::PoplClass>(stmt.name.GetLexeme());
    m_current_environment->Assign(stmt.name, PopLObject{klass});
}

/*
 * Expression visitor
 */
PopLObject Interpreter::operator()(const LiteralExpr& expr, const Expr&) const {
    return expr.value;
}

PopLObject Interpreter::operator()(const GroupingExpr& expr, const Expr&) {
    return Evaluate(*expr.expression);
}

PopLObject Interpreter::operator()(const TernaryExpr& expr, const Expr&) {
    PopLObject left = Evaluate(*expr.condition);
    CheckUninitialised(expr.question, left);
    if (left.isTruthy()) return Evaluate(*expr.thenBranch);
    return Evaluate(*expr.elseBranch);
}
PopLObject Interpreter::operator()(const VariableExpr& expr,
                                   const Expr&         originalExpr) const {
    if (expr.depth.has_value())
        return m_current_environment->GetAt(expr.depth.value(), expr.name);
    return m_global_environment->Get(expr.name);
}
PopLObject Interpreter::operator()(const NilExpr& expr, const Expr&) const {
    return PopLObject{NilValue{}};
}
PopLObject Interpreter::operator()(const LogicalExpr& expr, const Expr&) {
    auto left{Evaluate(*expr.left)};
    if (expr.op.GetType() == TokenType::OR) {
        if (left.isTruthy()) return left;
    } else {
        if (!left.isTruthy()) return left;
    }
    return Evaluate(*expr.right);
}

PopLObject Interpreter::operator()(const CallExpr& expr, const Expr&) {
    PopLObject              callee{Evaluate(*expr.callee)};
    std::vector<PopLObject> args;
    for (const auto& expr : expr.arguments) args.emplace_back(Evaluate(*expr));
    if (!callee.isCallable())
        throw runtime::RunTimeError(expr.ClosingParen,
                                    "Can only call function and classes.");
    PopLObject::CallablePtr func{callee.asCallable()};
    if (func->GetArity() != args.size())
        throw runtime::RunTimeError(
            expr.ClosingParen, std::format("Expected {} arguments but got {}.",
                                           func->GetArity(), args.size()));
    return func->Call(*this, args);
}

PopLObject Interpreter::operator()(const FunctionExpr& expr, const Expr&) {
    auto function = std::make_shared<callable::PoplFunction>(
        &expr, m_current_environment, std::nullopt);

    return PopLObject{function};
}

PopLObject Interpreter::Evaluate(const Expr& expr) {
    return visitExprWithArgs(
        expr,
        [&expr, this](auto&& contained, const Expr& originalExpr) {
            return (*this)(contained, originalExpr);
        },
        expr);
}

PopLObject Interpreter::operator()(const UnaryExpr& expr, const Expr&) {
    PopLObject right = Evaluate(*expr.right);
    CheckUninitialised(expr.op, right);
    switch (expr.op.GetType()) {
        case TokenType::MINUS:
            CheckNumberOperand(expr.op, right);
            return PopLObject{-right.asNumber()};
        case TokenType::BANG:
            return PopLObject{right.isTruthy()};
        default:
            break;
    }
    // Unreachable
    return PopLObject{NilValue{}};
}

PopLObject Interpreter::operator()(const GetExpr& expr, const Expr&) {
    auto obj{Evaluate(*expr.object)};
    if (obj.isInstance()) return obj.asInstance()->Get(expr.name);
    throw runtime::RunTimeError(expr.name, "Only instances have properties.");
}

PopLObject Interpreter::operator()(const BinaryExpr& expr, const Expr&) {
    PopLObject left  = Evaluate(*expr.left);
    PopLObject right = Evaluate(*expr.right);

    CheckUninitialised(expr.op, left);
    CheckUninitialised(expr.op, right);

    switch (expr.op.GetType()) {
        case TokenType::EQUAL_EQUAL:
            return PopLObject{left == right};
        case TokenType::GREATER:
            CheckNumberOperand(expr.op, left, right);
            return PopLObject{left.asNumber() > right.asNumber()};
        case TokenType::LESS:
            CheckNumberOperand(expr.op, left, right);
            return PopLObject{left.asNumber() < right.asNumber()};
        case TokenType::GREATER_EQUAL:
            CheckNumberOperand(expr.op, left, right);
            return PopLObject{left.asNumber() >= right.asNumber()};
        case TokenType::LESS_EQUAL:
            CheckNumberOperand(expr.op, left, right);
            return PopLObject{left.asNumber() <= right.asNumber()};
        case TokenType::PLUS:
            if (left.isNumber() && right.isNumber())
                return PopLObject{left.asNumber() + right.asNumber()};
            if (left.isString() && right.isString())
                return PopLObject{left.asString() + right.asString()};
            if (left.isString() || right.isString())
                return PopLObject{left.toString() + right.toString()};
            throw runtime::RunTimeError(
                expr.op, "Operands must be two numbers or two strings.");
            break;
        case TokenType::MINUS:
            CheckNumberOperand(expr.op, left, right);
            return PopLObject{left.asNumber() - right.asNumber()};
        case TokenType::SLASH:
            CheckNumberOperand(expr.op, left, right);
            if (right.asNumber() == 0.0)
                throw runtime::RunTimeError(expr.op, "Division by zero!");
            return PopLObject{left.asNumber() / right.asNumber()};
        case TokenType::STAR:
            CheckNumberOperand(expr.op, left, right);
            return PopLObject{left.asNumber() * right.asNumber()};
        case TokenType::COMMA:
            return right;
        default:
            break;
    }
    // Unreachable
    return PopLObject{NilValue{}};
}
void Interpreter::Execute(Stmt& stmt) {
    visitStmtWithArgs(
        stmt,
        [this, &stmt](auto&& contained, Stmt& originalStmt) {
            return (*this)(contained, originalStmt);
        },
        stmt);
}

void Interpreter::CheckNumberOperand(const Token&      op,
                                     const PopLObject& operand) const {
    if (operand.isNumber()) return;
    throw runtime::RunTimeError(op, "Operand must be a number.");
}
void Interpreter::CheckNumberOperand(const Token& op, const PopLObject& left,
                                     const PopLObject& right) const {
    if (left.isNumber() && right.isNumber()) return;
    throw runtime::RunTimeError(op, "Operands must be number");
}
void Interpreter::CheckUninitialised(const Token&      op,
                                     const PopLObject& value) const {
    if (value.isUninitialized())
        throw runtime::RunTimeError(op, "Use of Uninitialized value");
}
Token Interpreter::MakeReplReadToken(std::string_view what) const {
    return Token{TokenType::IDENTIFIER, std::string(what),
                 PopLObject{NilValue{}}, 1};
}
void Interpreter::ExecuteBlock(const std::vector<std::unique_ptr<Stmt>>& stmts,
                               std::shared_ptr<Environment> newEnv) {
    auto previous = m_current_environment;
    try {
        m_current_environment = newEnv;
        for (const auto& stmt : stmts) {
            Execute(*stmt);
        }
    } catch (...) {
        m_current_environment = previous;
        throw;
    }
    m_current_environment = previous;
}

const PopLObject& Interpreter::LookUpVariable(const Token& name,
                                              const Expr&  expr) const {
    auto it = m_locals.find(&expr);
    if (it != m_locals.end()) {
        int distance = it->second;
        return m_current_environment->GetAt(distance, name);
    } else {
        return m_global_environment->Get(name);
    }
}
};  // namespace popl
