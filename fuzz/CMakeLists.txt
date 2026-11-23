add_executable(zen_version_fuzz version_fuzz.c)
target_link_libraries(zen_version_fuzz PRIVATE zen::zen)

target_compile_options(zen_version_fuzz PRIVATE
    -fsanitize=fuzzer,address -fno-omit-frame-pointer)
target_link_options(zen_version_fuzz PRIVATE -fsanitize=fuzzer,address)
