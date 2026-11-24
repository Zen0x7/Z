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

#define ZLANG_EMITTED_SYMBOL_CAPACITY 128U
#define ZLANG_EMITTED_SEQUENCE_CAPACITY 32U
#define ZLANG_EMITTED_LITERAL_CAPACITY 64U
#define ZLANG_EMITTED_NAME_CAPACITY 64U

/* The emitter owns no memory. The caller supplies the output buffer and the
   diagnostic sink, which keeps the compiler core free of hidden state. */

typedef struct zlang_emitted_symbol_t {
    const char *source_name_;
    const char *type_name_;
    char generated_name_[ZLANG_EMITTED_NAME_CAPACITY];
    char storage_name_[ZLANG_EMITTED_NAME_CAPACITY];
    uint32_t capacity_;
    bool has_explicit_capacity_;
    bool is_sequence_;
} zlang_emitted_symbol_t;

typedef struct zlang_emitted_sequence_t {
    const zlang_ast_statement_t *statement_;
    char type_name_[ZLANG_EMITTED_NAME_CAPACITY];
    char capacity_macro_[ZLANG_EMITTED_NAME_CAPACITY];
    char element_macro_[ZLANG_EMITTED_NAME_CAPACITY];
} zlang_emitted_sequence_t;

typedef struct zlang_emitted_literal_t {
    size_t expression_index_;
    char data_name_[ZLANG_EMITTED_NAME_CAPACITY];
    char view_name_[ZLANG_EMITTED_NAME_CAPACITY];
} zlang_emitted_literal_t;

typedef struct zlang_emitter_t {
    char *buffer_;
    size_t capacity_;
    size_t length_;
    zlang_diagnostics_t *diagnostics_;
    const zlang_memory_plan_t *plan_;
    uint32_t next_index_;
    zlang_emitted_symbol_t emitted_symbols_[ZLANG_EMITTED_SYMBOL_CAPACITY];
    size_t emitted_symbol_count_;
    size_t active_symbol_count_;
    zlang_emitted_sequence_t emitted_sequences_[ZLANG_EMITTED_SEQUENCE_CAPACITY];
    size_t emitted_sequence_count_;
    zlang_emitted_literal_t emitted_literals_[ZLANG_EMITTED_LITERAL_CAPACITY];
    size_t emitted_literal_count_;
} zlang_emitter_t;

void zlang_emitter_init(zlang_emitter_t *emitter, char *buffer, size_t capacity,
                        zlang_diagnostics_t *diagnostics, const zlang_memory_plan_t *plan);

bool zlang_emit_program(const zlang_program_t *program, zlang_emitter_t *emitter);

#endif /* ZLANG_COMPILER_EMITTER_H */
