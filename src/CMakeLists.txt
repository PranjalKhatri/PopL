add_executable(PopL
                main.cpp
                diagnostics.cpp
                driver.cpp
                utils.cpp
                lexer.cpp
                parser.cpp
                interpreter.cpp
                popl_function.cpp
                clone_visitor.cpp
                native_function.cpp
                native_registry.cpp
)

target_include_directories(PopL
   PRIVATE
   ${PROJECT_SOURCE_DIR}/include
   ${PROJECT_SOURCE_DIR}/external
)

target_compile_features(PopL
    PRIVATE
        cxx_std_23
)
