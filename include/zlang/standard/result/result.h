#ifndef ZLANG_STANDARD_RESULT_RESULT_H
#define ZLANG_STANDARD_RESULT_RESULT_H

#include "zlang/detail/inline.h"
#include "zlang/runtime/contract.h"
#include "zlang/standard/status/status.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/** @file
 * @brief Canonical C layout and bounded capabilities for native Zlang results.
 *
 * `dump()` has no destination parameter: the canonical representation is a
 `zlang_string_t` view into storage embedded in the result. The storage exists
 * only when `ZLANG_STD_RESULT_DUMP` is defined. Timing state and clock reads
 * exist only when `ZLANG_STD_RESULT_TIMING` is defined.
 */

#define ZLANG_STD_RESULT_FIELDS_CODE zlang_std_status_t code_;
#define ZLANG_STD_RESULT_FIELDS_WHAT zlang_string_t what_;

#define ZLANG_STD_RESULT_DUMP_CAPACITY 128U

#if defined(ZLANG_STD_RESULT_TIMING)
#define ZLANG_STD_RESULT_TIME_FIELD uint64_t elapsed_ns_;
#define ZLANG_STD_RESULT_TIME_INIT , 0ULL
#define ZLANG_STD_RESULT_DUMP_TIME(result) ((result)->elapsed_ns_)
#else
#define ZLANG_STD_RESULT_TIME_FIELD
#define ZLANG_STD_RESULT_TIME_INIT
#define ZLANG_STD_RESULT_DUMP_TIME(result) 0ULL
#endif

#if defined(ZLANG_STD_RESULT_DUMP)
#define ZLANG_STD_RESULT_DUMP_FIELD char dump_storage_[ZLANG_STD_RESULT_DUMP_CAPACITY];
#define ZLANG_STD_RESULT_DUMP_INIT                                                                 \
    , { 0 }
#else
#define ZLANG_STD_RESULT_DUMP_FIELD
#define ZLANG_STD_RESULT_DUMP_INIT
#endif

typedef struct zlang_std_result_void_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    ZLANG_STD_RESULT_FIELDS_WHAT
    ZLANG_STD_RESULT_TIME_FIELD
    ZLANG_STD_RESULT_DUMP_FIELD
} zlang_std_result_void_status_t;

typedef struct zlang_std_result_i32_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    int32_t value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
    ZLANG_STD_RESULT_TIME_FIELD
    ZLANG_STD_RESULT_DUMP_FIELD
} zlang_std_result_i32_status_t;

typedef struct zlang_std_result_u32_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    uint32_t value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
    ZLANG_STD_RESULT_TIME_FIELD
    ZLANG_STD_RESULT_DUMP_FIELD
} zlang_std_result_u32_status_t;

typedef struct zlang_std_result_i64_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    int64_t value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
    ZLANG_STD_RESULT_TIME_FIELD
    ZLANG_STD_RESULT_DUMP_FIELD
} zlang_std_result_i64_status_t;

typedef struct zlang_std_result_u64_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    uint64_t value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
    ZLANG_STD_RESULT_TIME_FIELD
    ZLANG_STD_RESULT_DUMP_FIELD
} zlang_std_result_u64_status_t;

typedef struct zlang_std_result_bool_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    bool value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
    ZLANG_STD_RESULT_TIME_FIELD
    ZLANG_STD_RESULT_DUMP_FIELD
} zlang_std_result_bool_status_t;

typedef struct zlang_std_result_f32_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    float value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
    ZLANG_STD_RESULT_TIME_FIELD
    ZLANG_STD_RESULT_DUMP_FIELD
} zlang_std_result_f32_status_t;

typedef struct zlang_std_result_f64_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    double value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
    ZLANG_STD_RESULT_TIME_FIELD
    ZLANG_STD_RESULT_DUMP_FIELD
} zlang_std_result_f64_status_t;

typedef struct zlang_std_result_string_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    zlang_string_t value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
    ZLANG_STD_RESULT_TIME_FIELD
    ZLANG_STD_RESULT_DUMP_FIELD
} zlang_std_result_string_status_t;

#if defined(ZLANG_STD_RESULT_TIMING)
ZLANG_STATIC_INLINE uint64_t zlang_clock_ns(void) {
    const clock_t _ticks_k = clock();
    const uint64_t _seconds_k = (uint64_t)_ticks_k / (uint64_t)CLOCKS_PER_SEC;
    const uint64_t _remainder_k = (uint64_t)_ticks_k % (uint64_t)CLOCKS_PER_SEC;
    return _seconds_k * 1000000000ULL + (_remainder_k * 1000000000ULL) / (uint64_t)CLOCKS_PER_SEC;
}
#endif

