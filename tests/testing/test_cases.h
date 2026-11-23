/* Single source of truth for the testing module: consumed by the C test binary
   via X-macro expansion and by CMake to register one CTest test per case.
   Intentionally has no include guard so it can be included twice. */

ZLANG_TEST_CASE(reporting_helpers_count_failures)
ZLANG_TEST_CASE(check_macros_count_failures)
ZLANG_TEST_CASE(runner_selects_and_reports_cases)
ZLANG_TEST_CASE(main_parses_the_case_option)
