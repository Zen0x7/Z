#ifndef ZLANG_STANDARD_RANDOM_GENERATE_INT32_H
#define ZLANG_STANDARD_RANDOM_GENERATE_INT32_H

#include "zlang/detail/inline.h"

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

typedef struct zlang_std_random_state_t {
    uint32_t state_;
} zlang_std_random_state_t;

ZLANG_STATIC_INLINE void zlang_std_random_seed(zlang_std_random_state_t *state, uint32_t seed) {
    state->state_ = seed == 0U ? 0x9E3779B9U : seed;
}

ZLANG_STATIC_INLINE uint32_t zlang_std_random_next_u32(zlang_std_random_state_t *state) {
    uint32_t _value_k = state->state_;
    _value_k ^= _value_k << 13;
    _value_k ^= _value_k >> 17;
    _value_k ^= _value_k << 5;
    state->state_ = _value_k;
    return _value_k;
}

ZLANG_STATIC_INLINE int32_t zlang_std_random_generate_int32(zlang_std_random_state_t *state,
                                                            int32_t from, int32_t to) {
    if (from > to) {
        zlang_std_random_trap_invalid_argument();
        return 0;
    }
    const uint64_t _range_k = (uint64_t)((int64_t)to - (int64_t)from) + 1ULL;
    const uint32_t _value_k = zlang_std_random_next_u32(state);
    return from + (int32_t)(_value_k % _range_k);
}

#endif /* ZLANG_STANDARD_RANDOM_GENERATE_INT32_H */
