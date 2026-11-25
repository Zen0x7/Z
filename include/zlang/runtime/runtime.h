#ifndef ZLANG_RUNTIME_RUNTIME_H
#define ZLANG_RUNTIME_RUNTIME_H

#include "zlang/detail/inline.h"
#include "zlang/runtime/context.h"

#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/** @file
 * @brief Runtime primitives called by C code emitted from Zlang.
 *
 * Generated code never formats an error string at runtime. Errors are reported as fixed numeric
 * trap codes declared in SPEC.md.
 */
#define ZLANG_TRAP_OVERFLOW 1U
#define ZLANG_TRAP_BOUNDS 2U
#define ZLANG_TRAP_CAPACITY 3U
#define ZLANG_TRAP_INVALID_ARGUMENT 4U
#define ZLANG_TRAP_DIVISION_BY_ZERO 5U

/** Read-only view of host process arguments created by generated main. */
typedef struct zlang_arguments_t {
    int argc_;
    char **argv_;
} zlang_arguments_t;

/** A non-owning byte string: a pointer and an explicit length. */
typedef struct zlang_string_t {
    const char *data_;
    uint32_t length_;
} zlang_string_t;

/**
 * @brief Write the exact declared byte count of a string to a stream.
 * @param stream Destination stream.
 * @param value String view to write.
 * @return `true` on success.
 */
ZLANG_STATIC_INLINE bool zlang_write(FILE *stream, zlang_string_t value) {
    if (stream == NULL || (value.data_ == NULL && value.length_ != 0U)) {
        return false;
    }
    if (value.length_ == 0U) {
        return true;
    }
    return fwrite(value.data_, 1U, value.length_, stream) == value.length_;
}

/**
 * @brief Write a string view to standard output.
 * @param value String view to write.
 * @return `true` on success.
 */
ZLANG_STATIC_INLINE bool zlang_print(zlang_string_t value) { return zlang_write(stdout, value); }

#ifndef ZLANG_TRAP_SINK
/* raise() terminates through SIGABRT without giving optimizers a noreturn
   edge here, so callers can still guard custom sinks that do return. */
#define ZLANG_TRAP_SINK(code) ((void)(code), (void)raise(SIGABRT))
#endif

/**
 * @brief Terminate execution with a fixed trap code.
 * @param code One of the `ZLANG_TRAP_*` values.
 */
ZLANG_STATIC_INLINE void zlang_trap(uint8_t code) {
    (void)code;
    ZLANG_TRAP_SINK(code);
}

/**
 * @brief Add two signed integers, trapping on overflow.
 * @param left Left operand.
 * @param right Right operand.
 * @return The sum when it is representable.
 */
ZLANG_STATIC_INLINE int32_t zlang_add_i32(int32_t left, int32_t right) {
    const bool _overflow_k =
        (right > 0 && left > INT32_MAX - right) || (right < 0 && left < INT32_MIN - right);
    if (_overflow_k) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return left + right;
}

/**
 * @brief Add two unsigned integers, trapping on overflow.
 * @param left Left operand.
 * @param right Right operand.
 * @return The sum when it is representable.
 */
ZLANG_STATIC_INLINE uint32_t zlang_add_u32(uint32_t left, uint32_t right) {
    if (right > UINT32_MAX - left) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return left + right;
}

/**
 * @brief Print a signed integer using fixed stack storage.
 * @param value Value to write.
 * @return `true` on success.
 */
ZLANG_STATIC_INLINE bool zlang_print_i32(int32_t value) {
    char _digits[16];
    size_t _position_k = sizeof(_digits) - 1U;
    uint32_t _magnitude_k = value < 0 ? (uint32_t)(-(int64_t)value) : (uint32_t)value;

    _digits[_position_k] = '\0';
    do {
        _digits[--_position_k] = (char)('0' + (_magnitude_k % 10U));
        _magnitude_k /= 10U;
    } while (_magnitude_k != 0U);
    if (value < 0) {
        _digits[--_position_k] = '-';
    }
    return zlang_write(stdout, (zlang_string_t){_digits + _position_k,
                                                (uint32_t)(sizeof(_digits) - 1U - _position_k)});
}

