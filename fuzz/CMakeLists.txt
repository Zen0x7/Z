function(zlang_add_fuzz_target name)
    add_executable(${name} ${ARGN})
    target_compile_options(${name} PRIVATE
        -fsanitize=fuzzer,address -fno-omit-frame-pointer)
    target_link_options(${name} PRIVATE -fsanitize=fuzzer,address)
endfunction()

zlang_add_fuzz_target(zlang_version_fuzz version_fuzz.c)
target_link_libraries(zlang_version_fuzz PRIVATE zlang::zlang)

zlang_add_fuzz_target(zlang_lexer_fuzz lexer_fuzz.c)
target_link_libraries(zlang_lexer_fuzz PRIVATE zlang::compiler_core)

zlang_add_fuzz_target(zlang_parser_fuzz parser_fuzz.c)
target_link_libraries(zlang_parser_fuzz PRIVATE zlang::compiler_core)

zlang_add_fuzz_target(zlang_emitter_fuzz emitter_fuzz.c)
target_link_libraries(zlang_emitter_fuzz PRIVATE zlang::compiler_core)
