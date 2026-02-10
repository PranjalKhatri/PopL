cmake_minimum_required(VERSION 4.2.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(PopL
    VERSION 0.1.0
    DESCRIPTION "A small Interpreter"
    LANGUAGES CXX
)

add_subdirectory(src)
add_subdirectory(tools)
