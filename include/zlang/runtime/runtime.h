#ifndef ZLANG_RUNTIME_RUNTIME_H
#define ZLANG_RUNTIME_RUNTIME_H

#include "zlang/detail/inline.h"

#include <limits.h>
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
    return left / right;
}

/**
 * @brief Divide two unsigned integers, trapping on zero.
 */
ZLANG_STATIC_INLINE uint32_t zlang_div_u32(uint32_t left, uint32_t right) {
    if (right == 0U) {
        zlang_trap(ZLANG_TRAP_DIVISION_BY_ZERO);
    }
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
    return left % right;
}

/**
 * @brief Remainder of two unsigned integers, trapping on zero.
 */
ZLANG_STATIC_INLINE uint32_t zlang_mod_u32(uint32_t left, uint32_t right) {
    if (right == 0U) {
        zlang_trap(ZLANG_TRAP_DIVISION_BY_ZERO);
    }
    return left % right;
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

#endif /* ZLANG_RUNTIME_RUNTIME_H */
