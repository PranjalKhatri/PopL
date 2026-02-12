#pragma once

#include "popl/callable.hpp"
#include "popl/syntax/ast/stmt.hpp"

namespace popl::callable {
class PoplFunction : public PoplCallable {
   public:
    explicit PoplFunction(FunctionStmt&& declaration)
        : m_declaration(std::move(declaration)) {}
    PopLObject Call(Interpreter&                   interpreter,
                    const std::vector<PopLObject>& args) override;
    int        GetArity() const override { return m_declaration.params.size(); }

    std::string ToString() const override {
        return std::format("<fn {} (arity:{})>", m_declaration.name.GetLexeme(),
                           GetArity());
    }

   private:
    FunctionStmt m_declaration;
};
};  // namespace popl::callable
