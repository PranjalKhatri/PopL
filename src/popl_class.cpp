#include "popl/runtime/popl_class.hpp"

#include "popl/literal.hpp"
#include "popl/runtime/popl_instance.hpp"

namespace popl::runtime {
PopLObject PoplClass::Call(Interpreter&                   interpreter,
                           const std::vector<PopLObject>& args) {
    auto instance = std::make_shared<PoplInstance>(shared_from_this());

    return PopLObject{instance};
}
}  // namespace popl::runtime
