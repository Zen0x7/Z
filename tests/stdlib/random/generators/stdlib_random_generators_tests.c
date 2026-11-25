#include "zlang/testing/testing.h"

#define ZLANG_STD_RANDOM_GENERATE_UINT32
#define ZLANG_STD_RANDOM_GENERATE_INT64
#define ZLANG_STD_RANDOM_GENERATE_UINT64
#define ZLANG_STD_RANDOM_GENERATE_FLOAT32
#define ZLANG_STD_RANDOM_GENERATE_FLOAT64
#define ZLANG_TRAP_SINK(code) ((void)(code))
#include "zlang/runtime/runtime.h"

#include <stdint.h>

#define ZLANG_TEST_CASE(name) static void name(zlang_test_context_t *const context);
#include "test_cases.h"
#undef ZLANG_TEST_CASE

static void generators_stay_in_range(zlang_test_context_t *const context) {
    zlang_runtime_context_t _context;

    zlang_runtime_context_init(&_context);
    for (int32_t _iteration = 0; _iteration < 32; ++_iteration) {
        const uint32_t _u32_k = zlang_std_random_generate_uint32(&_context, 3U, 9U);
        const float _f32_k = zlang_std_random_generate_float32(&_context, 1.0F, 2.0F);
        const double _f64_k = zlang_std_random_generate_float64(&_context, 1.0, 2.0);
        ZLANG_CHECK(context, _u32_k >= 3U && _u32_k <= 9U);
        ZLANG_CHECK(context, _f32_k >= 1.0F && _f32_k < 2.0F);
        ZLANG_CHECK(context, _f64_k >= 1.0 && _f64_k < 2.0);
    }
}

static void generators_cover_extreme_ranges(zlang_test_context_t *const context) {
    zlang_runtime_context_t _context;

    zlang_runtime_context_init(&_context);
    for (int32_t _iteration = 0; _iteration < 32; ++_iteration) {
        const int64_t _i64_k = zlang_std_random_generate_int64(&_context, INT64_MIN, INT64_MAX);
        const uint64_t _u64_k = zlang_std_random_generate_uint64(&_context, 0U, UINT64_MAX);
        ZLANG_CHECK(context, _i64_k >= INT64_MIN && _i64_k <= INT64_MAX);
        ZLANG_CHECK(context, _u64_k <= UINT64_MAX);
    }
}

static void generators_reject_inverted_ranges(zlang_test_context_t *const context) {
    zlang_runtime_context_t _context;

    zlang_runtime_context_init(&_context);
    (void)zlang_std_random_generate_uint32(&_context, 9U, 3U);
    (void)zlang_std_random_generate_int64(&_context, 9LL, 3LL);
    (void)zlang_std_random_generate_uint64(&_context, 9ULL, 3ULL);
    (void)zlang_std_random_generate_float32(&_context, 9.0F, 3.0F);
    (void)zlang_std_random_generate_float64(&_context, 9.0, 3.0);
    ZLANG_CHECK(context, true);
}

#define ZLANG_TEST_CASE(name) {#name, name},
static const zlang_test_case_t cases_k[] = {
#include "test_cases.h"
};
#undef ZLANG_TEST_CASE

int main(const int argc, char **const argv) {
    return zlang_test_main(argc, argv, "zlang.stdlib.random.generators", cases_k,
                           sizeof(cases_k) / sizeof(cases_k[0]));
}
