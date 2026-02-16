#include "popl/runtime/popl_class.hpp"

#include "popl/literal.hpp"
#include "popl/runtime/popl_instance.hpp"

namespace popl {
namespace runtime {
popl::PopLObject PoplClass::Call(popl::Interpreter& interpreter,
                                 const std::vector<popl::PopLObject>& args) {
    auto instance = std::make_shared<PoplInstance>(shared_from_this());

    return popl::PopLObject{instance};
}
}  // namespace runtime
}  // namespace popl
