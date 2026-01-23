#include <fstream>
#include <iostream>
#include <print>
#include <sstream>
#include <stdexcept>
#include <string>

void RunPrompt() {
    std::string line;
    for (;;) {
        std::print("> ");
        if (!std::getline(std::cin, line)) {
            break;
        }
        Run(line);
    }
}

std::string ReadFile(const std::string& path) {
    std::ifstream inFile(path, std::ios::binary);

    // Enable exceptions on fail or badbit
    inFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        std::stringstream ss;
        ss << inFile.rdbuf();  // read entire file
        return ss.str();
    } catch (const std::ios_base::failure& e) {
        throw std::runtime_error("Failed to read file '" + path +
                                 "': " + e.what());
    }
}

void RunFile(const std::string& path) {
    try {
        std::string content = ReadFile(path);
        std::print("File content:\n{}\n", content);
    } catch (const std::runtime_error& e) {
        std::print("Error: {}\n", e.what());
    }
}

int main(int argc, char** argv) {
    if (argc > 2) {
        std::print("Usage: PopL [script]\n");
        return 64;
    } else if (argc == 2) {
        RunFile(argv[1]);
    } else {
        RunPrompt();
    }
}
