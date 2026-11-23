/* Single source of truth for the version module: consumed by the C test binary
   via X-macro expansion and by CMake to register one CTest test per case.
   Intentionally has no include guard so it can be included twice. */

ZLANG_TEST_CASE(macros_have_the_expected_values)
ZLANG_TEST_CASE(get_version_returns_the_macro_values)
ZLANG_TEST_CASE(get_version_is_repeatable)
