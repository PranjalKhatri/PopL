add_executable(PopL
                main.cpp
                lexer.cpp
                diagnostics.cpp
)

target_include_directories(PopL
   PRIVATE
   ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(PopL
    PRIVATE
        cxx_std_23
)
