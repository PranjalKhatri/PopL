#pragma once

#include <string>
#include <string_view>

#include "popl/syntax/visitors/interpreter.hpp"

namespace popl {

class Driver {
   public:
    int Init(int argc, char** argv);

   private:
    void Run(std::string source, bool replMode = false);
    void Run(const std::vector<Token>& tokens, bool replMode);
    int  RunRepl();
    int  RunFile(std::string_view path);
    void PrintUsage() const;
    bool IsStatementComplete(const std::vector<Token>& tokens) const;

   private:
    static Interpreter interpreter;
};

}  // namespace popl
