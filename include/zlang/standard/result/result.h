#ifndef ZLANG_STANDARD_RESULT_RESULT_H
#define ZLANG_STANDARD_RESULT_RESULT_H

#include "zlang/detail/inline.h"
#include "zlang/runtime/contract.h"
#include "zlang/standard/status/status.h"

#include <stdbool.h>
#include <stdint.h>

/** @file
 * @brief Canonical C layout for Zlang native `result_type<T, status_code>` values.
 *
 * The header owns the C representation. The compiler maps the Zlang type
 * application to one of the typedefs below. No message is formatted at
 * runtime: `what` is a read-only view of a fixed literal.
 */

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
    zlang_std_result_void_status_t _result_k = {code, zlang_std_status_message(code)};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_result_i32(const zlang_std_status_t code, const int32_t value) {
    zlang_std_result_i32_status_t _result_k = {code, value, zlang_std_status_message(code)};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_u32_status_t
zlang_std_result_u32(const zlang_std_status_t code, const uint32_t value) {
    zlang_std_result_u32_status_t _result_k = {code, value, zlang_std_status_message(code)};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_i64_status_t
zlang_std_result_i64(const zlang_std_status_t code, const int64_t value) {
    zlang_std_result_i64_status_t _result_k = {code, value, zlang_std_status_message(code)};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_u64_status_t
zlang_std_result_u64(const zlang_std_status_t code, const uint64_t value) {
    zlang_std_result_u64_status_t _result_k = {code, value, zlang_std_status_message(code)};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_bool_status_t
zlang_std_result_bool(const zlang_std_status_t code, const bool value) {
    zlang_std_result_bool_status_t _result_k = {code, value, zlang_std_status_message(code)};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_f32_status_t
zlang_std_result_f32(const zlang_std_status_t code, const float value) {
    zlang_std_result_f32_status_t _result_k = {code, value, zlang_std_status_message(code)};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_f64_status_t
zlang_std_result_f64(const zlang_std_status_t code, const double value) {
    zlang_std_result_f64_status_t _result_k = {code, value, zlang_std_status_message(code)};
    return _result_k;
}

ZLANG_STATIC_INLINE zlang_std_result_string_status_t
zlang_std_result_string(const zlang_std_status_t code, const zlang_string_t value) {
    zlang_std_result_string_status_t _result_k = {code, value, zlang_std_status_message(code)};
    return _result_k;
}

#endif /* ZLANG_STANDARD_RESULT_RESULT_H */