/**
 * @brief Print an unsigned integer using fixed stack storage.
 * @param value Value to write.
 * @return `true` on success.
 */
ZLANG_STATIC_INLINE bool zlang_print_u32(uint32_t value) {
    char _digits[16];
    size_t _position_k = sizeof(_digits) - 1U;

    _digits[_position_k] = '\0';
    do {
        _digits[--_position_k] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U);
    return zlang_write(stdout, (zlang_string_t){_digits + _position_k,
                                                (uint32_t)(sizeof(_digits) - 1U - _position_k)});
}

/**
 * @brief Print a boolean as `true` or `false`.
 * @param value Value to write.
 * @return `true` on success.
 */
ZLANG_STATIC_INLINE bool zlang_print_bool(bool value) {
    return zlang_print((zlang_string_t){value ? "true" : "false", value ? 4U : 5U});
}

/**
 * @brief Negate a signed 32-bit integer, trapping on overflow.
 */
ZLANG_STATIC_INLINE int32_t zlang_neg_i32(int32_t value) {
    if (value == INT32_MIN) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return -value;
}

/**
 * @brief Negate a signed 64-bit integer, trapping on overflow.
 */
ZLANG_STATIC_INLINE int64_t zlang_neg_i64(int64_t value) {
    if (value == INT64_MIN) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return -value;
}

/**
 * @brief Subtract two signed integers, trapping on overflow.
 */
ZLANG_STATIC_INLINE int32_t zlang_sub_i32(int32_t left, int32_t right) {
    const int64_t _result_k = (int64_t)left - (int64_t)right;
    if (_result_k < INT32_MIN || _result_k > INT32_MAX) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return (int32_t)_result_k;
}

/**
 * @brief Subtract two unsigned integers, trapping on overflow.
 */
ZLANG_STATIC_INLINE uint32_t zlang_sub_u32(uint32_t left, uint32_t right) {
    if (right > left) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return left - right;
}

/**
 * @brief Multiply two signed integers, trapping on overflow.
 */
ZLANG_STATIC_INLINE int32_t zlang_mul_i32(int32_t left, int32_t right) {
    const int64_t _result_k = (int64_t)left * (int64_t)right;
    if (_result_k < INT32_MIN || _result_k > INT32_MAX) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return (int32_t)_result_k;
}

/**
 * @brief Multiply two unsigned integers, trapping on overflow.
 */
ZLANG_STATIC_INLINE uint32_t zlang_mul_u32(uint32_t left, uint32_t right) {
    if (left != 0U && right > UINT32_MAX / left) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return left * right;
}

/**
 * @brief Divide two signed integers, trapping on zero and overflow.
 */
ZLANG_STATIC_INLINE int32_t zlang_div_i32(int32_t left, int32_t right) {
    if (right == 0) {
        zlang_trap(ZLANG_TRAP_DIVISION_BY_ZERO);
    }
    if (left == INT32_MIN && right == -1) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    // cppcheck-suppress zerodiv
    // cppcheck-suppress zerodivcond
    return left / right;
}

/**
 * @brief Divide two unsigned integers, trapping on zero.
 */
ZLANG_STATIC_INLINE uint32_t zlang_div_u32(uint32_t left, uint32_t right) {
    if (right == 0U) {
        zlang_trap(ZLANG_TRAP_DIVISION_BY_ZERO);
    }
    // cppcheck-suppress zerodiv
    // cppcheck-suppress zerodivcond
    return left / right;
}

/**
 * @brief Remainder of two signed integers, trapping on zero and overflow.
 */
