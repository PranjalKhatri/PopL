#include "popl/syntax/visitors/interpreter.hpp"

namespace popl {

void Interpreter::operator()(const WhileStmt& stmt) {
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

PopLObject Interpreter::operator()(const UnaryExpr& expr) const {
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

PopLObject Interpreter::operator()(const BinaryExpr& expr) const {
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

};  // namespace popl
