#include "zlang/testing/testing.h"
#include "zlang/version/version.h"

#define ZLANG_TEST_CASE(name) static void name(zlang_test_context_t *const context);
#include "test_cases.h"
#undef ZLANG_TEST_CASE

static void macros_have_the_expected_values(zlang_test_context_t *const context) {
    ZLANG_CHECK_UINT(context, ZLANG_EXPECTED_VERSION_MAJOR, LIBZLANG_VERSION_MAJOR);
    ZLANG_CHECK_UINT(context, ZLANG_EXPECTED_VERSION_MINOR, LIBZLANG_VERSION_MINOR);
    ZLANG_CHECK_UINT(context, ZLANG_EXPECTED_VERSION_PATCH, LIBZLANG_VERSION_PATCH);
}

static void get_version_returns_the_macro_values(zlang_test_context_t *const context) {
    zlang_version_t _version = zlang_get_version();

    ZLANG_CHECK_UINT(context, LIBZLANG_VERSION_MAJOR, _version.major_);
    ZLANG_CHECK_UINT(context, LIBZLANG_VERSION_MINOR, _version.minor_);
    ZLANG_CHECK_UINT(context, LIBZLANG_VERSION_PATCH, _version.patch_);
}

static void get_version_is_repeatable(zlang_test_context_t *const context) {
    zlang_version_t _first = zlang_get_version();
    zlang_version_t _second = zlang_get_version();

    ZLANG_CHECK_UINT(context, _first.major_, _second.major_);
    ZLANG_CHECK_UINT(context, _first.minor_, _second.minor_);
    ZLANG_CHECK_UINT(context, _first.patch_, _second.patch_);
}

#define ZLANG_TEST_CASE(name) {#name, name},
static const zlang_test_case_t cases_k[] = {
#include "test_cases.h"
};
#undef ZLANG_TEST_CASE

int main(const int argc, char **const argv) {
    return zlang_test_main(argc, argv, "zlang.version", cases_k,
                           sizeof(cases_k) / sizeof(cases_k[0]));
}
