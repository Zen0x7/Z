add_executable(zlang_version_fuzz version_fuzz.c)
target_link_libraries(zlang_version_fuzz PRIVATE zlang::zlang)

target_compile_options(zlang_version_fuzz PRIVATE
    -fsanitize=fuzzer,address -fno-omit-frame-pointer)
target_link_options(zlang_version_fuzz PRIVATE -fsanitize=fuzzer,address)
