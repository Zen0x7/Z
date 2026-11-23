/* Single source of truth for the version module: consumed by the C test binary
   via X-macro expansion and by CMake to register one CTest test per case.
   Intentionally has no include guard so it can be included twice. */

ZEN_TEST_CASE(macros_have_the_expected_values)
ZEN_TEST_CASE(get_version_returns_the_macro_values)
ZEN_TEST_CASE(get_version_is_repeatable)
