#include "zlang/testing/testing.h"

#define ZLANG_TEST_CASE(name) static void name(zlang_test_context_t *const context);
#include "test_cases.h"
#undef ZLANG_TEST_CASE

static void dummy_success(zlang_test_context_t *const context) { (void)context; }

static void dummy_failure(zlang_test_context_t *const context) { ZLANG_CHECK(context, 0); }

static void dummy_mismatch(zlang_test_context_t *const context) {
    ZLANG_CHECK_UINT(context, 1U, 2U);
}

static void reporting_helpers_count_failures(zlang_test_context_t *const context) {
    zlang_test_context_t _local = {0U};

    zlang_test_report_failure(&_local, __FILE__, 1U, "expected failure");
    ZLANG_CHECK_UINT(context, 1U, _local.failure_count_);

    zlang_test_report_failure_uint(&_local, __FILE__, 2U, "expected mismatch", 1U, 2U);
    ZLANG_CHECK_UINT(context, 2U, _local.failure_count_);
}

static void check_macros_count_failures(zlang_test_context_t *const context) {
    zlang_test_context_t _passed = {0U};
    ZLANG_CHECK(&_passed, 1);
    ZLANG_CHECK_UINT(context, 0U, _passed.failure_count_);

    zlang_test_context_t _failed_check = {0U};
    dummy_failure(&_failed_check);
    ZLANG_CHECK_UINT(context, 1U, _failed_check.failure_count_);

    zlang_test_context_t _failed_comparison = {0U};
    dummy_mismatch(&_failed_comparison);
    ZLANG_CHECK_UINT(context, 1U, _failed_comparison.failure_count_);
}

static void runner_selects_and_reports_cases(zlang_test_context_t *const context) {
    static const zlang_test_case_t runner_cases_k[] = {{"dummy_success", dummy_success},
                                                       {"dummy_failure", dummy_failure}};
    static const zlang_test_case_t runner_success_k[] = {{"dummy_success", dummy_success}};

    const int _selected_k =
        zlang_test_run_cases("zlang.testing", runner_cases_k, 2U, "dummy_success");
    ZLANG_CHECK_UINT(context, 0U, _selected_k);

    const int _failed_k =
        zlang_test_run_cases("zlang.testing", runner_cases_k, 2U, "dummy_failure");
    ZLANG_CHECK_UINT(context, 1U, _failed_k);

    const int _all_k = zlang_test_run_cases("zlang.testing", runner_success_k, 1U, NULL);
    ZLANG_CHECK_UINT(context, 0U, _all_k);

    const int _unknown_k = zlang_test_run_cases("zlang.testing", runner_success_k, 1U, "missing");
    ZLANG_CHECK_UINT(context, 1U, _unknown_k);
}

static void main_parses_the_case_option(zlang_test_context_t *const context) {
    static const zlang_test_case_t main_cases_k[] = {{"dummy_success", dummy_success}};
    char *const _arguments_k[] = {NULL};
    char _other_option[] = "--other";
    char _case_option[] = "--case";
    char _case_name[] = "dummy_success";
    char *const _other_arguments_k[] = {NULL, _other_option, NULL};
    char *const _case_arguments_k[] = {NULL, _case_option, _case_name};

    const int _no_option_k = zlang_test_main(1, _arguments_k, "zlang.testing", main_cases_k, 1U);
    ZLANG_CHECK_UINT(context, 0U, _no_option_k);

    const int _unknown_option_k =
        zlang_test_main(3, _other_arguments_k, "zlang.testing", main_cases_k, 1U);
    ZLANG_CHECK_UINT(context, 0U, _unknown_option_k);

    const int _case_option_k =
        zlang_test_main(3, _case_arguments_k, "zlang.testing", main_cases_k, 1U);
    ZLANG_CHECK_UINT(context, 0U, _case_option_k);
}

#define ZLANG_TEST_CASE(name) {#name, name},
static const zlang_test_case_t cases_k[] = {
#include "test_cases.h"
};
#undef ZLANG_TEST_CASE

int main(const int argc, char **const argv) {
    return zlang_test_main(argc, argv, "zlang.testing", cases_k,
                           sizeof(cases_k) / sizeof(cases_k[0]));
}
