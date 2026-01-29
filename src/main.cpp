#include <iostream>
#include <memory>
#include <popl/driver.hpp>

#include "popl/lexer/token.hpp"
#include "popl/lexer/token_types.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/ast/expr.hpp"
#include "popl/syntax/visitors/ast_printer.hpp"

using namespace popl;
int main(int argc, char** argv) {
    Expr       expression{BinaryExpr{
        std::make_unique<Expr>(
            UnaryExpr{Token{TokenType::MINUS, "-", {}, 1},
                      std::make_unique<Expr>(LiteralExpr{PopLObject{123.0}})}),

        Token{TokenType::STAR, "*", {}, 1},

        std::make_unique<Expr>(GroupingExpr{
            std::make_unique<Expr>(LiteralExpr{PopLObject{45.67}})})}};
    AstPrinter printer;
    std::cout << printer.Print(expression);
    // popl::Driver driver{};
    // driver.Init(argc, argv);
}
