#include "popl/driver.hpp"

#include <iostream>
#include <print>
#include <string>
#include <string_view>

#include "popl/diagnostics.hpp"
#include "popl/utils.hpp"

namespace popl {

int Driver::Init(int argc, char** argv) {
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
        if (Diagnostics::HadError()) {
            return 65;
        }
    } catch (const std::runtime_error& e) {
        std::print("Error: {}\n", e.what());
    }
    return 74;
}
};  // namespace popl
