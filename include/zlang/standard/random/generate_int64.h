#ifndef ZLANG_STANDARD_RANDOM_GENERATE_INT64_H
#define ZLANG_STANDARD_RANDOM_GENERATE_INT64_H

#include "zlang/standard/random/state.h"

ZLANG_STATIC_INLINE int64_t zlang_std_random_generate_int64(zlang_runtime_context_t *context,
                                                            int64_t from, int64_t to) {
    if (from > to) {
        zlang_std_random_trap_invalid_argument();
        return (int64_t)0;
    }
    const bool _full_range_k = from == INT64_MIN && to == INT64_MAX;
    const uint64_t _range_k = _full_range_k ? 0U : ((uint64_t)to - (uint64_t)from) + 1U;
    const uint64_t _value_k = zlang_std_random_next_u64(context);
    if (_full_range_k) {
        if (_value_k <= (uint64_t)INT64_MAX) {
            return (int64_t)_value_k;
        }
        return (int64_t)(_value_k - ((uint64_t)INT64_MAX + 1ULL)) + INT64_MIN;
    }
    const uint64_t _offset_k = _value_k % _range_k;
    const uint64_t _result_k = (uint64_t)from + _offset_k;
    if (_result_k <= (uint64_t)INT64_MAX) {
        return (int64_t)_result_k;
    }
    return (int64_t)(_result_k - ((uint64_t)INT64_MAX + 1ULL)) + INT64_MIN;
}

#endif /* ZLANG_STANDARD_RANDOM_GENERATE_INT64_H */
