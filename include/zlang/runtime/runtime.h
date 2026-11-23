#ifndef ZLANG_RUNTIME_RUNTIME_H
#define ZLANG_RUNTIME_RUNTIME_H

#include "zlang/detail/inline.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

#endif /* ZLANG_RUNTIME_RUNTIME_H */
