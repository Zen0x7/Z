/* Single source of truth for the spec module: consumed by the C test binary
   via X-macro expansion and by CMake to register one CTest test per case.
   Intentionally has no include guard so it can be included twice. */

ZLANG_TEST_CASE(emits_a_scalar_declaration)
ZLANG_TEST_CASE(emits_a_print_call)
ZLANG_TEST_CASE(emits_a_native_foreach_loop)
ZLANG_TEST_CASE(rejects_an_unbounded_sequence_at_the_declaration)
ZLANG_TEST_CASE(reports_lexer_and_parser_diagnostics)
ZLANG_TEST_CASE(skips_comments_and_unescapes_strings)
ZLANG_TEST_CASE(rejects_a_duplicate_declaration)
ZLANG_TEST_CASE(rejects_an_undefined_initializer)
ZLANG_TEST_CASE(rejects_a_type_mismatch)
