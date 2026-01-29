#pragma once

#include <initializer_list>
#include <vector>

#include "popl/lexer/token.hpp"
#include "popl/lexer/token_types.hpp"
#include "popl/syntax/ast/expr.hpp"

namespace popl {

class Parser {
   public:
    Parser(std::vector<Token> tokens) : m_tokens{std::move(tokens)} {}

   private:
    // Checks current token type against given type
    bool Check(TokenType type) const {
        if (IsAtEnd()) return false;
        return Peek().GetType() == type;
    }
    // Check if END_OF_FILE token is reached
    bool  IsAtEnd() const { return Peek().GetType() == TokenType::END_OF_FILE; }
    Token Peek() const { return m_tokens.at(m_current); }
    Token Previous() const { return m_tokens.at(m_current - 1); };

    Token Advance();
    // returns whether the current token type matches any of the given types and
    // advances if it matches
    bool  Match(std::initializer_list<TokenType> tokenTypes);

    std::unique_ptr<Expr> MakeExprPtr(Expr&& e) const {
        return std::make_unique<Expr>(std::move(e));
    }

    Expr Expression();
    Expr Equality();
    Expr Comparison();
    Expr Term();
    Expr Factor();
    Expr Unary();
    Expr Primary();

    template <typename SubParser>
    Expr ParseBinary(SubParser&&                      parseOperand,
                     std::initializer_list<TokenType> ops);

   private:
    std::vector<Token> m_tokens{};
    int                m_current{};
};

template <typename SubParser>
Expr Parser::ParseBinary(SubParser&&                      parseOperand,
                         std::initializer_list<TokenType> ops) {
    Expr expr = (this->*parseOperand)();

    while (Match(ops)) {
        Token op    = Previous();
        Expr  right = (this->*parseOperand)();

        expr.node.emplace<BinaryExpr>(MakeExprPtr(std::move(expr)), op,
                                      MakeExprPtr(std::move(right)));
    }

    return expr;
}
};  // namespace popl
