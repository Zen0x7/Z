#ifndef ZLANG_STANDARD_RANDOM_GENERATE_INT64_H
#define ZLANG_STANDARD_RANDOM_GENERATE_INT64_H

#include "zlang/standard/random/state.h"
#include "zlang/standard/result/result.h"

ZLANG_STATIC_INLINE zlang_std_result_i64_status_t
zlang_std_random_generate_int64(zlang_runtime_context_t *context, int64_t from, int64_t to) {
    ZLANG_STD_TIMING_BEGIN();
    if (from > to) {
        zlang_std_random_trap_invalid_argument();
        return ZLANG_STD_TIMED_CALL(i64,
                                    zlang_std_result_i64(zlang_std_status_invalid_argument_, 0));
    }
    const bool _full_range_k = from == INT64_MIN && to == INT64_MAX;
    const uint64_t _range_k = _full_range_k ? 0U : ((uint64_t)to - (uint64_t)from) + 1U;
    const uint64_t _value_k = zlang_std_random_next_u64(context);
    if (_full_range_k) {
        if (_value_k <= (uint64_t)INT64_MAX) {
            return ZLANG_STD_TIMED_CALL(
                i64, zlang_std_result_i64(zlang_std_status_success_, (int64_t)_value_k));
        }
        return ZLANG_STD_TIMED_CALL(
            i64,
            zlang_std_result_i64(zlang_std_status_success_,
                                 (int64_t)(_value_k - ((uint64_t)INT64_MAX + 1ULL)) + INT64_MIN));
    }
    const uint64_t _offset_k = _value_k % _range_k;
    const uint64_t _result_k = (uint64_t)from + _offset_k;
    if (_result_k <= (uint64_t)INT64_MAX) {
        return ZLANG_STD_TIMED_CALL(
            i64, zlang_std_result_i64(zlang_std_status_success_, (int64_t)_result_k));
    }
    return ZLANG_STD_TIMED_CALL(
        i64, zlang_std_result_i64(zlang_std_status_success_,
                                  (int64_t)(_result_k - ((uint64_t)INT64_MAX + 1ULL)) + INT64_MIN));
}

#endif /* ZLANG_STANDARD_RANDOM_GENERATE_INT64_H */
