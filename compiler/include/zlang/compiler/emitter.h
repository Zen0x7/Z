#ifndef ZLANG_COMPILER_EMITTER_H
#define ZLANG_COMPILER_EMITTER_H

#include "zlang/compiler/ast.h"
#include "zlang/compiler/diagnostics.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* The emitter owns no memory. The caller supplies the output buffer and the
   diagnostic sink, which keeps the compiler core free of hidden state. */

typedef struct zlang_emitter_t {
    char *buffer_;
    size_t capacity_;
    size_t length_;
    zlang_diagnostics_t *diagnostics_;
    uint32_t next_index_;
} zlang_emitter_t;

void zlang_emitter_init(zlang_emitter_t *emitter, char *buffer, size_t capacity,
                        zlang_diagnostics_t *diagnostics);

bool zlang_emit_program(const zlang_program_t *program, zlang_emitter_t *emitter);

#endif /* ZLANG_COMPILER_EMITTER_H */
