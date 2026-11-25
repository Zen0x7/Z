#ifndef ZLANG_STANDARD_RANDOM_GENERATE_FLOAT32_H
#define ZLANG_STANDARD_RANDOM_GENERATE_FLOAT32_H

#include "zlang/standard/random/state.h"

#include <math.h>

ZLANG_STATIC_INLINE float zlang_std_random_generate_float32(zlang_runtime_context_t *context,
                                                            float from, float to) {
    if (from > to || !isfinite(from) || !isfinite(to)) {
        zlang_std_random_trap_invalid_argument();
        return 0.0F;
    }
    const double _range_k = (double)to - (double)from;
    if (!isfinite(_range_k)) {
        zlang_std_random_trap_invalid_argument();
        return 0.0F;
    }
    const double _fraction_k = (double)zlang_std_random_next_u32(context) / 4294967296.0;
    return (float)((double)from + (_range_k * _fraction_k));
}

#endif /* ZLANG_STANDARD_RANDOM_GENERATE_FLOAT32_H */
