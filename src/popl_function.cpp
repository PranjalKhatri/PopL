#include "popl/popl_function.hpp"

#include "popl/environment.hpp"
#include "popl/literal.hpp"
#include "popl/syntax/visitors/interpreter.hpp"

namespace popl::callable {
PopLObject PoplFunction::Call(Interpreter&                   interpreter,
                              const std::vector<PopLObject>& args) {
    Environment localEnv{interpreter.GetGlobalEnvironment()};
    for (size_t i = 0; i < m_declaration.params.size(); ++i) {
        localEnv.Define(m_declaration.params[i], args[i]);
    }
    try {
        interpreter.ExecuteBlock(m_declaration.body, &localEnv);
    } catch (const runtime::control_flow::ReturnSignal& returnValue) {
        return returnValue.value;
    }
    return PopLObject{NilValue()};
}
};  // namespace popl::callable
