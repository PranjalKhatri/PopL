#pragma once
#include "popl/literal.hpp"

namespace popl::runtime::control_flow {
/// Act as signals through exceptions.
/// No information is needed. Internal only,
/// not displayed to user.
struct BreakSignal {};
struct ContinueSignal {};

/// ReturnSignal carries a value for `return`.
struct ReturnSignal {
    popl::PopLObject value;
};

};  // namespace popl::runtime::control_flow
