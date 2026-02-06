#include "popl/driver.hpp"

#include <cstdlib>
#include <iostream>
#include <print>
#include <string>
#include <string_view>

#include "popl/diagnostics.hpp"
#include "popl/lexer/lexer.hpp"
#include "popl/syntax/grammar/parser.hpp"
#include "popl/utils.hpp"

namespace popl {
Interpreter Driver::interpreter{};
int         Driver::Init(int argc, char** argv) {
    if (argc > 2) {
        PrintUsage();
        return 64;
    } else if (argc == 2) {
        return RunFile(argv[1]);
    } else {
        return RunRepl();
    }
}

void Driver::PrintUsage() const { std::print("Usage: popl [script]"); }

int Driver::RunRepl() {
    std::string line;
    for (;;) {
        std::print("> ");
        if (!std::getline(std::cin, line)) {
            break;
        }
        Run(line);
        Diagnostics::ResetError();
    }
    return 0;
}

int Driver::RunFile(std::string_view path) {
    try {
        Run(utils::ReadFile(path));
        if (Diagnostics::HadError()) std::exit(65);
        if (Diagnostics::HadRunTimeError()) std::exit(70);
    } catch (const std::runtime_error& e) {
        std::print("Error: {}\n", e.what());
    }
    return 74;
}

void Driver::Run(std::string source) {
    Lexer  lexer{std::move(source)};
    auto   tokens{lexer.ScanTokens()};
    Parser parser{tokens};
    auto   statements = parser.Parse();
    if (Diagnostics::HadError()) return;
    interpreter.Interpret(statements);
    // std::println("{}", AstPrinter{}.Print(expression.value()));
}
};  // namespace popl
