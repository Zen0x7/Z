# Zero tolerance for warnings: every diagnostic is an error on every toolchain.
if(MSVC)
    add_compile_options(/W4 /WX /permissive- /utf-8)
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Wcast-qual
        -Wformat=2
        -Wundef
        -Wstrict-prototypes
        -Wmissing-prototypes
        -Wold-style-definition
        -Wwrite-strings
        -Wvla
        -Wpointer-arith
        -Wswitch-default
        -Wdouble-promotion)

    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        add_compile_options(
            -Wlogical-op
            -Wduplicated-cond
            -Wjump-misses-init
            -Wformat-overflow=2
            -Wformat-truncation=2)
    endif()
else()
    message(WARNING "No strict warning preset for ${CMAKE_C_COMPILER_ID}")
endif()
