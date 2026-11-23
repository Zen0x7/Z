# Coverage uses GCC + gcovr; its Cobertura XML is accepted by Codecov.
option(ZLANG_COVERAGE "Instrument the build for gcovr coverage" OFF)

if(ZLANG_COVERAGE)
    if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
        message(FATAL_ERROR "ZLANG_COVERAGE=ON requires GCC (use the llvm-cov toolchain directly for Clang)")
    endif()
    if(NOT ZLANG_SANITIZER STREQUAL "none")
        message(FATAL_ERROR "ZLANG_COVERAGE=ON cannot be combined with ZLANG_SANITIZER=${ZLANG_SANITIZER}")
    endif()

    find_program(GCOVR_EXECUTABLE gcovr REQUIRED)

    add_compile_options(-O0 -g --coverage)
    add_link_options(--coverage)
endif()
