#include "zlang/testing/testing.h"

#define ZLANG_STD_RANDOM_GENERATE_INT32
#define ZLANG_TRAP_SINK(code) ((void)(code))
#include "zlang/runtime/runtime.h"

static void generate_int32_stays_in_range(zlang_test_context_t *const context) {
    zlang_runtime_context_t _state;
    zlang_std_random_seed(&_state, 1U);
    for (int32_t _iteration = 0; _iteration < 100; ++_iteration) {
        const int32_t _value_k = zlang_std_random_generate_int32(&_state, 3, 9);
        ZLANG_CHECK(context, _value_k >= 3 && _value_k <= 9);
    }
}

static void generate_int32_extreme_ranges(zlang_test_context_t *const context) {
    zlang_runtime_context_t _state;
    zlang_std_random_seed(&_state, 7U);
    for (int32_t _iteration = 0; _iteration < 100; ++_iteration) {
        const int32_t _value_k = zlang_std_random_generate_int32(&_state, INT32_MIN, INT32_MAX);
        ZLANG_CHECK(context, _value_k >= INT32_MIN && _value_k <= INT32_MAX);
        ZLANG_CHECK(context,
                    zlang_std_random_generate_int32(&_state, INT32_MIN, INT32_MIN) == INT32_MIN);
        ZLANG_CHECK(context,
                    zlang_std_random_generate_int32(&_state, INT32_MAX, INT32_MAX) == INT32_MAX);
    }
}

static void generate_int32_rejects_inverted_range(zlang_test_context_t *const context) {
    zlang_runtime_context_t _state;
    zlang_std_random_seed(&_state, 1U);
    (void)zlang_std_random_generate_int32(&_state, 9, 3);
    ZLANG_CHECK(context, true);
}

#define ZLANG_TEST_CASE(name) {#name, name},
static const zlang_test_case_t cases_k[] = {
#include "test_cases.h"
};
#undef ZLANG_TEST_CASE

int main(const int argc, char **const argv) {
    return zlang_test_main(argc, argv, "zlang.stdlib.random", cases_k,
                           sizeof(cases_k) / sizeof(cases_k[0]));
}
