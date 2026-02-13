#include "popl/popl_function.hpp"

#include <memory>

#include "popl/environment.hpp"
#include "popl/literal.hpp"
#include "popl/runtime/control_flow.hpp"
#include "popl/syntax/visitors/interpreter.hpp"

namespace popl::callable {
PopLObject PoplFunction::Call(Interpreter&                   interpreter,
                              const std::vector<PopLObject>& args) {
    auto localEnv{std::make_shared<Environment>(m_closure)};
    for (size_t i = 0; i < m_declaration.params.size(); ++i) {
        localEnv->Define(m_declaration.params[i], args[i]);
    }
    try {
        interpreter.ExecuteBlock(m_declaration.body, localEnv);
    } catch (const runtime::control_flow::ReturnSignal& returnValue) {
        return returnValue.value;
    }
    return PopLObject{NilValue{}};
}
};  // namespace popl::callable
