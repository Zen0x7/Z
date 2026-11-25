/** @file
 * @brief Semantic analysis API for Zlang.
 */

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
    const zlang_ast_statement_t *declaration_;
    size_t scope_depth_;
    size_t parameter_start_;
    size_t parameter_count_;
    uint32_t capacity_;
    uint32_t extents_[ZLANG_ARRAY_MAX_RANK];
    uint32_t rank_;
    bool is_sequence_;
    bool is_function_;
    bool has_explicit_capacity_;
    bool is_mutated_;
    bool is_const_;
    bool is_reference_;
} zlang_symbol_t;

typedef struct zlang_semantic_context_t {
    zlang_symbol_t *symbols_;
    size_t symbol_capacity_;
    size_t symbol_count_;
    size_t scope_depth_;
    size_t loop_depth_;
    const char *current_return_type_;
    bool has_error_;
    zlang_diagnostics_t *diagnostics_;
    const zlang_program_t *program_;
    const zlang_ast_statement_t *current_struct_;
} zlang_semantic_context_t;

void zlang_semantic_context_init(zlang_semantic_context_t *context, zlang_symbol_t *symbols,
                                 size_t symbol_capacity, zlang_diagnostics_t *diagnostics,
                                 const zlang_program_t *program);

/* Checks declarations, identifier use, and bounded sequence capacity before
   emission. Returns false when any semantic diagnostic was added. */
bool zlang_analyze_program(const zlang_program_t *program, zlang_semantic_context_t *context);

#endif /* ZLANG_COMPILER_SEMANTICS_H */
