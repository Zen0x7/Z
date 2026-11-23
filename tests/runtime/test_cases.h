/* Single source of truth for the runtime module: consumed by the C test binary
   via X-macro expansion and by CMake to register one CTest test per case.
   Intentionally has no include guard so it can be included twice. */

ZLANG_TEST_CASE(write_copies_the_exact_declared_byte_count)
ZLANG_TEST_CASE(write_handles_empty_and_invalid_arguments)
ZLANG_TEST_CASE(trap_codes_are_fixed)
