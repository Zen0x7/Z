/* Single source of truth for the runtime module: consumed by the C test binary
   via X-macro expansion and by CMake to register one CTest test per case.
   Intentionally has no include guard so it can be included twice. */

ZLANG_TEST_CASE(write_copies_the_exact_declared_byte_count)
ZLANG_TEST_CASE(write_handles_empty_and_invalid_arguments)
ZLANG_TEST_CASE(trap_codes_are_fixed)
ZLANG_TEST_CASE(checked_addition_returns_the_sum)
ZLANG_TEST_CASE(arithmetic_helpers_compute)
ZLANG_TEST_CASE(array_append_writes_and_increments_length)
ZLANG_TEST_CASE(string_concat_copies_bounded_parts)
ZLANG_TEST_CASE(string_concat_handles_self_concatenation)
ZLANG_TEST_CASE(traps_terminate_with_the_declared_code)
