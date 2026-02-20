#pragma once

#include <memory>
#include <optional>
#include <string>

#include "callable.hpp"
#include "popl/environment.hpp"
#include "popl/runtime/popl_instance.hpp"
#include "popl/syntax/ast/expr.hpp"

namespace popl::callable {
class PoplFunction : public PoplCallable {
   public:
    PoplFunction(const FunctionExpr*          declaration,
                 std::shared_ptr<Environment> closure,
                 std::optional<std::string>   name)
        : m_declaration{declaration},
          m_name(std::move(name)),
          m_closure(std::move(closure)) {}

    PopLObject Call(Interpreter&                   interpreter,
                    const std::vector<PopLObject>& args) override;

    std::shared_ptr<PoplFunction> Bind(
        std::shared_ptr<runtime::PoplInstance> instance);
    int GetArity() const override { return m_declaration->params.size(); }
    std::string ToString() const override;

   private:
    const FunctionExpr*          m_declaration;
    std::optional<std::string>   m_name;
    std::shared_ptr<Environment> m_closure;
};
};  // namespace popl::callable
