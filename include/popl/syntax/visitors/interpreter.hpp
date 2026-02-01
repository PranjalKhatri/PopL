#pragma once

#include "popl/lexer/token_types.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/ast/expr.hpp"
namespace popl {
struct Interpreter {
    PopLObject Evaluate(const Expr& expr) { return visitExpr(expr, *this); }

    PopLObject operator()(const LiteralExpr& expr) { return expr.value; }

    PopLObject operator()(const GroupingExpr& expr) {
        return Evaluate(*expr.expression);
    }

    PopLObject operator()(const UnaryExpr& expr) {
        PopLObject right = Evaluate(*expr.right);
        switch (expr.op.GetType()) {
            case TokenType::MINUS:
                return PopLObject{-right.asNumber()};
            case TokenType::BANG:
                return PopLObject{right.isTruthy()};
            default:
                break;
        }
        // Unreachable
        return {};
    }

    PopLObject operator()(const BinaryExpr& expr) {
        PopLObject left  = Evaluate(*expr.left);
        PopLObject right = Evaluate(*expr.right);
        switch (expr.op.GetType()) {
            case TokenType::EQUAL_EQUAL:
                return PopLObject{left == right};
            case TokenType::GREATER:
                return PopLObject{left.asNumber() > right.asNumber()};
            case TokenType::LESS:
                return PopLObject{left.asNumber() < right.asNumber()};
            case TokenType::GREATER_EQUAL:
                return PopLObject{left.asNumber() >= right.asNumber()};
            case TokenType::LESS_EQUAL:
                return PopLObject{left.asNumber() <= right.asNumber()};
            case TokenType::PLUS:
                if (left.isNumber() && right.isNumber())
                    return PopLObject{left.asNumber() + right.asNumber()};
                if (left.isString() && right.isString())
                    return PopLObject{left.asString() + right.asString()};
                break;
            case TokenType::MINUS:
                return PopLObject{left.asNumber() - right.asNumber()};
            case TokenType::SLASH:
                return PopLObject{left.asNumber() / right.asNumber()};
            case TokenType::STAR:
                return PopLObject{left.asNumber() * right.asNumber()};
            default:
                break;
        }
        // Unreachable
        return {};
    }

    PopLObject operator()(const TernaryExpr& expr) {
        PopLObject left = Evaluate(*expr.right);
        if (left.isTruthy()) return Evaluate(*expr.mid);
        return Evaluate(*expr.right);
    }
};
};  // namespace popl
