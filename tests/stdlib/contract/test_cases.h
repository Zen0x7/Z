/* Single source of truth for the stdlib contract module: consumed by the C test
   binary via X-macro expansion and by CMake to register one CTest test per case.
   Intentionally has no include guard so it can be included twice. */

ZLANG_TEST_CASE(result_constructors_carry_code_value_and_message)
ZLANG_TEST_CASE(status_messages_and_predicate_are_complete)
ZLANG_TEST_CASE(stream_backends_share_read_and_write_semantics)
