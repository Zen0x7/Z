#ifndef ZLANG_STANDARD_RANDOM_GENERATE_UINT32_H
#define ZLANG_STANDARD_RANDOM_GENERATE_UINT32_H

#include "zlang/standard/random/state.h"
#include "zlang/standard/result/result.h"

ZLANG_STATIC_INLINE zlang_std_result_u32_status_t
zlang_std_random_generate_uint32(zlang_runtime_context_t *context, uint32_t from, uint32_t to) {
    ZLANG_STD_TIMING_BEGIN();
    if (from > to) {
        zlang_std_random_trap_invalid_argument();
        return ZLANG_STD_TIMED_CALL(u32,
                                    zlang_std_result_u32(zlang_std_status_invalid_argument_, 0U));
    }
    const uint64_t _range_k = (uint64_t)to - (uint64_t)from + 1ULL;
    const uint64_t _offset_k = zlang_std_random_next_u64(context) % _range_k;
    return ZLANG_STD_TIMED_CALL(u32, zlang_std_result_u32(zlang_std_status_success_,
                                                          (uint32_t)((uint64_t)from + _offset_k)));
}

#endif /* ZLANG_STANDARD_RANDOM_GENERATE_UINT32_H */
