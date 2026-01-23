#include "popl/utils.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace utils {
std::string ReadFile(std::string_view path) {
    std::ifstream inFile(std::string(path), std::ios::binary);

    inFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        std::stringstream ss;
        ss << inFile.rdbuf();
        return ss.str();
    } catch (const std::ios_base::failure& e) {
        throw std::runtime_error("Failed to read file '" + std::string(path) +
                                 "': " + e.what());
    }
}
}  // namespace utils
