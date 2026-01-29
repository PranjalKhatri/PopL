add_executable(PopL
                main.cpp
                diagnostics.cpp
                driver.cpp
                utils.cpp
                lexer.cpp
                parser.cpp
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
