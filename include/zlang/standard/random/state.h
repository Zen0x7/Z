#ifndef ZLANG_STANDARD_RANDOM_STATE_H
#define ZLANG_STANDARD_RANDOM_STATE_H

#include "zlang/detail/inline.h"
#include "zlang/runtime/context.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef ZLANG_TRAP_SINK
#define ZLANG_TRAP_SINK(code) ((void)(code))
#endif

#ifndef ZLANG_TRAP_INVALID_ARGUMENT
#define ZLANG_TRAP_INVALID_ARGUMENT 4U
#endif

ZLANG_STATIC_INLINE void zlang_std_random_trap_invalid_argument(void) {
    ZLANG_TRAP_SINK(ZLANG_TRAP_INVALID_ARGUMENT);
}

ZLANG_STATIC_INLINE void zlang_std_random_seed(zlang_runtime_context_t *context, uint32_t seed) {
    if (context != NULL) {
        context->random_state_ = seed == 0U ? 0x9E3779B9U : seed;
    }
}

ZLANG_STATIC_INLINE uint32_t zlang_std_random_next_u32(zlang_runtime_context_t *context) {
    uint64_t _value_k = context->random_state_;
    _value_k ^= (_value_k << 13) & UINT32_MAX;
    _value_k ^= _value_k >> 17;
    _value_k ^= (_value_k << 5) & UINT32_MAX;
    _value_k &= UINT32_MAX;
    context->random_state_ = (uint32_t)_value_k;
    return context->random_state_;
}

ZLANG_STATIC_INLINE uint64_t zlang_std_random_next_u64(zlang_runtime_context_t *context) {
    const uint64_t _high_k = (uint64_t)zlang_std_random_next_u32(context) << 32;
    return _high_k | (uint64_t)zlang_std_random_next_u32(context);
}

#endif /* ZLANG_STANDARD_RANDOM_STATE_H */
