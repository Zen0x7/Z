#ifndef ZLANG_STANDARD_MATH_MATH_H
#define ZLANG_STANDARD_MATH_MATH_H

#include "zlang/detail/inline.h"
#include "zlang/standard/result/result.h"

#include <math.h>
#include <stdint.h>

#if defined(ZLANG_STD_MATH_ABS_I32) || defined(ZLANG_STD_MATH_ABS_U32) ||                          \
    defined(ZLANG_STD_MATH_ABS_I64) || defined(ZLANG_STD_MATH_ABS_U64) ||                          \
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
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_math_abs_i32(int32_t value) {
    ZLANG_STD_TIMING_BEGIN();
    /* cppcheck-suppress integerOverflowCond */
    if (value == INT32_MIN) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
        return ZLANG_STD_TIMED_CALL(i32,
                                    zlang_std_result_i32(zlang_std_status_invalid_argument_, 0));
    }
    return ZLANG_STD_TIMED_CALL(
        i32, zlang_std_result_i32(zlang_std_status_success_, value < 0 ? -value : value));
}

ZLANG_STATIC_INLINE zlang_std_result_u32_status_t zlang_std_math_abs_u32(uint32_t value) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(u32, zlang_std_result_u32(zlang_std_status_success_, value));
}

ZLANG_STATIC_INLINE zlang_std_result_i64_status_t zlang_std_math_abs_i64(int64_t value) {
    ZLANG_STD_TIMING_BEGIN();
    /* cppcheck-suppress integerOverflowCond */
    if (value == INT64_MIN) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
        return ZLANG_STD_TIMED_CALL(i64,
                                    zlang_std_result_i64(zlang_std_status_invalid_argument_, 0));
    }
    return ZLANG_STD_TIMED_CALL(
        i64, zlang_std_result_i64(zlang_std_status_success_, value < 0 ? -value : value));
}

ZLANG_STATIC_INLINE zlang_std_result_u64_status_t zlang_std_math_abs_u64(uint64_t value) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(u64, zlang_std_result_u64(zlang_std_status_success_, value));
}

ZLANG_STATIC_INLINE zlang_std_result_f32_status_t zlang_std_math_abs_f32(float value) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(
        f32, zlang_std_result_f32(zlang_std_status_success_, value < 0.0F ? -value : value));
}

ZLANG_STATIC_INLINE zlang_std_result_f64_status_t zlang_std_math_abs_f64(double value) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(
        f64, zlang_std_result_f64(zlang_std_status_success_, value < 0.0 ? -value : value));
}
#endif

#if defined(ZLANG_STD_MATH_CLAMP_ACTIVE)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_math_clamp_i32(int32_t value,
                                                                           int32_t low,
                                                                           int32_t high) {
    ZLANG_STD_TIMING_BEGIN();
    if (low > high) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value < low) {
        return ZLANG_STD_TIMED_CALL(i32, zlang_std_result_i32(zlang_std_status_success_, low));
    }
    if (value > high) {
        return ZLANG_STD_TIMED_CALL(i32, zlang_std_result_i32(zlang_std_status_success_, high));
    }
    return ZLANG_STD_TIMED_CALL(i32, zlang_std_result_i32(zlang_std_status_success_, value));
}

ZLANG_STATIC_INLINE zlang_std_result_u32_status_t zlang_std_math_clamp_u32(uint32_t value,
                                                                           uint32_t low,
                                                                           uint32_t high) {
    ZLANG_STD_TIMING_BEGIN();
    if (low > high) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value < low) {
        return ZLANG_STD_TIMED_CALL(u32, zlang_std_result_u32(zlang_std_status_success_, low));
    }
    if (value > high) {
        return ZLANG_STD_TIMED_CALL(u32, zlang_std_result_u32(zlang_std_status_success_, high));
    }
    return ZLANG_STD_TIMED_CALL(u32, zlang_std_result_u32(zlang_std_status_success_, value));
}

ZLANG_STATIC_INLINE zlang_std_result_i64_status_t zlang_std_math_clamp_i64(int64_t value,
                                                                           int64_t low,
                                                                           int64_t high) {
    ZLANG_STD_TIMING_BEGIN();
    if (low > high) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value < low) {
        return ZLANG_STD_TIMED_CALL(i64, zlang_std_result_i64(zlang_std_status_success_, low));
    }
    if (value > high) {
        return ZLANG_STD_TIMED_CALL(i64, zlang_std_result_i64(zlang_std_status_success_, high));
    }
    return ZLANG_STD_TIMED_CALL(i64, zlang_std_result_i64(zlang_std_status_success_, value));
}

ZLANG_STATIC_INLINE zlang_std_result_u64_status_t zlang_std_math_clamp_u64(uint64_t value,
                                                                           uint64_t low,
                                                                           uint64_t high) {
    ZLANG_STD_TIMING_BEGIN();
    if (low > high) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value < low) {
        return ZLANG_STD_TIMED_CALL(u64, zlang_std_result_u64(zlang_std_status_success_, low));
    }
    if (value > high) {
        return ZLANG_STD_TIMED_CALL(u64, zlang_std_result_u64(zlang_std_status_success_, high));
    }
    return ZLANG_STD_TIMED_CALL(u64, zlang_std_result_u64(zlang_std_status_success_, value));
}