ZLANG_STATIC_INLINE int32_t zlang_mod_i32(int32_t left, int32_t right) {
    if (right == 0) {
        zlang_trap(ZLANG_TRAP_DIVISION_BY_ZERO);
    }
    if (left == INT32_MIN && right == -1) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    // cppcheck-suppress zerodiv
    // cppcheck-suppress zerodivcond
    return left % right;
}

/**
 * @brief Remainder of two unsigned integers, trapping on zero.
 */
ZLANG_STATIC_INLINE uint32_t zlang_mod_u32(uint32_t left, uint32_t right) {
    if (right == 0U) {
        zlang_trap(ZLANG_TRAP_DIVISION_BY_ZERO);
    }
    // cppcheck-suppress zerodiv
    // cppcheck-suppress zerodivcond
    return left % right;
}

/**
 * @brief Print a signed 64-bit integer using fixed stack storage.
 */
ZLANG_STATIC_INLINE bool zlang_print_i64(int64_t value) {
    char _digits[24];
    size_t _position_k = sizeof(_digits) - 1U;
    uint64_t _magnitude_k = value < 0 ? (uint64_t)(-(value + 1)) + 1U : (uint64_t)value;

    _digits[_position_k] = '\0';
    do {
        _digits[--_position_k] = (char)('0' + (_magnitude_k % 10U));
        _magnitude_k /= 10U;
    } while (_magnitude_k != 0U);
    if (value < 0) {
        _digits[--_position_k] = '-';
    }
    return zlang_write(stdout, (zlang_string_t){_digits + _position_k,
                                                (uint32_t)(sizeof(_digits) - 1U - _position_k)});
}

/**
 * @brief Print an unsigned 64-bit integer using fixed stack storage.
 */
ZLANG_STATIC_INLINE bool zlang_print_u64(uint64_t value) {
    char _digits[24];
    size_t _position_k = sizeof(_digits) - 1U;

    _digits[_position_k] = '\0';
    do {
        _digits[--_position_k] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U);
    return zlang_write(stdout, (zlang_string_t){_digits + _position_k,
                                                (uint32_t)(sizeof(_digits) - 1U - _position_k)});
}

/**
 * @brief Print a 32-bit float with a deterministic decimal form.
 */
ZLANG_STATIC_INLINE bool zlang_print_f32(float value) {
    char _digits[64];
    const int _written_k = snprintf(_digits, sizeof(_digits), "%.9g", (double)value);
    if (_written_k < 0 || (size_t)_written_k >= sizeof(_digits)) {
        return false;
    }
    return zlang_print((zlang_string_t){_digits, (uint32_t)_written_k});
}

/**
 * @brief Print a 64-bit double with a deterministic decimal form.
 */
ZLANG_STATIC_INLINE bool zlang_print_f64(double value) {
    char _digits[64];
    const int _written_k = snprintf(_digits, sizeof(_digits), "%.17g", value);
    if (_written_k < 0 || (size_t)_written_k >= sizeof(_digits)) {
        return false;
    }
    return zlang_print((zlang_string_t){_digits, (uint32_t)_written_k});
}

/**
 * @brief Add two signed 64-bit integers, trapping on overflow.
 */
ZLANG_STATIC_INLINE int64_t zlang_add_i64(int64_t left, int64_t right) {
    const bool _overflow_k =
        (right > 0 && left > INT64_MAX - right) || (right < 0 && left < INT64_MIN - right);
    if (_overflow_k) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return left + right;
}

/**
 * @brief Add two unsigned 64-bit integers, trapping on overflow.
 */
ZLANG_STATIC_INLINE uint64_t zlang_add_u64(uint64_t left, uint64_t right) {
    if (right > UINT64_MAX - left) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return left + right;
}

/**
 * @brief Subtract two signed 64-bit integers, trapping on overflow.
 */
ZLANG_STATIC_INLINE int64_t zlang_sub_i64(int64_t left, int64_t right) {
    const bool _overflow_k =
        (right < 0 && left > INT64_MAX + right) || (right > 0 && left < INT64_MIN + right);
    if (_overflow_k) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return left - right;
}

