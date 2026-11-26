#ifndef ZLANG_STANDARD_RESULT_RESULT_H
#define ZLANG_STANDARD_RESULT_RESULT_H

#include "zlang/detail/inline.h"
#include "zlang/runtime/runtime.h"

#include <stdbool.h>
#include <stdint.h>

/** @file
 * @brief Canonical C layout for Zlang native `Result<T, Status>` values.
 *
 * The header owns the C representation. The compiler maps the Zlang type
 * application to one of the typedefs below. No message is formatted at
 * runtime: `what` is a read-only view of a fixed literal.
 */

typedef enum zlang_std_status_t {
    ZLANG_STD_STATUS_SUCCESS = 0U,
    ZLANG_STD_STATUS_END_OF_INPUT = 1U,
    ZLANG_STD_STATUS_IO_ERROR = 2U,
    ZLANG_STD_STATUS_INVALID_ARGUMENT = 3U,
    ZLANG_STD_STATUS_CAPACITY = 4U
} zlang_std_status_t;

ZLANG_STATIC_INLINE zlang_string_t zlang_std_status_message(const zlang_std_status_t code) {
    switch (code) {
    case ZLANG_STD_STATUS_SUCCESS:
        return (zlang_string_t){"success", 7U};
    case ZLANG_STD_STATUS_END_OF_INPUT:
        return (zlang_string_t){"end of input", 12U};
    case ZLANG_STD_STATUS_IO_ERROR:
        return (zlang_string_t){"input/output error", 18U};
    case ZLANG_STD_STATUS_INVALID_ARGUMENT:
        return (zlang_string_t){"invalid argument", 16U};
    case ZLANG_STD_STATUS_CAPACITY:
        return (zlang_string_t){"capacity exceeded", 17U};
    default:
        return (zlang_string_t){"unknown status", 14U};
    }
}

ZLANG_STATIC_INLINE bool zlang_std_status_is_ok(const zlang_std_status_t code) {
    return code == ZLANG_STD_STATUS_SUCCESS;
}

#define ZLANG_STD_RESULT_FIELDS_CODE zlang_std_status_t code_;
#define ZLANG_STD_RESULT_FIELDS_WHAT zlang_string_t what_;

typedef struct zlang_std_result_void_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    ZLANG_STD_RESULT_FIELDS_WHAT
} zlang_std_result_void_status_t;

typedef struct zlang_std_result_i32_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    int32_t value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
} zlang_std_result_i32_status_t;

typedef struct zlang_std_result_u32_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    uint32_t value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
} zlang_std_result_u32_status_t;

typedef struct zlang_std_result_i64_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    int64_t value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
} zlang_std_result_i64_status_t;

typedef struct zlang_std_result_u64_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    uint64_t value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
} zlang_std_result_u64_status_t;

typedef struct zlang_std_result_bool_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    bool value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
} zlang_std_result_bool_status_t;

typedef struct zlang_std_result_f32_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    float value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
} zlang_std_result_f32_status_t;

typedef struct zlang_std_result_f64_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    double value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
} zlang_std_result_f64_status_t;

typedef struct zlang_std_result_string_status_t {
    ZLANG_STD_RESULT_FIELDS_CODE
    zlang_string_t value_;
    ZLANG_STD_RESULT_FIELDS_WHAT
} zlang_std_result_string_status_t;

ZLANG_STATIC_INLINE zlang_std_result_void_status_t
zlang_std_result_void(const zlang_std_status_t code) {
    zlang_std_result_void_status_t result = {code, zlang_std_status_message(code)};
    return result;
}

ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_result_i32(const zlang_std_status_t code, const int32_t value) {
    zlang_std_result_i32_status_t result = {code, value, zlang_std_status_message(code)};
    return result;
}

ZLANG_STATIC_INLINE zlang_std_result_u32_status_t
zlang_std_result_u32(const zlang_std_status_t code, const uint32_t value) {
    zlang_std_result_u32_status_t result = {code, value, zlang_std_status_message(code)};
    return result;
}

ZLANG_STATIC_INLINE zlang_std_result_i64_status_t
zlang_std_result_i64(const zlang_std_status_t code, const int64_t value) {
    zlang_std_result_i64_status_t result = {code, value, zlang_std_status_message(code)};
    return result;
}

ZLANG_STATIC_INLINE zlang_std_result_u64_status_t
zlang_std_result_u64(const zlang_std_status_t code, const uint64_t value) {
    zlang_std_result_u64_status_t result = {code, value, zlang_std_status_message(code)};
    return result;
}

ZLANG_STATIC_INLINE zlang_std_result_bool_status_t
zlang_std_result_bool(const zlang_std_status_t code, const bool value) {
    zlang_std_result_bool_status_t result = {code, value, zlang_std_status_message(code)};
    return result;
}

ZLANG_STATIC_INLINE zlang_std_result_f32_status_t
zlang_std_result_f32(const zlang_std_status_t code, const float value) {
    zlang_std_result_f32_status_t result = {code, value, zlang_std_status_message(code)};
    return result;
}

ZLANG_STATIC_INLINE zlang_std_result_f64_status_t
zlang_std_result_f64(const zlang_std_status_t code, const double value) {
    zlang_std_result_f64_status_t result = {code, value, zlang_std_status_message(code)};
    return result;
}

ZLANG_STATIC_INLINE zlang_std_result_string_status_t
zlang_std_result_string(const zlang_std_status_t code, const zlang_string_t value) {
    zlang_std_result_string_status_t result = {code, value, zlang_std_status_message(code)};
    return result;
}

#endif /* ZLANG_STANDARD_RESULT_RESULT_H */