#if defined(ZLANG_STD_RESULT_TIMING)
#define ZLANG_STD_RESULT_TIME_HELPERS(tag, type)                                                   \
    ZLANG_STATIC_INLINE type zlang_std_result_##tag##_timed(type result, uint64_t start_ns) {      \
        result.elapsed_ns_ = zlang_clock_ns() - start_ns;                                          \
        return result;                                                                             \
    }                                                                                              \
    ZLANG_STATIC_INLINE uint64_t zlang_std_result_##tag##_time(type result) {                      \
        return result.elapsed_ns_;                                                                 \
    }
#else
#define ZLANG_STD_RESULT_TIME_HELPERS(tag, type)                                                   \
    ZLANG_STATIC_INLINE type zlang_std_result_##tag##_timed(type result, uint64_t start_ns) {      \
        (void)start_ns;                                                                            \
        return result;                                                                             \
    }                                                                                              \
    ZLANG_STATIC_INLINE uint64_t zlang_std_result_##tag##_time(type result) {                      \
        (void)result;                                                                              \
        return 0U;                                                                                 \
    }
#endif

ZLANG_STD_RESULT_TIME_HELPERS(void, zlang_std_result_void_status_t)
ZLANG_STD_RESULT_TIME_HELPERS(i32, zlang_std_result_i32_status_t)
ZLANG_STD_RESULT_TIME_HELPERS(u32, zlang_std_result_u32_status_t)
ZLANG_STD_RESULT_TIME_HELPERS(i64, zlang_std_result_i64_status_t)
ZLANG_STD_RESULT_TIME_HELPERS(u64, zlang_std_result_u64_status_t)
ZLANG_STD_RESULT_TIME_HELPERS(bool, zlang_std_result_bool_status_t)
ZLANG_STD_RESULT_TIME_HELPERS(f32, zlang_std_result_f32_status_t)
ZLANG_STD_RESULT_TIME_HELPERS(f64, zlang_std_result_f64_status_t)
ZLANG_STD_RESULT_TIME_HELPERS(string, zlang_std_result_string_status_t)

#undef ZLANG_STD_RESULT_TIME_HELPERS

#if defined(ZLANG_STD_RESULT_TIMING)
#define ZLANG_STD_TIMING_BEGIN() const uint64_t _timing_start_ns_k = zlang_clock_ns()
#define ZLANG_STD_TIMED_CALL(tag, expression)                                                      \
    zlang_std_result_##tag##_timed((expression), _timing_start_ns_k)
#else
#define ZLANG_STD_TIMING_BEGIN() ((void)0)
#define ZLANG_STD_TIMED_CALL(tag, expression) (expression)
#endif

