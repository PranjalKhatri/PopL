#pragma once

#include <string>
#include <string_view>

namespace popl {

class Driver {
   public:
    int Init(int argc, char** argv);

   private:
    void Run(std::string source);
    void RunRepl();
    void RunFile(std::string_view path);
    void PrintUsage() const;
};

}  // namespace popl
