#include "popl/syntax/visitors/interpreter.hpp"

namespace popl {
PopLObject Interpreter::operator()(const UnaryExpr& expr) const {
    PopLObject right = Evaluate(*expr.right);
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
            throw RunTimeError(expr.op,
                               "Operands must be two numbers or two strings.");
            break;
        case TokenType::MINUS:
            CheckNumberOperand(expr.op, left, right);
            return PopLObject{left.asNumber() - right.asNumber()};
        case TokenType::SLASH:
            CheckNumberOperand(expr.op, left, right);
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
