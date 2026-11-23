#include "zen/testing/testing.h"
#include "zen/version/version.h"

#define ZEN_TEST_CASE(name) static void name(zen_test_context_t *const context);
#include "test_cases.h"
#undef ZEN_TEST_CASE

static void macros_have_the_expected_values(zen_test_context_t *const context) {
    ZEN_CHECK_UINT(context, 0U, LIBZEN_VERSION_MAJOR);
    ZEN_CHECK_UINT(context, 0U, LIBZEN_VERSION_MINOR);
    ZEN_CHECK_UINT(context, 0U, LIBZEN_VERSION_PATCH);
}

static void get_version_returns_the_macro_values(zen_test_context_t *const context) {
    zen_version_t _version = zen_get_version();

    ZEN_CHECK_UINT(context, LIBZEN_VERSION_MAJOR, _version.major_);
    ZEN_CHECK_UINT(context, LIBZEN_VERSION_MINOR, _version.minor_);
    ZEN_CHECK_UINT(context, LIBZEN_VERSION_PATCH, _version.patch_);
}

static void get_version_is_repeatable(zen_test_context_t *const context) {
    zen_version_t _first = zen_get_version();
    zen_version_t _second = zen_get_version();

    ZEN_CHECK_UINT(context, _first.major_, _second.major_);
    ZEN_CHECK_UINT(context, _first.minor_, _second.minor_);
    ZEN_CHECK_UINT(context, _first.patch_, _second.patch_);
}

#define ZEN_TEST_CASE(name) {#name, name},
static const zen_test_case_t cases_k[] = {
#include "test_cases.h"
};
#undef ZEN_TEST_CASE

int main(const int argc, char **const argv) {
    return zen_test_main(argc, argv, "zen.version", cases_k, sizeof(cases_k) / sizeof(cases_k[0]));
}