/**
 * @brief Subtract two unsigned 64-bit integers, trapping on overflow.
 */
ZLANG_STATIC_INLINE uint64_t zlang_sub_u64(uint64_t left, uint64_t right) {
    if (right > left) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return left - right;
}

/**
 * @brief Multiply two signed 64-bit integers, trapping on overflow.
 */
ZLANG_STATIC_INLINE int64_t zlang_mul_i64(int64_t left, int64_t right) {
    if ((left == -1 && right == INT64_MIN) || (right == -1 && left == INT64_MIN)) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    uint64_t _magnitude_left_k = left < 0 ? (uint64_t)(-(left + 1)) + 1U : (uint64_t)left;
    uint64_t _magnitude_right_k = right < 0 ? (uint64_t)(-(right + 1)) + 1U : (uint64_t)right;
    const uint64_t _limit_k =
        (left < 0) != (right < 0) ? (uint64_t)INT64_MAX + 1U : (uint64_t)INT64_MAX;
    if (_magnitude_left_k != 0U && _magnitude_right_k > _limit_k / _magnitude_left_k) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    const uint64_t _result_k = _magnitude_left_k * _magnitude_right_k;
    if ((left < 0) != (right < 0)) {
        if (_result_k == (uint64_t)INT64_MAX + 1U) {
            return INT64_MIN;
        }
        return -(int64_t)_result_k;
    }
    return (int64_t)_result_k;
}

/**
 * @brief Multiply two unsigned 64-bit integers, trapping on overflow.
 */
ZLANG_STATIC_INLINE uint64_t zlang_mul_u64(uint64_t left, uint64_t right) {
    if (left != 0U && right > UINT64_MAX / left) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return left * right;
}

/**
 * @brief Divide two signed 64-bit integers, trapping on zero and overflow.
 */
ZLANG_STATIC_INLINE int64_t zlang_div_i64(int64_t left, int64_t right) {
    if (right == 0) {
        zlang_trap(ZLANG_TRAP_DIVISION_BY_ZERO);
    }
    if (left == INT64_MIN && right == -1) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    // cppcheck-suppress zerodiv
    // cppcheck-suppress zerodivcond
    return left / right;
}

/**
 * @brief Divide two unsigned 64-bit integers, trapping on zero.
 */
ZLANG_STATIC_INLINE uint64_t zlang_div_u64(uint64_t left, uint64_t right) {
    if (right == 0U) {
        zlang_trap(ZLANG_TRAP_DIVISION_BY_ZERO);
    }
    // cppcheck-suppress zerodiv
    // cppcheck-suppress zerodivcond
    return left / right;
}

/**
 * @brief Remainder of two signed 64-bit integers.
 */
ZLANG_STATIC_INLINE int64_t zlang_mod_i64(int64_t left, int64_t right) {
    if (right == 0) {
        zlang_trap(ZLANG_TRAP_DIVISION_BY_ZERO);
    }
    if (left == INT64_MIN && right == -1) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    // cppcheck-suppress zerodiv
    // cppcheck-suppress zerodivcond
    return left % right;
}

/**
 * @brief Remainder of two unsigned 64-bit integers.
 */
ZLANG_STATIC_INLINE uint64_t zlang_mod_u64(uint64_t left, uint64_t right) {
    if (right == 0U) {
        zlang_trap(ZLANG_TRAP_DIVISION_BY_ZERO);
    }
    // cppcheck-suppress zerodiv
    // cppcheck-suppress zerodivcond
    return left % right;
}

/**
 * @brief Convert a signed 64-bit value to int32_t, trapping on overflow.
 */
ZLANG_STATIC_INLINE int32_t zlang_to_i32_i64(int64_t value) {
    if (value < (int64_t)INT32_MIN || value > (int64_t)INT32_MAX) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return (int32_t)value;
}

/**
 * @brief Convert an unsigned 32-bit value to int32_t, trapping on overflow.
 */
