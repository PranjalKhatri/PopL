#pragma once

#include <string>
#include <string_view>

#include "popl/syntax/visitors/interpreter.hpp"

namespace popl {

class Driver {
   public:
    int Init(int argc, char** argv);

   private:
    void Run(std::string source);
    int  RunRepl();
    int  RunFile(std::string_view path);
    void PrintUsage() const;

   private:
    static Interpreter interpreter;
};

}  // namespace popl
