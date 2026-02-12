#include "popl/syntax/visitors/interpreter.hpp"

#include <format>
#include <memory>
#include <print>
#include <variant>

#include "popl/callable.hpp"
#include "popl/diagnostics.hpp"
#include "popl/literal.hpp"
#include "popl/popl_function.hpp"
#include "popl/runtime/run_time_error.hpp"
#include "popl/syntax/ast/stmt.hpp"

namespace popl {

void Interpreter::Interpret(std::vector<Stmt>& statements, bool replMode) {
    m_repl_mode = replMode;
    try {
        for (auto& statement : statements) {
            Execute(statement);
        }
    } catch (const runtime::RunTimeError& error) {
        Diagnostics::ReportRunTimeError(error);
    }
}

void Interpreter::operator()(const ExpressionStmt& stmt) {
    PopLObject obj = Evaluate(*(stmt.expression));
    if (m_repl_mode) {
        CheckUninitialised(MakeReplReadToken(), obj);
        std::println("{}", obj.toString());
    }
}
void Interpreter::operator()(const PrintStmt& stmt) {
    PopLObject value = Evaluate(*(stmt.expression));
    CheckUninitialised(MakeReplReadToken("Print"), value);
    println("{}", value.toString());
}
void Interpreter::operator()(const NilStmt& stmt) {
    // Yeah.. do nothing
}
void Interpreter::operator()(const VarStmt& stmt) {
    PopLObject value{};
    if (!std::holds_alternative<NilExpr>(stmt.initializer->node))
        value = Evaluate(*(stmt.initializer));
    environment->Define(stmt.name, value);
}
void Interpreter::operator()(const BlockStmt& stmt) {
    Environment blockEnv(environment);
    ExecuteBlock(stmt.statements, &blockEnv);
}
void Interpreter::operator()(const AssignStmt& stmt) {
    PopLObject value{Evaluate(*(stmt.value))};
    environment->Assign(stmt.name, value);
}
void Interpreter::operator()(IfStmt& stmt) {
    if (Evaluate(*stmt.condition).isTruthy())
        Execute(*stmt.thenBranch);
    else
        Execute(*stmt.elseBranch);
}
void Interpreter::operator()(const BreakStmt& stmt) {
    if (m_loop_depth > 0) throw runtime::control_flow::BreakSignal{};
    throw runtime::RunTimeError{
        stmt.keyword, "'break' statement can't be used outside of loops."};
}
void Interpreter::operator()(const ContinueStmt& stmt) {
    if (m_loop_depth > 0) throw runtime::control_flow::ContinueSignal{};
    throw runtime::RunTimeError{
        stmt.keyword, "'continue' statement can't be used outside of loops."};
}

void Interpreter::operator()(WhileStmt& stmt) {
    LoopGuard guard{m_loop_depth};
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
void Interpreter::operator()(FunctionStmt& stmt) {
    auto func = std::make_shared<callable::PoplFunction>(std::move(stmt));

    environment->Define(stmt.name, PopLObject{func});
}
/*
 * Expression visitor
 */
PopLObject Interpreter::operator()(const LiteralExpr& expr) const {
    return expr.value;
}

PopLObject Interpreter::operator()(const GroupingExpr& expr) {
    return Evaluate(*expr.expression);
}

PopLObject Interpreter::operator()(const TernaryExpr& expr) {
    PopLObject left = Evaluate(*expr.condition);
    CheckUninitialised(expr.question, left);
    if (left.isTruthy()) return Evaluate(*expr.thenBranch);
    return Evaluate(*expr.elseBranch);
}
PopLObject Interpreter::operator()(const VariableExpr& expr) const {
    return environment->Get(expr.name);
}
PopLObject Interpreter::operator()(const NilExpr& expr) const {
    return PopLObject{NilValue{}};
}
PopLObject Interpreter::operator()(const LogicalExpr& expr) {
    auto left{Evaluate(*expr.left)};
    if (expr.op.GetType() == TokenType::OR) {
        if (left.isTruthy()) return left;
    } else {
        if (!left.isTruthy()) return left;
    }
    return Evaluate(*expr.right);
}

PopLObject Interpreter::operator()(const CallExpr& expr) {
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
PopLObject Interpreter::Evaluate(const Expr& expr) {
    return visitExpr(expr, *this);
}

PopLObject Interpreter::operator()(const UnaryExpr& expr) {
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
    return {};
}

PopLObject Interpreter::operator()(const BinaryExpr& expr) {
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
        default:
            break;
    }
    // Unreachable
    return {};
}
void Interpreter::Execute(Stmt& stmt) { visitStmt(stmt, *this); }

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
    return Token{TokenType::IDENTIFIER, std::string(what), {}, 1};
}
void Interpreter::ExecuteBlock(const std::vector<std::unique_ptr<Stmt>>& stmts,
                               Environment* newEnv) {
    Environment* previous = environment;
    try {
        environment = newEnv;
        for (const auto& stmt : stmts) {
            Execute(*stmt);
        }
    } catch (...) {
        environment = previous;
        throw;
    }
    environment = previous;
}
};  // namespace popl
