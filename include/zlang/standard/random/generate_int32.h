#ifndef ZLANG_STANDARD_RANDOM_GENERATE_INT32_H
#define ZLANG_STANDARD_RANDOM_GENERATE_INT32_H

#include "zlang/standard/random/state.h"

ZLANG_STATIC_INLINE int32_t zlang_std_random_generate_int32(zlang_runtime_context_t *context,
                                                            int32_t from, int32_t to) {
    if (from > to) {
        zlang_std_random_trap_invalid_argument();
        return 0;
    }
    const uint64_t _range_k = (uint64_t)((int64_t)to - (int64_t)from) + 1ULL;
    const uint32_t _value_k = zlang_std_random_next_u32(context);
    const int64_t _sum_k = (int64_t)from + (int64_t)(_value_k % _range_k);
    return (int32_t)_sum_k;
}

#endif /* ZLANG_STANDARD_RANDOM_GENERATE_INT32_H */
