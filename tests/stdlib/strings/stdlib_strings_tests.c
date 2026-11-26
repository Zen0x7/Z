#include "zlang/testing/testing.h"

#define ZLANG_STD_STRINGS_COMPARE
#define ZLANG_STD_STRINGS_CONTAINS
#define ZLANG_STD_STRINGS_STARTS_WITH
#define ZLANG_TRAP_SINK(code) ((void)(code))
#include "zlang/runtime/runtime.h"

#define ZLANG_TEST_CASE(name) static void name(zlang_test_context_t *const context);
#include "test_cases.h"
#undef ZLANG_TEST_CASE

static void strings_helpers_compute(zlang_test_context_t *const context) {
    const zlang_string_t _text_k = {"hello", 5U};

    ZLANG_CHECK(context, zlang_std_strings_compare(_text_k, _text_k).value_ == 0);
    ZLANG_CHECK(context,
                zlang_std_strings_compare(_text_k, (zlang_string_t){"hellp", 5U}).value_ < 0);
    ZLANG_CHECK(context, zlang_std_strings_contains(_text_k, (zlang_string_t){"ell", 3U}).value_);
    ZLANG_CHECK(context, !zlang_std_strings_contains(_text_k, (zlang_string_t){"xyz", 3U}).value_);
    ZLANG_CHECK(context, zlang_std_strings_starts_with(_text_k, (zlang_string_t){"he", 2U}).value_);
    ZLANG_CHECK(context,
                !zlang_std_strings_starts_with(_text_k, (zlang_string_t){"lo", 2U}).value_);
}

#define ZLANG_TEST_CASE(name) {#name, name},
static const zlang_test_case_t cases_k[] = {
#include "test_cases.h"
};
#undef ZLANG_TEST_CASE

int main(const int argc, char **const argv) {
    return zlang_test_main(argc, argv, "zlang.stdlib.strings", cases_k,
                           sizeof(cases_k) / sizeof(cases_k[0]));
}