ZLANG_STATIC_INLINE int32_t zlang_to_i32_u32(uint32_t value) {
    if (value > (uint32_t)INT32_MAX) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return (int32_t)value;
}

/**
 * @brief Convert a floating value to int32_t, trapping on invalid input.
 */
ZLANG_STATIC_INLINE int32_t zlang_to_i32_f64(double value) {
    if (!isfinite(value) || value < (double)INT32_MIN || value > (double)INT32_MAX) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    return (int32_t)value;
}

/**
 * @brief Convert a signed 64-bit value to uint32_t, trapping on overflow.
 */
ZLANG_STATIC_INLINE uint32_t zlang_to_u32_i64(int64_t value) {
    if (value < 0 || value > (int64_t)UINT32_MAX) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return (uint32_t)value;
}

/**
 * @brief Convert an unsigned 32-bit value to uint32_t.
 */
ZLANG_STATIC_INLINE uint32_t zlang_to_u32_u32(uint32_t value) { return value; }

/**
 * @brief Convert a floating value to uint32_t, trapping on invalid input.
 */
ZLANG_STATIC_INLINE uint32_t zlang_to_u32_f64(double value) {
    if (!isfinite(value) || value < 0.0 || value > (double)UINT32_MAX) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    return (uint32_t)value;
}

/**
 * @brief Convert an unsigned 64-bit value to int32_t, trapping on overflow.
 */
ZLANG_STATIC_INLINE int32_t zlang_to_i32_u64(uint64_t value) {
    if (value > (uint64_t)INT32_MAX) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return (int32_t)value;
}

/**
 * @brief Convert an unsigned 64-bit value to uint32_t, trapping on overflow.
 */
ZLANG_STATIC_INLINE uint32_t zlang_to_u32_u64(uint64_t value) {
    if (value > (uint64_t)UINT32_MAX) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return (uint32_t)value;
}

/**
 * @brief Convert an unsigned 64-bit value to int64_t, trapping on overflow.
 */
ZLANG_STATIC_INLINE int64_t zlang_to_i64_u64(uint64_t value) {
    if (value > (uint64_t)INT64_MAX) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return (int64_t)value;
}

/**
 * @brief Convert a signed 64-bit value to uint64_t, trapping on overflow.
 */
ZLANG_STATIC_INLINE uint64_t zlang_to_u64_i64(int64_t value) {
    if (value < 0) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return (uint64_t)value;
}

/**
 * @brief Convert an unsigned 64-bit value to uint64_t.
 */
ZLANG_STATIC_INLINE uint64_t zlang_to_u64_u64(uint64_t value) { return value; }

/**
 * @brief Convert a floating value to uint64_t, trapping on invalid input.
 */
ZLANG_STATIC_INLINE uint64_t zlang_to_u64_f64(double value) {
    if (!isfinite(value) || value < 0.0 || value >= 18446744073709551616.0) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    return (uint64_t)value;
}

/**
 * @brief Convert a signed 64-bit value to int64_t, trapping on overflow.
 */
ZLANG_STATIC_INLINE int64_t zlang_to_i64_i64(int64_t value) { return value; }

/**
 * @brief Convert a floating value to int64_t, trapping on invalid input.
 */
ZLANG_STATIC_INLINE int64_t zlang_to_i64_f64(double value) {
    if (!isfinite(value) || value < -9223372036854775808.0 || value >= 9223372036854775808.0) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    return (int64_t)value;
}

/**
 * @brief Compare two string views by content.
 * @return -1 when left is byte-wise smaller, 0 when equal, 1 when greater.
 */
