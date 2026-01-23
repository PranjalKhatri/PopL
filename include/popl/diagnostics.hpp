#pragma once

#include <string_view>

namespace popl {
class Diagnostics {
   public:
    void Error(unsigned int line, std::string_view message);
    bool HadError() const { return m_had_error; }

   private:
    void Report(unsigned int line, std::string_view where,
                std::string_view message);

   private:
    bool m_had_error;
};
}  // namespace popl
