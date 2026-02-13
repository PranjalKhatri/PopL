#pragma once

#include <memory>

#include "popl/callable.hpp"
#include "popl/environment.hpp"
#include "popl/syntax/ast/stmt.hpp"
#include "popl/syntax/visitors/clone_visitor.hpp"

namespace popl::callable {
class PoplFunction : public PoplCallable {
   public:
    PoplFunction(const FunctionStmt&          declaration,
                 std::shared_ptr<Environment> closure)
        : m_declaration{declaration.name, declaration.params, {}},
          m_closure(std::move(closure)) {
        for (const auto& stmt : declaration.body) {
            m_declaration.body.push_back(std::make_unique<Stmt>(Clone(*stmt)));
        }
    }

    PopLObject Call(Interpreter&                   interpreter,
                    const std::vector<PopLObject>& args) override;
    int        GetArity() const override { return m_declaration.params.size(); }

    std::string ToString() const override {
        return std::format("<fn {} (arity:{})>", m_declaration.name.GetLexeme(),
                           GetArity());
    }

   private:
    FunctionStmt                 m_declaration;
    std::shared_ptr<Environment> m_closure;
};
};  // namespace popl::callable
