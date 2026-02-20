#include "popl/callables/popl_function.hpp"

#include <memory>

#include "popl/environment.hpp"
#include "popl/lexer/token_types.hpp"
#include "popl/literal.hpp"
#include "popl/runtime/control_flow.hpp"
#include "popl/syntax/visitors/interpreter.hpp"

namespace popl::callable {
PopLObject PoplFunction::Call(Interpreter&                   interpreter,
                              const std::vector<PopLObject>& args) {
    auto localEnv{std::make_shared<Environment>(m_closure)};
    for (size_t i = 0; i < m_declaration->params.size(); ++i) {
        localEnv->Define(m_declaration->params[i], args[i]);
    }
    if (m_isInitializer) {
        return m_closure->GetAt(
            0, Token{TokenType::THIS, "this", PopLObject{NilValue{}}, 0});
    }
    try {
        interpreter.ExecuteBlock(m_declaration->body, localEnv);
    } catch (const runtime::control_flow::ReturnSignal& returnValue) {
        return returnValue.value;
    }
    return PopLObject{NilValue{}};
}

std::shared_ptr<PoplFunction> PoplFunction::Bind(
    std::shared_ptr<runtime::PoplInstance> instance) {
    auto environment = std::make_shared<Environment>(m_closure);
    environment->Define("this", PopLObject{instance});

    return std::make_shared<PoplFunction>(m_declaration, environment, m_name,
                                          m_isInitializer);
}
std::string PoplFunction::ToString() const {
    if (m_name) {
        return std::format("<fn {} (arity:{})>", *m_name, GetArity());
    }
    return std::format("<fn anonymous (arity:{})>", GetArity());
}
};  // namespace popl::callable
