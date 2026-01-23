#include "popl/driver.hpp"

#include <iostream>
#include <print>
#include <string>
#include <string_view>

#include "popl/utils.hpp"

namespace popl {

int Driver::Init(int argc, char** argv) {
    if (argc > 2) {
        PrintUsage();
        return 64;
    } else if (argc == 2) {
        RunFile(argv[1]);
    } else {
        RunRepl();
    }
    return 0;
}

void Driver::PrintUsage() const { std::print("Usage: popl [script]"); }

void Driver::RunRepl() {
    std::string line;
    for (;;) {
        std::print("> ");
        if (!std::getline(std::cin, line)) {
            break;
        }
        Run(line);
    }
}

void Driver::RunFile(std::string_view path) {
    try {
        std::string content = utils::ReadFile(path);
        std::print("File content:\n{}\n", content);
    } catch (const std::runtime_error& e) {
        std::print("Error: {}\n", e.what());
    }
}
};  // namespace popl