ZLANG_STATIC_INLINE int32_t zlang_string_equal_compare(zlang_string_t left, zlang_string_t right) {
    if ((left.data_ == NULL && left.length_ != 0U) ||
        (right.data_ == NULL && right.length_ != 0U)) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    const uint32_t _length_k = left.length_ < right.length_ ? left.length_ : right.length_;
    for (uint32_t _index = 0U; _index < _length_k; ++_index) {
        const uint8_t _left_k = (uint8_t)left.data_[_index];
        const uint8_t _right_k = (uint8_t)right.data_[_index];
        if (_left_k < _right_k) {
            return -1;
        }
        if (_left_k > _right_k) {
            return 1;
        }
    }
    if (left.length_ < right.length_) {
        return -1;
    }
    if (left.length_ > right.length_) {
        return 1;
    }
    return 0;
}

/**
 * @brief Test two string views for content equality.
 */
ZLANG_STATIC_INLINE bool zlang_string_equal(zlang_string_t left, zlang_string_t right) {
    return zlang_string_equal_compare(left, right) == 0;
}

/**
 * @brief Concatenate string views into caller-owned storage.
 * @param storage Destination buffer with room for `capacity + 1` bytes.
 * @param capacity Maximum number of content bytes, excluding the terminator.
 * @param parts String views to concatenate in order.
 * @param count Number of views.
 * @return A view of `storage` containing the concatenated bytes.
 *
 * Parts are copied from last to first so a caller-owned buffer may also be one
 * of the inputs. Overflow and insufficient capacity trap instead of growing.
 */
ZLANG_STATIC_INLINE zlang_string_t zlang_string_concat(char *storage, uint32_t capacity,
                                                       const zlang_string_t *parts, size_t count) {
    size_t _total_k = 0U;
    for (size_t _index = 0U; _index < count; ++_index) {
        if (parts == NULL || (parts[_index].data_ == NULL && parts[_index].length_ != 0U)) {
            zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
            return (zlang_string_t){NULL, 0U};
        }
        if ((size_t)parts[_index].length_ > (size_t)(UINT32_MAX - (uint32_t)_total_k)) {
            zlang_trap(ZLANG_TRAP_OVERFLOW);
            return (zlang_string_t){NULL, 0U};
        }
        _total_k += (size_t)parts[_index].length_;
    }
    if (_total_k > (size_t)capacity) {
        zlang_trap(ZLANG_TRAP_CAPACITY);
        return (zlang_string_t){NULL, 0U};
    }
    if (_total_k != 0U && storage == NULL) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
        return (zlang_string_t){NULL, 0U};
    }

    size_t _offset_k = _total_k;
    for (size_t _index = count; _index > 0U; --_index) {
        const zlang_string_t _part_k = parts[_index - 1U];
        _offset_k -= (size_t)_part_k.length_;
        if (_part_k.length_ != 0U) {
            memmove(storage + _offset_k, _part_k.data_, (size_t)_part_k.length_);
        }
    }
    if (storage != NULL) {
        storage[_total_k] = '\0';
    }
    return (zlang_string_t){storage, (uint32_t)_total_k};
}

/**
 * @brief Append a bounded string to caller-owned sequence storage.
 * @param storage Flat storage with `capacity * (element_capacity + 1)` bytes.
 * @param data String views updated with the stored element.
 * @param length Current element count; updated on success.
 * @param capacity Maximum element count.
 * @param element_capacity Content capacity of one element.
 * @param parts String views to concatenate into the new element.
 * @param count Number of views.
 * @return The stored string view.
 */
ZLANG_STATIC_INLINE zlang_string_t zlang_string_array_append(char *storage, zlang_string_t *data,
                                                             uint32_t *length, uint32_t capacity,
                                                             uint32_t element_capacity,
                                                             const zlang_string_t *parts,
                                                             size_t count) {
    if (storage == NULL || data == NULL || length == NULL || parts == NULL || *length >= capacity ||
        element_capacity == UINT32_MAX || capacity > UINT32_MAX / (element_capacity + 1U)) {
        zlang_trap(ZLANG_TRAP_CAPACITY);
        return (zlang_string_t){NULL, 0U};
    }
    char *const _slot_k = storage + ((size_t)(*length) * (size_t)(element_capacity + 1U));
    const zlang_string_t _stored_k = zlang_string_concat(_slot_k, element_capacity, parts, count);
    data[*length] = _stored_k;
    ++(*length);
    return _stored_k;
}

