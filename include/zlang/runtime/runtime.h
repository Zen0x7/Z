#ifndef ZLANG_RUNTIME_RUNTIME_H
#define ZLANG_RUNTIME_RUNTIME_H

#include "zlang/detail/inline.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
#define ZLANG_TRAP_SINK(code) abort()
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
