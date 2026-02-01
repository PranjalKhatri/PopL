#pragma once

#include <string_view>

#include "popl/lexer/token.hpp"
#include "popl/syntax/Exceptions/run_time_error.hpp"

namespace popl {
class Diagnostics {
   public:
    static void Error(unsigned int line, std::string_view message);
    static void Error(Token token, std::string_view message);
    static void ReportRunTimeError(const RunTimeError& error);
    // mutually exclusive from run_time_error
    static bool HadError() { return ms_had_error; }
    static bool HadRunTimeError() { return ms_had_runtime_error; }
    static void ResetError() { ms_had_error = false; }

   private:
    static void Report(unsigned int line, std::string_view where,
                       std::string_view message);

   private:
    static bool ms_had_error;
    static bool ms_had_runtime_error;
};
}  // namespace popl
