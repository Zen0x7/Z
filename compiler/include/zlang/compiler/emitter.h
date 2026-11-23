/** @file
 * @brief C99 source emitter API.
 */

#ifndef ZLANG_COMPILER_EMITTER_H
#define ZLANG_COMPILER_EMITTER_H

#include "zlang/compiler/ast.h"
#include "zlang/compiler/diagnostics.h"
#include "zlang/compiler/planner.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ZLANG_EMITTED_TYPE_CAPACITY 32U

/* The emitter owns no memory. The caller supplies the output buffer and the
   diagnostic sink, which keeps the compiler core free of hidden state. */

typedef struct zlang_emitter_t {
    char *buffer_;
    size_t capacity_;
    size_t length_;
    zlang_diagnostics_t *diagnostics_;
    const zlang_memory_plan_t *plan_;
    uint32_t next_index_;
    char emitted_types_[ZLANG_EMITTED_TYPE_CAPACITY][64];
    size_t emitted_type_count_;
} zlang_emitter_t;

void zlang_emitter_init(zlang_emitter_t *emitter, char *buffer, size_t capacity,
                        zlang_diagnostics_t *diagnostics, const zlang_memory_plan_t *plan);

bool zlang_emit_program(const zlang_program_t *program, zlang_emitter_t *emitter);

#endif /* ZLANG_COMPILER_EMITTER_H */