ZLANG_STATIC_INLINE zlang_std_result_void_status_t
zlang_std_result_void(const zlang_std_status_t code) {
    zlang_std_result_void_status_t _result_k = {
        code, zlang_std_status_message(code) ZLANG_STD_RESULT_TIME_INIT ZLANG_STD_RESULT_DUMP_INIT};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_result_i32(const zlang_std_status_t code, const int32_t value) {
    zlang_std_result_i32_status_t _result_k = {
        code, value,
        zlang_std_status_message(code) ZLANG_STD_RESULT_TIME_INIT ZLANG_STD_RESULT_DUMP_INIT};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_u32_status_t
zlang_std_result_u32(const zlang_std_status_t code, const uint32_t value) {
    zlang_std_result_u32_status_t _result_k = {
        code, value,
        zlang_std_status_message(code) ZLANG_STD_RESULT_TIME_INIT ZLANG_STD_RESULT_DUMP_INIT};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_i64_status_t
zlang_std_result_i64(const zlang_std_status_t code, const int64_t value) {
    zlang_std_result_i64_status_t _result_k = {
        code, value,
        zlang_std_status_message(code) ZLANG_STD_RESULT_TIME_INIT ZLANG_STD_RESULT_DUMP_INIT};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_u64_status_t
zlang_std_result_u64(const zlang_std_status_t code, const uint64_t value) {
    zlang_std_result_u64_status_t _result_k = {
        code, value,
        zlang_std_status_message(code) ZLANG_STD_RESULT_TIME_INIT ZLANG_STD_RESULT_DUMP_INIT};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_bool_status_t
zlang_std_result_bool(const zlang_std_status_t code, const bool value) {
    zlang_std_result_bool_status_t _result_k = {
        code, value,
        zlang_std_status_message(code) ZLANG_STD_RESULT_TIME_INIT ZLANG_STD_RESULT_DUMP_INIT};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_f32_status_t
zlang_std_result_f32(const zlang_std_status_t code, const float value) {
    zlang_std_result_f32_status_t _result_k = {
        code, value,
        zlang_std_status_message(code) ZLANG_STD_RESULT_TIME_INIT ZLANG_STD_RESULT_DUMP_INIT};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_f64_status_t
zlang_std_result_f64(const zlang_std_status_t code, const double value) {
    zlang_std_result_f64_status_t _result_k = {
        code, value,
        zlang_std_status_message(code) ZLANG_STD_RESULT_TIME_INIT ZLANG_STD_RESULT_DUMP_INIT};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_string_status_t
zlang_std_result_string(const zlang_std_status_t code, const zlang_string_t value) {
    zlang_std_result_string_status_t _result_k = {
        code, value,
        zlang_std_status_message(code) ZLANG_STD_RESULT_TIME_INIT ZLANG_STD_RESULT_DUMP_INIT};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_string_t zlang_std_result_dump_view(char *storage, const int written) {
    if (written < 0) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    size_t _length_k = (size_t)written;
    if (_length_k >= (size_t)ZLANG_STD_RESULT_DUMP_CAPACITY) {
        _length_k = (size_t)ZLANG_STD_RESULT_DUMP_CAPACITY - 1U;
    }
    storage[_length_k] = '\0';
    return (zlang_string_t){storage, (uint32_t)_length_k};
}
#if defined(ZLANG_STD_RESULT_DUMP)
ZLANG_STATIC_INLINE zlang_string_t zlang_std_result_dump_compose(const zlang_std_status_t code,
                                                                 const zlang_string_t value,
                                                                 const zlang_string_t what,
                                                                 const uint64_t elapsed_ns,
                                                                 char *storage) {
    if (value.data_ == NULL && value.length_ != 0U) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    const int _value_length_k = value.length_ > (uint32_t)INT_MAX ? 0 : (int)value.length_;
    const int _what_length_k = what.length_ > (uint32_t)INT_MAX ? 0 : (int)what.length_;
    const int _written_k =
        snprintf(storage, ZLANG_STD_RESULT_DUMP_CAPACITY, "code=%d value=%.*s what=%.*s time=%llu",
                 (int)code, _value_length_k, value.data_, _what_length_k, what.data_,
                 (unsigned long long)elapsed_ns);
    return zlang_std_result_dump_view(storage, _written_k);
}

#define ZLANG_STD_RESULT_DUMP_FN_VOID(tag, type)                                                   \
    ZLANG_STATIC_INLINE zlang_string_t zlang_std_result_##tag##_dump(type *result) {               \
        if (result == NULL) {                                                                      \
            zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);                                               \
        }                                                                                          \
        const zlang_string_t _what_k = zlang_std_status_message(result->code_);                    \
        const int _length_k = _what_k.length_ > (uint32_t)INT_MAX ? 0 : (int)_what_k.length_;      \
        const int _written_k =                                                                     \
            snprintf(result->dump_storage_, ZLANG_STD_RESULT_DUMP_CAPACITY,                        \
                     "code=%d what=%.*s time=%llu", (int)result->code_, _length_k, _what_k.data_,  \
                     (unsigned long long)ZLANG_STD_RESULT_DUMP_TIME(result));                      \
        return zlang_std_result_dump_view(result->dump_storage_, _written_k);                      \
    }

#define ZLANG_STD_RESULT_DUMP_FN(tag, type, format, value_argument)                                \
    ZLANG_STATIC_INLINE zlang_string_t zlang_std_result_##tag##_dump(type *result) {               \
        if (result == NULL) {                                                                      \
            zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);                                               \
        }                                                                                          \
        const zlang_string_t _what_k = zlang_std_status_message(result->code_);                    \
        const int _what_length_k = _what_k.length_ > (uint32_t)INT_MAX ? 0 : (int)_what_k.length_; \
        const int _written_k =                                                                     \
            snprintf(result->dump_storage_, ZLANG_STD_RESULT_DUMP_CAPACITY, format,                \
                     (int)result->code_, value_argument, _what_length_k, _what_k.data_,            \
                     (unsigned long long)ZLANG_STD_RESULT_DUMP_TIME(result));                      \
        return zlang_std_result_dump_view(result->dump_storage_, _written_k);                      \
    }

ZLANG_STD_RESULT_DUMP_FN_VOID(void, zlang_std_result_void_status_t)
ZLANG_STD_RESULT_DUMP_FN(i32, zlang_std_result_i32_status_t,
                         "code=%d value=%lld what=%.*s time=%llu", (long long)result->value_)
ZLANG_STD_RESULT_DUMP_FN(u32, zlang_std_result_u32_status_t,
                         "code=%d value=%llu what=%.*s time=%llu",
                         (unsigned long long)result->value_)
ZLANG_STD_RESULT_DUMP_FN(i64, zlang_std_result_i64_status_t,
                         "code=%d value=%lld what=%.*s time=%llu", (long long)result->value_)
ZLANG_STD_RESULT_DUMP_FN(u64, zlang_std_result_u64_status_t,
                         "code=%d value=%llu what=%.*s time=%llu",
                         (unsigned long long)result->value_)
ZLANG_STD_RESULT_DUMP_FN(bool, zlang_std_result_bool_status_t,
                         "code=%d value=%s what=%.*s time=%llu", result->value_ ? "true" : "false")
ZLANG_STD_RESULT_DUMP_FN(f32, zlang_std_result_f32_status_t, "code=%d value=%g what=%.*s time=%llu",
                         (double)result->value_)
ZLANG_STD_RESULT_DUMP_FN(f64, zlang_std_result_f64_status_t, "code=%d value=%g what=%.*s time=%llu",
                         result->value_)
ZLANG_STATIC_INLINE zlang_string_t
zlang_std_result_string_dump(zlang_std_result_string_status_t *result) {
    if (result == NULL || (result->value_.data_ == NULL && result->value_.length_ != 0U)) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    const zlang_string_t _what_k = zlang_std_status_message(result->code_);
    const int _value_length_k =
        result->value_.length_ > (uint32_t)INT_MAX ? 0 : (int)result->value_.length_;
    const int _what_length_k = _what_k.length_ > (uint32_t)INT_MAX ? 0 : (int)_what_k.length_;
    const int _written_k =
        snprintf(result->dump_storage_, ZLANG_STD_RESULT_DUMP_CAPACITY,
                 "code=%d value=%.*s what=%.*s time=%llu", (int)result->code_, _value_length_k,
                 result->value_.data_, _what_length_k, _what_k.data_,
                 (unsigned long long)ZLANG_STD_RESULT_DUMP_TIME(result));
    return zlang_std_result_dump_view(result->dump_storage_, _written_k);
}

#undef ZLANG_STD_RESULT_DUMP_FN
#undef ZLANG_STD_RESULT_DUMP_FN_VOID
#else
ZLANG_STATIC_INLINE zlang_string_t
zlang_std_result_void_dump(zlang_std_result_void_status_t *result) {
    (void)result;
    return (zlang_string_t){"", 0U};
}
ZLANG_STATIC_INLINE zlang_string_t
zlang_std_result_i32_dump(zlang_std_result_i32_status_t *result) {
    (void)result;
    return (zlang_string_t){"", 0U};
}
ZLANG_STATIC_INLINE zlang_string_t
zlang_std_result_u32_dump(zlang_std_result_u32_status_t *result) {
    (void)result;
    return (zlang_string_t){"", 0U};
}
ZLANG_STATIC_INLINE zlang_string_t
zlang_std_result_i64_dump(zlang_std_result_i64_status_t *result) {
    (void)result;
    return (zlang_string_t){"", 0U};
}
ZLANG_STATIC_INLINE zlang_string_t
zlang_std_result_u64_dump(zlang_std_result_u64_status_t *result) {
    (void)result;
    return (zlang_string_t){"", 0U};
}
ZLANG_STATIC_INLINE zlang_string_t
zlang_std_result_bool_dump(zlang_std_result_bool_status_t *result) {
    (void)result;
    return (zlang_string_t){"", 0U};
}
ZLANG_STATIC_INLINE zlang_string_t
zlang_std_result_f32_dump(zlang_std_result_f32_status_t *result) {
    (void)result;
    return (zlang_string_t){"", 0U};
}
ZLANG_STATIC_INLINE zlang_string_t
zlang_std_result_f64_dump(zlang_std_result_f64_status_t *result) {
    (void)result;
    return (zlang_string_t){"", 0U};
}
ZLANG_STATIC_INLINE zlang_string_t
zlang_std_result_string_dump(zlang_std_result_string_status_t *result) {
    (void)result;
    return (zlang_string_t){"", 0U};
}
#endif

#endif /* ZLANG_STANDARD_RESULT_RESULT_H */
