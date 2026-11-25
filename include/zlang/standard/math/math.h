#ifndef ZLANG_STANDARD_MATH_MATH_H
#define ZLANG_STANDARD_MATH_MATH_H

#include "zlang/detail/inline.h"

#include <math.h>
#include <stdint.h>

#if defined(ZLANG_STD_MATH_ABS_I32) || defined(ZLANG_STD_MATH_ABS_I64) ||                          \
    defined(ZLANG_STD_MATH_ABS_F32) || defined(ZLANG_STD_MATH_ABS_F64)
#define ZLANG_STD_MATH_ABS_ACTIVE
#endif

#if defined(ZLANG_STD_MATH_CLAMP_I32) || defined(ZLANG_STD_MATH_CLAMP_U32) ||                      \
    defined(ZLANG_STD_MATH_CLAMP_I64) || defined(ZLANG_STD_MATH_CLAMP_U64) ||                      \
    defined(ZLANG_STD_MATH_CLAMP_F32) || defined(ZLANG_STD_MATH_CLAMP_F64)
#define ZLANG_STD_MATH_CLAMP_ACTIVE
#endif

#if defined(ZLANG_STD_MATH_MIN_I32) || defined(ZLANG_STD_MATH_MIN_U32) ||                          \
    defined(ZLANG_STD_MATH_MIN_I64) || defined(ZLANG_STD_MATH_MIN_U64) ||                          \
    defined(ZLANG_STD_MATH_MIN_F32) || defined(ZLANG_STD_MATH_MIN_F64)
#define ZLANG_STD_MATH_MIN_ACTIVE
#endif

#if defined(ZLANG_STD_MATH_MAX_I32) || defined(ZLANG_STD_MATH_MAX_U32) ||                          \
    defined(ZLANG_STD_MATH_MAX_I64) || defined(ZLANG_STD_MATH_MAX_U64) ||                          \
    defined(ZLANG_STD_MATH_MAX_F32) || defined(ZLANG_STD_MATH_MAX_F64)
#define ZLANG_STD_MATH_MAX_ACTIVE
#endif

#if defined(ZLANG_STD_MATH_ABS_ACTIVE)
ZLANG_STATIC_INLINE int32_t zlang_std_math_abs_i32(int32_t value) {
    if (value == INT32_MIN) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return value < 0 ? -value : value;
}

ZLANG_STATIC_INLINE uint32_t zlang_std_math_abs_u32(uint32_t value) { return value; }

ZLANG_STATIC_INLINE int64_t zlang_std_math_abs_i64(int64_t value) {
    if (value == INT64_MIN) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return value < 0 ? -value : value;
}

ZLANG_STATIC_INLINE uint64_t zlang_std_math_abs_u64(uint64_t value) { return value; }

ZLANG_STATIC_INLINE float zlang_std_math_abs_f32(float value) {
    return value < 0.0F ? -value : value;
}

ZLANG_STATIC_INLINE double zlang_std_math_abs_f64(double value) {
    return value < 0.0 ? -value : value;
}
#endif

#if defined(ZLANG_STD_MATH_CLAMP_ACTIVE)
ZLANG_STATIC_INLINE int32_t zlang_std_math_clamp_i32(int32_t value, int32_t low, int32_t high) {
    if (low > high) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

ZLANG_STATIC_INLINE uint32_t zlang_std_math_clamp_u32(uint32_t value, uint32_t low, uint32_t high) {
    if (low > high) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

ZLANG_STATIC_INLINE int64_t zlang_std_math_clamp_i64(int64_t value, int64_t low, int64_t high) {
    if (low > high) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

ZLANG_STATIC_INLINE uint64_t zlang_std_math_clamp_u64(uint64_t value, uint64_t low, uint64_t high) {
    if (low > high) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

ZLANG_STATIC_INLINE float zlang_std_math_clamp_f32(float value, float low, float high) {
    if (low > high || !isfinite(value) || !isfinite(low) || !isfinite(high)) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

ZLANG_STATIC_INLINE double zlang_std_math_clamp_f64(double value, double low, double high) {
    if (low > high || !isfinite(value) || !isfinite(low) || !isfinite(high)) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}
#endif

#if defined(ZLANG_STD_MATH_MIN_ACTIVE)
ZLANG_STATIC_INLINE int32_t zlang_std_math_min_i32(int32_t left, int32_t right) {
    return left < right ? left : right;
}
ZLANG_STATIC_INLINE uint32_t zlang_std_math_min_u32(uint32_t left, uint32_t right) {
    return left < right ? left : right;
}
ZLANG_STATIC_INLINE int64_t zlang_std_math_min_i64(int64_t left, int64_t right) {
    return left < right ? left : right;
}
ZLANG_STATIC_INLINE uint64_t zlang_std_math_min_u64(uint64_t left, uint64_t right) {
    return left < right ? left : right;
}
ZLANG_STATIC_INLINE float zlang_std_math_min_f32(float left, float right) {
    return left < right ? left : right;
}
ZLANG_STATIC_INLINE double zlang_std_math_min_f64(double left, double right) {
    return left < right ? left : right;
}
#endif

#if defined(ZLANG_STD_MATH_MAX_ACTIVE)
ZLANG_STATIC_INLINE int32_t zlang_std_math_max_i32(int32_t left, int32_t right) {
    return left > right ? left : right;
}
ZLANG_STATIC_INLINE uint32_t zlang_std_math_max_u32(uint32_t left, uint32_t right) {
    return left > right ? left : right;
}
ZLANG_STATIC_INLINE int64_t zlang_std_math_max_i64(int64_t left, int64_t right) {
    return left > right ? left : right;
}
ZLANG_STATIC_INLINE uint64_t zlang_std_math_max_u64(uint64_t left, uint64_t right) {
    return left > right ? left : right;
}
ZLANG_STATIC_INLINE float zlang_std_math_max_f32(float left, float right) {
    return left > right ? left : right;
}
ZLANG_STATIC_INLINE double zlang_std_math_max_f64(double left, double right) {
    return left > right ? left : right;
}
#endif

#if defined(ZLANG_STD_MATH_SQRT_F32) || defined(ZLANG_STD_MATH_SQRT_F64)
ZLANG_STATIC_INLINE float zlang_std_math_sqrt_f32(float value) {
    if (value < 0.0F || !isfinite(value)) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value <= 1.0F) {
        return value;
    }
    float _low_k = 1.0F;
    float _high_k = value;
    for (uint32_t _step = 0U; _step < 32U; ++_step) {
        const float _mid_k = _low_k + ((_high_k - _low_k) / 2.0F);
        if (_mid_k > value / _mid_k) {
            _high_k = _mid_k;
        } else {
            _low_k = _mid_k;
        }
    }
    return _low_k;
}

ZLANG_STATIC_INLINE double zlang_std_math_sqrt_f64(double value) {
    if (value < 0.0 || !isfinite(value)) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value <= 1.0) {
        return value;
    }
    double _low_k = 1.0;
    double _high_k = value;
    for (uint32_t _step = 0U; _step < 64U; ++_step) {
        const double _mid_k = _low_k + ((_high_k - _low_k) / 2.0);
        if (_mid_k > value / _mid_k) {
            _high_k = _mid_k;
        } else {
            _low_k = _mid_k;
        }
    }
    return _low_k;
}
#endif

#endif /* ZLANG_STANDARD_MATH_MATH_H */
