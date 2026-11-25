#ifndef ZLANG_RUNTIME_CONTEXT_H
#define ZLANG_RUNTIME_CONTEXT_H

#include <stddef.h>
#include <stdint.h>

/** Internal generated-C state shared by stateful standard functions. */
typedef struct zlang_runtime_context_t {
    uint32_t random_state_;
} zlang_runtime_context_t;

/** Initialize runtime context with the deterministic default PRNG seed. */
static inline void zlang_runtime_context_init(zlang_runtime_context_t *context) {
    if (context != NULL) {
        context->random_state_ = 0x6D2B79F5U;
    }
}

#endif /* ZLANG_RUNTIME_CONTEXT_H */
