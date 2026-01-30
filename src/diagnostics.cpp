#include "popl/diagnostics.hpp"

#include <cstdio>
#include <print>
#include <string_view>

namespace popl {

bool Diagnostics::ms_had_error = false;

void Diagnostics::Error(unsigned int line, std::string_view message) {
    Report(line, "", message);
}

void Diagnostics::Error(Token token, std::string_view message) {
    if (token.GetType() == TokenType::END_OF_FILE)
        Report(token.GetLine(), " at end", message);
    else
        Report(token.GetLine(), " at '" + token.GetLexeme() + "'", message);
}
void Diagnostics::Report(unsigned int line, std::string_view where,
                         std::string_view message) {
    ms_had_error = true;
    std::println(stderr, "[line {}] Error {} : {}", line, where, message);
}

};  // namespace popl