/**
 * @brief Read an element pointer with a bounds trap.
 */
ZLANG_STATIC_INLINE void *zlang_array_get(void *data, uint32_t index, uint32_t length,
                                          size_t element_size) {
    if (data == NULL || index >= length || element_size == 0U) {
        zlang_trap(ZLANG_TRAP_BOUNDS);
        return NULL;
    }
    return (char *)data + ((size_t)index * element_size);
}

/**
 * @brief Read a signed integer element with a bounds trap.
 */
ZLANG_STATIC_INLINE int32_t zlang_array_get_i32(const int32_t *data, uint32_t index,
                                                uint32_t length) {
    if (data == NULL || index >= length) {
        zlang_trap(ZLANG_TRAP_BOUNDS);
        return 0;
    }
    return data[index];
}

/**
 * @brief Read an unsigned integer element with a bounds trap.
 */
ZLANG_STATIC_INLINE uint32_t zlang_array_get_u32(const uint32_t *data, uint32_t index,
                                                 uint32_t length) {
    if (data == NULL || index >= length) {
        zlang_trap(ZLANG_TRAP_BOUNDS);
        return 0U;
    }
    return data[index];
}

/**
 * @brief Read a boolean element with a bounds trap.
 */
ZLANG_STATIC_INLINE bool zlang_array_get_bool(const bool *data, uint32_t index, uint32_t length) {
    if (data == NULL || index >= length) {
        zlang_trap(ZLANG_TRAP_BOUNDS);
        return false;
    }
    return data[index];
}

/**
 * @brief Read a bounded string element with a bounds trap.
 */
ZLANG_STATIC_INLINE zlang_string_t zlang_array_get_string(const zlang_string_t *data,
                                                          uint32_t index, uint32_t length) {
    const zlang_string_t _empty_k = {NULL, 0U};
    if (data == NULL || index >= length) {
        zlang_trap(ZLANG_TRAP_BOUNDS);
        return _empty_k;
    }
    return data[index];
}

/**
 * @brief Read one byte of a bounded string with a bounds trap.
 */
ZLANG_STATIC_INLINE uint32_t zlang_string_byte(zlang_string_t value, uint32_t index) {
    if (value.data_ == NULL || index >= value.length_) {
        zlang_trap(ZLANG_TRAP_BOUNDS);
        return 0U;
    }
    return (uint32_t)(unsigned char)value.data_[index];
}

/**
 * @brief Append one element to a fixed-capacity sequence, trapping on overflow.
 * @param data Sequence storage.
 * @param length Current element count; updated on success.
 * @param capacity Maximum element count.
 * @param element_size Size of one element in bytes.
 * @param value Element to copy from.
 */
ZLANG_STATIC_INLINE void zlang_array_append(void *data, uint32_t *length, uint32_t capacity,
                                            size_t element_size, const void *value) {
    if (data == NULL || length == NULL || value == NULL || element_size == 0U ||
        *length >= capacity) {
        zlang_trap(ZLANG_TRAP_CAPACITY);
        return;
    }
    memcpy((char *)data + ((size_t)(*length) * element_size), value, element_size);
    ++(*length);
}

#if defined(ZLANG_STD_RANDOM_GENERATE_INT32) || defined(ZLANG_STD_RANDOM_GENERATE_UINT32) ||       \
    defined(ZLANG_STD_RANDOM_GENERATE_INT64) || defined(ZLANG_STD_RANDOM_GENERATE_UINT64) ||       \
    defined(ZLANG_STD_RANDOM_GENERATE_FLOAT32) || defined(ZLANG_STD_RANDOM_GENERATE_FLOAT64)
#include "zlang/standard/random/state.h"
#endif

