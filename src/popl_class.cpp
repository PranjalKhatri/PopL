#include "popl/runtime/popl_class.hpp"

#include <optional>

#include "popl/literal.hpp"
#include "popl/runtime/popl_instance.hpp"

namespace popl {
namespace runtime {
popl::PopLObject PoplClass::Call(popl::Interpreter& interpreter,
                                 const std::vector<popl::PopLObject>& args) {
    auto instance = std::make_shared<PoplInstance>(shared_from_this());

    auto initializer = GetMethod("init");
    if (initializer) {
        initializer.value()->Bind(instance)->Call(interpreter, args);
    }
    return popl::PopLObject{instance};
}

int PoplClass::GetArity() const {
    auto initializer{GetMethod("init")};
    if (!initializer) return 0;
    return initializer.value()->GetArity();
}
std::optional<std::shared_ptr<popl::callable::PoplFunction>>
PoplClass::GetMethod(const std::string& name) const {
    auto it = m_methods.find(name);
    if (it == m_methods.end()) return std::nullopt;
    return it->second;
}
}  // namespace runtime
}  // namespace popl
