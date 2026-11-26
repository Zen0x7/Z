#ifndef ZLANG_STANDARD_RANDOM_GENERATE_FLOAT32_H
#define ZLANG_STANDARD_RANDOM_GENERATE_FLOAT32_H

#include "zlang/standard/random/state.h"
#include "zlang/standard/result/result.h"

#include <math.h>

ZLANG_STATIC_INLINE zlang_std_result_f32_status_t
zlang_std_random_generate_float32(zlang_runtime_context_t *context, float from, float to) {
    ZLANG_STD_TIMING_BEGIN();
    if (from > to || !isfinite(from) || !isfinite(to)) {
        zlang_std_random_trap_invalid_argument();
        return ZLANG_STD_TIMED_CALL(f32,
                                    zlang_std_result_f32(zlang_std_status_invalid_argument_, 0.0F));
    }
    const double _range_k = (double)to - (double)from;
    if (!isfinite(_range_k)) {
        zlang_std_random_trap_invalid_argument();
        return ZLANG_STD_TIMED_CALL(f32,
                                    zlang_std_result_f32(zlang_std_status_invalid_argument_, 0.0F));
    }
    const double _fraction_k = (double)zlang_std_random_next_u32(context) / 4294967296.0;
    return ZLANG_STD_TIMED_CALL(
        f32, zlang_std_result_f32(zlang_std_status_success_,
                                  (float)((double)from + (_range_k * _fraction_k))));
}

#endif /* ZLANG_STANDARD_RANDOM_GENERATE_FLOAT32_H */
