#ifndef ZLANG_FUZZ_SUPPORT_H
#define ZLANG_FUZZ_SUPPORT_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Fuzz input is copied into a bounded, NUL-terminated buffer because the
 * compiler front end consumes ordinary C strings. */

#define ZLANG_FUZZ_SOURCE_CAPACITY 512U

static void zlang_fuzz_source(char *buffer, size_t capacity, const uint8_t *data, size_t size) {
    const size_t _copy_k = size < capacity - 1U ? size : capacity - 1U;

    if (_copy_k != 0U && data != NULL) {
        memcpy(buffer, data, _copy_k);
    }
    buffer[_copy_k] = '\0';
}

#endif /* ZLANG_FUZZ_SUPPORT_H */
