#pragma once

#include <memory>
#include <optional>
#include <string>

#include "popl/callable.hpp"
#include "popl/environment.hpp"
#include "popl/syntax/ast/expr.hpp"
#include "popl/syntax/visitors/clone_visitor.hpp"

namespace popl::callable {
class PoplFunction : public PoplCallable {
   public:
    PoplFunction(const FunctionExpr&          declaration,
                 std::shared_ptr<Environment> closure,
                 std::optional<std::string>   name)
        : m_declaration{declaration.params, {}},
          m_name(std::move(name)),
          m_closure(std::move(closure)) {
        for (const auto& stmt : declaration.body) {
            m_declaration.body.push_back(std::make_unique<Stmt>(Clone(*stmt)));
        }
    }

    PopLObject Call(Interpreter&                   interpreter,
                    const std::vector<PopLObject>& args) override;

    int GetArity() const override { return m_declaration.params.size(); }
    std::string ToString() const override;

   private:
    FunctionExpr                 m_declaration;
    std::optional<std::string>   m_name;
    std::shared_ptr<Environment> m_closure;
};
};  // namespace popl::callable
