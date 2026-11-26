#include "zlang/testing/testing.h"

#define ZLANG_STD_MATH_ABS_I32
#define ZLANG_STD_MATH_ABS_I64
#define ZLANG_STD_MATH_CLAMP_I32
#define ZLANG_STD_MATH_MIN_I32
#define ZLANG_STD_MATH_MAX_I32
#define ZLANG_STD_MATH_SQRT_F64
#define ZLANG_TRAP_SINK(code) ((void)(code))
#include "zlang/runtime/runtime.h"

#define ZLANG_TEST_CASE(name) static void name(zlang_test_context_t *const context);
#include "test_cases.h"
#undef ZLANG_TEST_CASE

static void math_overloads_compute(zlang_test_context_t *const context) {
    ZLANG_CHECK(context, zlang_std_math_abs_i32(-7).value_ == 7);
    ZLANG_CHECK(context, zlang_std_math_abs_i64(-7LL).value_ == 7LL);
    ZLANG_CHECK(context, zlang_std_math_clamp_i32(9, 1, 5).value_ == 5);
    ZLANG_CHECK(context, zlang_std_math_min_i32(2, 3).value_ == 2);
    ZLANG_CHECK(context, zlang_std_math_max_i32(2, 3).value_ == 3);
    ZLANG_CHECK(context, zlang_std_math_sqrt_f64(9.0).value_ > 2.99);
    ZLANG_CHECK(context, zlang_std_math_sqrt_f64(9.0).value_ < 3.01);
}

#define ZLANG_TEST_CASE(name) {#name, name},
static const zlang_test_case_t cases_k[] = {
#include "test_cases.h"
};
#undef ZLANG_TEST_CASE

int main(const int argc, char **const argv) {
    return zlang_test_main(argc, argv, "zlang.stdlib.math", cases_k,
                           sizeof(cases_k) / sizeof(cases_k[0]));
}
