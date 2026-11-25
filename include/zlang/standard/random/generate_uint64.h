#ifndef ZLANG_STANDARD_RANDOM_GENERATE_UINT64_H
#define ZLANG_STANDARD_RANDOM_GENERATE_UINT64_H

#include "zlang/standard/random/state.h"

ZLANG_STATIC_INLINE uint64_t zlang_std_random_generate_uint64(zlang_runtime_context_t *context,
                                                              uint64_t from, uint64_t to) {
    if (from > to) {
        zlang_std_random_trap_invalid_argument();
        return 0U;
    }
    const uint64_t _value_k = zlang_std_random_next_u64(context);
    const uint64_t _span_k = to - from;
    if (_span_k == UINT64_MAX) {
        return from + _value_k;
    }
    const uint64_t _offset_k = _value_k % (_span_k + 1ULL);
    return from + _offset_k;
}

#endif /* ZLANG_STANDARD_RANDOM_GENERATE_UINT64_H */
