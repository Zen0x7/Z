#ifndef ZLANG_STANDARD_RANDOM_GENERATE_FLOAT64_H
#define ZLANG_STANDARD_RANDOM_GENERATE_FLOAT64_H

#include "zlang/standard/random/state.h"
#include "zlang/standard/result/result.h"

#include <math.h>

ZLANG_STATIC_INLINE zlang_std_result_f64_status_t
zlang_std_random_generate_float64(zlang_runtime_context_t *context, double from, double to) {
    if (from > to || !isfinite(from) || !isfinite(to)) {
        zlang_std_random_trap_invalid_argument();
        return zlang_std_result_f64(zlang_std_status_invalid_argument_, 0.0);
    }
    const double _range_k = to - from;
    if (!isfinite(_range_k)) {
        zlang_std_random_trap_invalid_argument();
        return zlang_std_result_f64(zlang_std_status_invalid_argument_, 0.0);
    }
    const double _fraction_k = (double)zlang_std_random_next_u64(context) / 18446744073709551616.0;
    return zlang_std_result_f64(zlang_std_status_success_, from + (_range_k * _fraction_k));
}

#endif /* ZLANG_STANDARD_RANDOM_GENERATE_FLOAT64_H */
