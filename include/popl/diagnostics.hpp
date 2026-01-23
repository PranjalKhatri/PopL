#pragma once

#include <string_view>

namespace popl {
class Diagnostics {
   public:
    static void Error(unsigned int line, std::string_view message);
    static bool HadError() { return ms_had_error; }
    static void ResetError() { ms_had_error = false; }

   private:
    static void Report(unsigned int line, std::string_view where,
                       std::string_view message);

   private:
    static bool ms_had_error;
};
}  // namespace popl
