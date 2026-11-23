#include "zen/testing/testing.h"

#define ZEN_TEST_CASE(name) static void name(zen_test_context_t *const context);
#include "test_cases.h"
#undef ZEN_TEST_CASE

static void dummy_success(zen_test_context_t *const context) { (void)context; }

static void dummy_failure(zen_test_context_t *const context) { ZEN_CHECK(context, 0); }

static void dummy_mismatch(zen_test_context_t *const context) { ZEN_CHECK_UINT(context, 1U, 2U); }

static void reporting_helpers_count_failures(zen_test_context_t *const context) {
    zen_test_context_t _local = {0U};

    zen_test_report_failure(&_local, __FILE__, 1U, "expected failure");
    ZEN_CHECK_UINT(context, 1U, _local.failure_count_);

    zen_test_report_failure_uint(&_local, __FILE__, 2U, "expected mismatch", 1U, 2U);
    ZEN_CHECK_UINT(context, 2U, _local.failure_count_);
}

static void check_macros_count_failures(zen_test_context_t *const context) {
    zen_test_context_t _passed = {0U};
    ZEN_CHECK(&_passed, 1);
    ZEN_CHECK_UINT(context, 0U, _passed.failure_count_);

    zen_test_context_t _failed_check = {0U};
    dummy_failure(&_failed_check);
    ZEN_CHECK_UINT(context, 1U, _failed_check.failure_count_);

    zen_test_context_t _failed_comparison = {0U};
    dummy_mismatch(&_failed_comparison);
    ZEN_CHECK_UINT(context, 1U, _failed_comparison.failure_count_);
}

static void runner_selects_and_reports_cases(zen_test_context_t *const context) {
    static const zen_test_case_t runner_cases_k[] = {{"dummy_success", dummy_success},
                                                     {"dummy_failure", dummy_failure}};
    static const zen_test_case_t runner_success_k[] = {{"dummy_success", dummy_success}};

    const int _selected_k = zen_test_run_cases("zen.testing", runner_cases_k, 2U, "dummy_success");
    ZEN_CHECK_UINT(context, 0U, _selected_k);

    const int _failed_k = zen_test_run_cases("zen.testing", runner_cases_k, 2U, "dummy_failure");
    ZEN_CHECK_UINT(context, 1U, _failed_k);

    const int _all_k = zen_test_run_cases("zen.testing", runner_success_k, 1U, NULL);
    ZEN_CHECK_UINT(context, 0U, _all_k);

    const int _unknown_k = zen_test_run_cases("zen.testing", runner_success_k, 1U, "missing");
    ZEN_CHECK_UINT(context, 1U, _unknown_k);
}

static void main_parses_the_case_option(zen_test_context_t *const context) {
    static const zen_test_case_t main_cases_k[] = {{"dummy_success", dummy_success}};
    char *const _arguments_k[] = {NULL};
    char _other_option[] = "--other";
    char _case_option[] = "--case";
    char _case_name[] = "dummy_success";
    char *const _other_arguments_k[] = {NULL, _other_option, NULL};
    char *const _case_arguments_k[] = {NULL, _case_option, _case_name};

    const int _no_option_k = zen_test_main(1, _arguments_k, "zen.testing", main_cases_k, 1U);
    ZEN_CHECK_UINT(context, 0U, _no_option_k);

    const int _unknown_option_k =
        zen_test_main(3, _other_arguments_k, "zen.testing", main_cases_k, 1U);
    ZEN_CHECK_UINT(context, 0U, _unknown_option_k);

    const int _case_option_k = zen_test_main(3, _case_arguments_k, "zen.testing", main_cases_k, 1U);
    ZEN_CHECK_UINT(context, 0U, _case_option_k);
}

#define ZEN_TEST_CASE(name) {#name, name},
static const zen_test_case_t cases_k[] = {
#include "test_cases.h"
};
#undef ZEN_TEST_CASE

#ifdef ZEN_SCS_MAIN
int zen_main_impl(int argc, char **argv);
#endif

int main(const int argc, char **const argv) {
    return zen_test_main(argc, argv, "zen.testing", cases_k, sizeof(cases_k) / sizeof(cases_k[0]));
}
