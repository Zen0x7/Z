#ifndef ZLANG_COMPILER_SEMANTICS_H
#define ZLANG_COMPILER_SEMANTICS_H

#include "zlang/compiler/ast.h"
#include "zlang/compiler/diagnostics.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct zlang_symbol_t {
    const char *name_;
    const char *type_name_;
    uint32_t capacity_;
    bool is_sequence_;
} zlang_symbol_t;

typedef struct zlang_semantic_context_t {
    zlang_symbol_t *symbols_;
    size_t symbol_capacity_;
    size_t symbol_count_;
    zlang_diagnostics_t *diagnostics_;
} zlang_semantic_context_t;

void zlang_semantic_context_init(zlang_semantic_context_t *context, zlang_symbol_t *symbols,
                                 size_t symbol_capacity, zlang_diagnostics_t *diagnostics);

/* Checks declarations, identifier use, and bounded sequence capacity before
   emission. Returns false when any semantic diagnostic was added. */
bool zlang_analyze_program(const zlang_program_t *program, zlang_semantic_context_t *context);

#endif /* ZLANG_COMPILER_SEMANTICS_H */
