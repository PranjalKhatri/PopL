#include "popl/callables/native_functions.hpp"
#include "popl/literal.hpp"

namespace popl::callable {

NativeFunction::NativeFunction(std::string name, int arity, FnType fn)
    : m_name(std::move(name)), m_arity(arity), m_function(std::move(fn)) {}

int NativeFunction::GetArity() const { return m_arity; }

PopLObject NativeFunction::Call(Interpreter&                   interpreter,
                                const std::vector<PopLObject>& args) {
    return m_function(interpreter, args);
}

std::string NativeFunction::ToString() const {
    return "<native fn " + m_name + ">";
}

}  // namespace popl::callable
