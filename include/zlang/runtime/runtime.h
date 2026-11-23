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

/* Fixed trap codes published in SPEC.md. Generated code never formats an
   error string at runtime. */
#define ZLANG_TRAP_OVERFLOW 1U
#define ZLANG_TRAP_BOUNDS 2U
#define ZLANG_TRAP_CAPACITY 3U
#define ZLANG_TRAP_INVALID_ARGUMENT 4U

typedef struct zlang_string_t {
    const char *data_;
    uint32_t length_;
} zlang_string_t;

ZLANG_STATIC_INLINE bool zlang_write(FILE *stream, zlang_string_t value) {
    if (stream == NULL || (value.data_ == NULL && value.length_ != 0U)) {
        return false;
    }
    if (value.length_ == 0U) {
        return true;
    }
    return fwrite(value.data_, 1U, value.length_, stream) == value.length_;
}

ZLANG_STATIC_INLINE bool zlang_print(zlang_string_t value) { return zlang_write(stdout, value); }

ZLANG_STATIC_INLINE void zlang_trap(uint8_t code) {
    (void)code;
    abort();
}

ZLANG_STATIC_INLINE int32_t zlang_add_i32(int32_t left, int32_t right) {
    const bool _overflow_k =
        (right > 0 && left > INT32_MAX - right) || (right < 0 && left < INT32_MIN - right);
    if (_overflow_k) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return left + right;
}

ZLANG_STATIC_INLINE uint32_t zlang_add_u32(uint32_t left, uint32_t right) {
    if (right > UINT32_MAX - left) {
        zlang_trap(ZLANG_TRAP_OVERFLOW);
    }
    return left + right;
}

ZLANG_STATIC_INLINE void zlang_array_append(void *data, uint32_t *length, uint32_t capacity,
                                            size_t element_size, const void *value) {
    if (data == NULL || length == NULL || value == NULL || element_size == 0U ||
        *length >= capacity) {
        zlang_trap(ZLANG_TRAP_CAPACITY);
    }
    memcpy((char *)data + ((size_t)(*length) * element_size), value, element_size);
    ++(*length);
}

#endif /* ZLANG_RUNTIME_RUNTIME_H */
