add_executable(GenerateAst
                GenerateAst.cpp
)

target_compile_features(GenerateAst
    PRIVATE
        cxx_std_23
)
