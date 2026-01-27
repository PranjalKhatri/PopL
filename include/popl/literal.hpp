#pragma once

#include <string>
#include <variant>

namespace popl {
using Literal = std::variant<std::monostate, double, std::string, bool>;
}
