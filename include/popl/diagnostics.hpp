#pragma once

#include <string_view>

class Diagnostics {
   public:
    void Error(unsigned int line, std::string_view message);

   private:
    void Report(unsigned int line, std::string_view where,
                std::string_view message);
};
