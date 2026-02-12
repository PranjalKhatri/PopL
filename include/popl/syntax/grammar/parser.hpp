#pragma once

#include <initializer_list>
#include <stdexcept>
#include <vector>

#include "popl/diagnostics.hpp"
#include "popl/lexer/token.hpp"
#include "popl/lexer/token_types.hpp"
#include "popl/syntax/ast/expr.hpp"
#include "popl/syntax/ast/stmt.hpp"

namespace popl {
class ParseError : public std::runtime_error {
   public:
    ParseError() : runtime_error("Parser RunTime Error") {}
};
class Parser {
   public:
    Parser(std::vector<Token> tokens) : m_tokens{std::move(tokens)} {}
    std::vector<Stmt> Parse();

   private:
    // Checks current token type against given type
    bool Check(TokenType type) const {
        if (IsAtEnd()) return false;
        return Peek().GetType() == type;
    }
    // Check if END_OF_FILE token is reached
    bool  IsAtEnd() const { return Peek().GetType() == TokenType::END_OF_FILE; }
    Token Peek() const { return m_tokens.at(m_current); }
    Token PeekNext() const {
        if (!IsAtEnd()) return m_tokens.at(m_current + 1);
        assert(false && "PeekNext called at last");
        // TODO: Do Something
        return Peek();
    }
    Token Previous() const { return m_tokens.at(m_current - 1); };

    Token Advance() {
        if (!IsAtEnd()) m_current++;
        return Previous();
    }

    // returns whether the current token type matches any of the given types and
    // advances if it matches
    bool Match(std::initializer_list<TokenType> tokenTypes);

    Token Consume(TokenType type, const std::string& message) {
        if (Check(type)) return Advance();
        throw Error(Peek(), message);
    }

    std::unique_ptr<Expr> MakeExprPtr(Expr&& e) const {
        return std::make_unique<Expr>(std::move(e));
    }
    std::unique_ptr<Stmt> MakeStmtPtr(Stmt&& stmt) const {
        return std::make_unique<Stmt>(std::move(stmt));
    }
    ParseError Error(Token token, const std::string& message) {
        Diagnostics::Error(token, message);
        return ParseError{};
    }
    void Synchronize();

    Stmt Statement();
    Stmt Declaration();
    Stmt FunctionDeclaration(std::string_view kind);
    Stmt VarDeclaration();
    Stmt PrintStatement();
    Stmt ExpressionStatement();
    Stmt IfStatement();
    Stmt WhileStatement();
    Stmt ForStatement();
    Stmt BreakStatement();
    Stmt ContinueStatement();
    Stmt ReturnStatement();
    Stmt AssignmentStatement();

    std::vector<std::unique_ptr<Stmt>> BlockStatement();

    Expr     Expression();
    Expr     Comma();
    Expr     ArgumentExpression();
    Expr     Ternary();
    Expr     OrExpression();
    Expr     AndExpression();
    Expr     Equality();
    Expr     Comparison();
    Expr     Term();
    Expr     Factor();
    Expr     Unary();
    Expr     CallExpression();
    CallExpr FinishCall(Expr callee);
    Expr     Primary();

    template <typename ExprType = BinaryExpr, typename SubParser>
    Expr ParseBinary(SubParser&&                      parseOperand,
                     std::initializer_list<TokenType> ops);

   private:
    /// RAII class for nesting counter
    class DepthGuard {
       public:
        explicit DepthGuard(int& depth) : m_depth(depth) { ++m_depth; }

        ~DepthGuard() { --m_depth; }

       private:
        int& m_depth;
    };

    std::vector<Token> m_tokens{};
    int                m_current{};
    int                m_loop_depth{}, m_function_depth{};
};

template <typename ExprType, typename SubParser>
Expr Parser::ParseBinary(SubParser&&                      parseOperand,
                         std::initializer_list<TokenType> ops) {
    Expr expr = (this->*parseOperand)();

    while (Match(ops)) {
        Token op    = Previous();
        Expr  right = (this->*parseOperand)();

        expr.node.emplace<ExprType>(MakeExprPtr(std::move(expr)), op,
                                    MakeExprPtr(std::move(right)));
    }

    return expr;
}
};  // namespace popl