ZLANG_STATIC_INLINE zlang_std_result_f32_status_t zlang_std_math_clamp_f32(float value, float low,
                                                                           float high) {
    ZLANG_STD_TIMING_BEGIN();
    if (low > high || !isfinite(value) || !isfinite(low) || !isfinite(high)) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value < low) {
        return ZLANG_STD_TIMED_CALL(f32, zlang_std_result_f32(zlang_std_status_success_, low));
    }
    if (value > high) {
        return ZLANG_STD_TIMED_CALL(f32, zlang_std_result_f32(zlang_std_status_success_, high));
    }
    return ZLANG_STD_TIMED_CALL(f32, zlang_std_result_f32(zlang_std_status_success_, value));
}

ZLANG_STATIC_INLINE zlang_std_result_f64_status_t zlang_std_math_clamp_f64(double value, double low,
                                                                           double high) {
    ZLANG_STD_TIMING_BEGIN();
    if (low > high || !isfinite(value) || !isfinite(low) || !isfinite(high)) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value < low) {
        return ZLANG_STD_TIMED_CALL(f64, zlang_std_result_f64(zlang_std_status_success_, low));
    }
    if (value > high) {
        return ZLANG_STD_TIMED_CALL(f64, zlang_std_result_f64(zlang_std_status_success_, high));
    }
    return ZLANG_STD_TIMED_CALL(f64, zlang_std_result_f64(zlang_std_status_success_, value));
}
#endif

#if defined(ZLANG_STD_MATH_MIN_ACTIVE)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_math_min_i32(int32_t left,
                                                                         int32_t right) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(
        i32, zlang_std_result_i32(zlang_std_status_success_, left < right ? left : right));
}
ZLANG_STATIC_INLINE zlang_std_result_u32_status_t zlang_std_math_min_u32(uint32_t left,
                                                                         uint32_t right) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(
        u32, zlang_std_result_u32(zlang_std_status_success_, left < right ? left : right));
}
ZLANG_STATIC_INLINE zlang_std_result_i64_status_t zlang_std_math_min_i64(int64_t left,
                                                                         int64_t right) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(
        i64, zlang_std_result_i64(zlang_std_status_success_, left < right ? left : right));
}
ZLANG_STATIC_INLINE zlang_std_result_u64_status_t zlang_std_math_min_u64(uint64_t left,
                                                                         uint64_t right) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(
        u64, zlang_std_result_u64(zlang_std_status_success_, left < right ? left : right));
}
ZLANG_STATIC_INLINE zlang_std_result_f32_status_t zlang_std_math_min_f32(float left, float right) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(
        f32, zlang_std_result_f32(zlang_std_status_success_, left < right ? left : right));
}
ZLANG_STATIC_INLINE zlang_std_result_f64_status_t zlang_std_math_min_f64(double left,
                                                                         double right) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(
        f64, zlang_std_result_f64(zlang_std_status_success_, left < right ? left : right));
}
#endif

#if defined(ZLANG_STD_MATH_MAX_ACTIVE)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_math_max_i32(int32_t left,
                                                                         int32_t right) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(
        i32, zlang_std_result_i32(zlang_std_status_success_, left > right ? left : right));
}
ZLANG_STATIC_INLINE zlang_std_result_u32_status_t zlang_std_math_max_u32(uint32_t left,
                                                                         uint32_t right) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(
        u32, zlang_std_result_u32(zlang_std_status_success_, left > right ? left : right));
}
ZLANG_STATIC_INLINE zlang_std_result_i64_status_t zlang_std_math_max_i64(int64_t left,
                                                                         int64_t right) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(
        i64, zlang_std_result_i64(zlang_std_status_success_, left > right ? left : right));
}
ZLANG_STATIC_INLINE zlang_std_result_u64_status_t zlang_std_math_max_u64(uint64_t left,
                                                                         uint64_t right) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(
        u64, zlang_std_result_u64(zlang_std_status_success_, left > right ? left : right));
}
ZLANG_STATIC_INLINE zlang_std_result_f32_status_t zlang_std_math_max_f32(float left, float right) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(
        f32, zlang_std_result_f32(zlang_std_status_success_, left > right ? left : right));
}
ZLANG_STATIC_INLINE zlang_std_result_f64_status_t zlang_std_math_max_f64(double left,
                                                                         double right) {
    ZLANG_STD_TIMING_BEGIN();
    return ZLANG_STD_TIMED_CALL(
        f64, zlang_std_result_f64(zlang_std_status_success_, left > right ? left : right));
}
#endif

#if defined(ZLANG_STD_MATH_SQRT_F32) || defined(ZLANG_STD_MATH_SQRT_F64)
ZLANG_STATIC_INLINE zlang_std_result_f32_status_t zlang_std_math_sqrt_f32(float value) {
    ZLANG_STD_TIMING_BEGIN();
    if (value < 0.0F || !isfinite(value)) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value <= 1.0F) {
        return ZLANG_STD_TIMED_CALL(f32, zlang_std_result_f32(zlang_std_status_success_, value));
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
    return ZLANG_STD_TIMED_CALL(f32, zlang_std_result_f32(zlang_std_status_success_, _low_k));
}

ZLANG_STATIC_INLINE zlang_std_result_f64_status_t zlang_std_math_sqrt_f64(double value) {
    ZLANG_STD_TIMING_BEGIN();
    if (value < 0.0 || !isfinite(value)) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value <= 1.0) {
        return ZLANG_STD_TIMED_CALL(f64, zlang_std_result_f64(zlang_std_status_success_, value));
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
    return ZLANG_STD_TIMED_CALL(f64, zlang_std_result_f64(zlang_std_status_success_, _low_k));
}
#endif

#endif /* ZLANG_STANDARD_MATH_MATH_H */
