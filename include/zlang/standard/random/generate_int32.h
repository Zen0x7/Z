#ifndef ZLANG_STANDARD_RANDOM_GENERATE_INT32_H
#define ZLANG_STANDARD_RANDOM_GENERATE_INT32_H

#include "zlang/standard/random/state.h"
#include "zlang/standard/result/result.h"

ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_random_generate_int32(zlang_runtime_context_t *context, int32_t from, int32_t to) {
    ZLANG_STD_TIMING_BEGIN();
    if (from > to) {
        zlang_std_random_trap_invalid_argument();
        return ZLANG_STD_TIMED_CALL(i32,
                                    zlang_std_result_i32(zlang_std_status_invalid_argument_, 0));
    }
    const uint64_t _range_k = (uint64_t)((int64_t)to - (int64_t)from) + 1ULL;
    const uint32_t _value_k = zlang_std_random_next_u32(context);
    const int64_t _sum_k = (int64_t)from + (int64_t)(_value_k % _range_k);
    return ZLANG_STD_TIMED_CALL(i32,
                                zlang_std_result_i32(zlang_std_status_success_, (int32_t)_sum_k));
}

#endif /* ZLANG_STANDARD_RANDOM_GENERATE_INT32_H */
