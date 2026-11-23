#ifndef ZEN_TESTING_TESTING_H
#define ZEN_TESTING_TESTING_H

#include "zen/detail/inline.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct zen_test_context_t {
    unsigned int failure_count_;
} zen_test_context_t;

typedef void (*zen_test_fn_t)(zen_test_context_t *const context);

typedef struct zen_test_case_t {
    const char *name_;
    zen_test_fn_t run_;
} zen_test_case_t;

ZEN_STATIC_INLINE void zen_test_report_failure(zen_test_context_t *const context,
                                               const char *const file, const unsigned int line,
                                               const char *const expression) {
    ++context->failure_count_;
    fprintf(stderr, "%s:%u: check failed: %s\n", file, line, expression);
}

ZEN_STATIC_INLINE void zen_test_report_failure_uint(zen_test_context_t *const context,
                                                    const char *const file, const unsigned int line,
                                                    const char *const expression,
                                                    const uintmax_t expected,
                                                    const uintmax_t actual) {
    ++context->failure_count_;
    fprintf(stderr, "%s:%u: check failed: %s (expected %" PRIuMAX ", got %" PRIuMAX ")\n", file,
            line, expression, expected, actual);
}

ZEN_STATIC_INLINE int zen_test_run_cases(const char *const suite,
                                         const zen_test_case_t *const cases,
                                         const size_t case_count,
                                         const char *const requested_case) {
    size_t _selected_count = 0U;
    unsigned int _total_failures = 0U;

    printf("%s: %zu test case(s) available\n", suite, case_count);

    for (size_t _index = 0U; _index < case_count; ++_index) {
        const zen_test_case_t *const _case_k = &cases[_index];
        if (requested_case != NULL && strcmp(_case_k->name_, requested_case) != 0) {
            continue;
        }
        ++_selected_count;

        zen_test_context_t _case_context = {0U};
        printf("  [ RUN      ] %s\n", _case_k->name_);
        _case_k->run_(&_case_context);
        if (_case_context.failure_count_ == 0U) {
            printf("  [       OK ] %s\n", _case_k->name_);
        } else {
            printf("  [  FAILED  ] %s\n", _case_k->name_);
            _total_failures += _case_context.failure_count_;
        }
    }

    if (requested_case != NULL && _selected_count == 0U) {
        fprintf(stderr, "%s: unknown test case: %s\n", suite, requested_case);
        return 1;
    }
    if (_total_failures != 0U) {
        fprintf(stderr, "%s: %u failure(s)\n", suite, _total_failures);
        return 1;
    }

    printf("%s: all selected test cases passed\n", suite);
    return 0;
}

ZEN_STATIC_INLINE int zen_test_main(const int argc, char *const *const argv,
                                    const char *const suite, const zen_test_case_t *const cases,
                                    const size_t case_count) {
    const char *_requested_case = NULL;
    if (argc == 3 && strcmp(argv[1], "--case") == 0) {
        _requested_case = argv[2];
    }

    return zen_test_run_cases(suite, cases, case_count, _requested_case);
}

#define ZEN_CHECK(context, condition)                                                              \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            zen_test_report_failure((context), __FILE__, (unsigned int)__LINE__, #condition);      \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ZEN_CHECK_UINT(context, expected, actual)                                                  \
    do {                                                                                           \
        const uintmax_t _expected_k = (uintmax_t)(expected);                                       \
        const uintmax_t _actual_k = (uintmax_t)(actual);                                           \
        if (_expected_k != _actual_k) {                                                            \
            zen_test_report_failure_uint((context), __FILE__, (unsigned int)__LINE__, #actual,     \
                                         _expected_k, _actual_k);                                  \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#endif /* ZEN_TESTING_TESTING_H */
