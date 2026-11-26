#ifndef ZLANG_RUNTIME_CONTRACT_H
#define ZLANG_RUNTIME_CONTRACT_H

#include "zlang/detail/inline.h"

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>

/** @file
 * @brief Low-level runtime primitives shared by headers that cannot include runtime.h.
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

#ifndef ZLANG_TRAP_SINK
#define ZLANG_TRAP_SINK(code) ((void)(code), (void)raise(SIGABRT))
#endif

ZLANG_STATIC_INLINE void zlang_trap(const uint8_t code) {
    (void)code;
    ZLANG_TRAP_SINK(code);
}

#endif /* ZLANG_RUNTIME_CONTRACT_H */
