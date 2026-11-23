add_library(zlang_compiler_core STATIC
    src/diagnostics.c
    src/emitter.c
    src/lexer.c
    src/parser.c
    src/planner.c
    src/semantics.c)

add_library(zlang::compiler_core ALIAS zlang_compiler_core)

target_include_directories(zlang_compiler_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_compile_features(zlang_compiler_core PUBLIC c_std_99)
