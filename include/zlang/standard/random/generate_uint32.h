#ifndef ZLANG_STANDARD_RANDOM_GENERATE_UINT32_H
#define ZLANG_STANDARD_RANDOM_GENERATE_UINT32_H

#include "zlang/standard/random/state.h"

ZLANG_STATIC_INLINE uint32_t zlang_std_random_generate_uint32(zlang_runtime_context_t *context,
                                                              uint32_t from, uint32_t to) {
    if (from > to) {
        zlang_std_random_trap_invalid_argument();
        return (uint32_t)0;
    }
    const uint64_t _range_k = (uint64_t)to - (uint64_t)from + 1ULL;
    const uint64_t _offset_k = zlang_std_random_next_u64(context) % _range_k;
    return (uint32_t)((uint64_t)from + _offset_k);
}

#endif /* ZLANG_STANDARD_RANDOM_GENERATE_UINT32_H */
