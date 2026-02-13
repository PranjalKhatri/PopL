#pragma once

#include "popl/syntax/ast/expr.hpp"
#include "popl/syntax/ast/stmt.hpp"

namespace popl {

Expr Clone(const Expr& expr);
Stmt Clone(const Stmt& stmt);

}  // namespace popl