#if defined(ZLANG_STD_RANDOM_GENERATE_INT32)
#include "zlang/standard/random/generate_int32.h"
#endif
#if defined(ZLANG_STD_RANDOM_GENERATE_UINT32)
#include "zlang/standard/random/generate_uint32.h"
#endif
#if defined(ZLANG_STD_RANDOM_GENERATE_INT64)
#include "zlang/standard/random/generate_int64.h"
#endif
#if defined(ZLANG_STD_RANDOM_GENERATE_UINT64)
#include "zlang/standard/random/generate_uint64.h"
#endif
#if defined(ZLANG_STD_RANDOM_GENERATE_FLOAT32)
#include "zlang/standard/random/generate_float32.h"
#endif
#if defined(ZLANG_STD_RANDOM_GENERATE_FLOAT64)
#include "zlang/standard/random/generate_float64.h"
#endif

#if defined(ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT) ||                                           \
    defined(ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_AT) ||                                              \
    defined(ZLANG_STD_PROGRAM_OPTIONS_HAS_OPTION) ||                                               \
    defined(ZLANG_STD_PROGRAM_OPTIONS_OPTION_VALUE) || defined(ZLANG_STD_PROGRAM_OPTIONS_HAS_FLAG)
#include "zlang/standard/program/options.h"
#endif

#if defined(ZLANG_STD_IO_WRITE_STRING) || defined(ZLANG_STD_IO_WRITE_BOOL) ||                      \
    defined(ZLANG_STD_IO_WRITE_I32) || defined(ZLANG_STD_IO_WRITE_U32) ||                          \
    defined(ZLANG_STD_IO_WRITE_I64) || defined(ZLANG_STD_IO_WRITE_U64) ||                          \
    defined(ZLANG_STD_IO_WRITE_F32) || defined(ZLANG_STD_IO_WRITE_F64)
#include "zlang/standard/io/io.h"
#endif

#if defined(ZLANG_STD_MATH_ABS_I32) || defined(ZLANG_STD_MATH_ABS_U32) ||                          \
    defined(ZLANG_STD_MATH_ABS_I64) || defined(ZLANG_STD_MATH_ABS_U64) ||                          \
    defined(ZLANG_STD_MATH_ABS_F32) || defined(ZLANG_STD_MATH_ABS_F64) ||                          \
    defined(ZLANG_STD_MATH_CLAMP_I32) || defined(ZLANG_STD_MATH_CLAMP_U32) ||                      \
    defined(ZLANG_STD_MATH_CLAMP_I64) || defined(ZLANG_STD_MATH_CLAMP_U64) ||                      \
    defined(ZLANG_STD_MATH_CLAMP_F32) || defined(ZLANG_STD_MATH_CLAMP_F64) ||                      \
    defined(ZLANG_STD_MATH_MIN_I32) || defined(ZLANG_STD_MATH_MIN_U32) ||                          \
    defined(ZLANG_STD_MATH_MIN_I64) || defined(ZLANG_STD_MATH_MIN_U64) ||                          \
    defined(ZLANG_STD_MATH_MIN_F32) || defined(ZLANG_STD_MATH_MIN_F64) ||                          \
    defined(ZLANG_STD_MATH_MAX_I32) || defined(ZLANG_STD_MATH_MAX_U32) ||                          \
    defined(ZLANG_STD_MATH_MAX_I64) || defined(ZLANG_STD_MATH_MAX_U64) ||                          \
    defined(ZLANG_STD_MATH_MAX_F32) || defined(ZLANG_STD_MATH_MAX_F64) ||                          \
    defined(ZLANG_STD_MATH_SQRT_F32) || defined(ZLANG_STD_MATH_SQRT_F64)
#include "zlang/standard/math/math.h"
#endif

#if defined(ZLANG_STD_STRINGS_COMPARE) || defined(ZLANG_STD_STRINGS_CONTAINS) ||                   \
    defined(ZLANG_STD_STRINGS_STARTS_WITH)
#include "zlang/standard/strings/strings.h"
#endif

#endif /* ZLANG_RUNTIME_RUNTIME_H */
