#include "popl/diagnostics.hpp"

#include <cstdio>
#include <print>
#include <string_view>

namespace popl {

void Diagnostics::Error(unsigned int line, std::string_view message) {
    Report(line, "", message);
}

void Diagnostics::Report(unsigned int line, std::string_view where,
                         std::string_view message) {
    m_had_error = true;
    std::print(stderr, "[line {}] Error {} : {}", line, where, message);
}

};  // namespace popl
