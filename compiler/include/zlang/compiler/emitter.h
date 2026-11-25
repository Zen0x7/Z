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
#define ZLANG_EMITTED_USER_TYPE_CAPACITY 32U
#define ZLANG_EMITTED_ENUM_ITEM_CAPACITY 128U
#define ZLANG_EMITTED_COMMENT_CAPACITY 128U
#define ZLANG_EMITTED_CSE_CAPACITY 16U
#define ZLANG_MAX_ARRAY_SPECIALIZATIONS 16U
#define ZLANG_EMITTED_ARRAY_HELPER_CAPACITY 64U
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
    bool is_reference_;
    bool is_array_;
    uint32_t array_rank_;
    uint32_t array_extents_[ZLANG_ARRAY_MAX_RANK];
} zlang_emitted_symbol_t;

typedef struct zlang_emitted_sequence_t {
    const zlang_ast_statement_t *statement_;
    char type_name_[ZLANG_EMITTED_NAME_CAPACITY];
    char capacity_macro_[ZLANG_EMITTED_NAME_CAPACITY];
    char element_macro_[ZLANG_EMITTED_NAME_CAPACITY];
    char length_macro_[ZLANG_EMITTED_NAME_CAPACITY];
    char extent_macros_[ZLANG_ARRAY_MAX_RANK][ZLANG_EMITTED_NAME_CAPACITY];
} zlang_emitted_sequence_t;

typedef struct zlang_emitted_literal_t {
    size_t expression_index_;
    size_t primary_expression_index_;
    char data_name_[ZLANG_EMITTED_NAME_CAPACITY];
    char view_name_[ZLANG_EMITTED_NAME_CAPACITY];
    char length_macro_[ZLANG_EMITTED_NAME_CAPACITY];
    bool is_primary_;
} zlang_emitted_literal_t;

typedef struct zlang_emitted_capacity_t {
    const zlang_ast_statement_t *statement_;
    char macro_[ZLANG_EMITTED_NAME_CAPACITY];
} zlang_emitted_capacity_t;

typedef struct zlang_emitted_user_type_t {
    const zlang_ast_statement_t *statement_;
    char type_name_[ZLANG_EMITTED_NAME_CAPACITY];
} zlang_emitted_user_type_t;

typedef struct zlang_emitted_enum_item_t {
    const char *source_name_;
    const char *type_name_;
    char generated_name_[ZLANG_EMITTED_NAME_CAPACITY];
} zlang_emitted_enum_item_t;

typedef struct zlang_array_specialization_t {
    size_t function_statement_;
    const char *type_name_;
    uint32_t rank_;
    uint32_t extents_[ZLANG_ARRAY_MAX_RANK];
} zlang_array_specialization_t;

typedef struct zlang_emitted_array_helper_t {
    char name_[ZLANG_EMITTED_NAME_CAPACITY];
    char type_name_[ZLANG_EMITTED_NAME_CAPACITY];
    uint32_t rank_;
    bool is_set_;
} zlang_emitted_array_helper_t;

typedef struct zlang_emitted_function_t {
    const zlang_ast_statement_t *statement_;
    const zlang_array_specialization_t *specialization_;
    char generated_name_[ZLANG_EMITTED_NAME_CAPACITY];
    bool needs_state_;
} zlang_emitted_function_t;

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
    zlang_emitted_capacity_t emitted_capacities_[ZLANG_EMITTED_SEQUENCE_CAPACITY];
    size_t emitted_capacity_count_;
    zlang_emitted_user_type_t emitted_user_types_[ZLANG_EMITTED_USER_TYPE_CAPACITY];
    size_t emitted_user_type_count_;
    zlang_emitted_enum_item_t emitted_enum_items_[ZLANG_EMITTED_ENUM_ITEM_CAPACITY];
    size_t emitted_enum_item_count_;
    size_t cse_expression_indices_[ZLANG_EMITTED_CSE_CAPACITY];
    char cse_names_[ZLANG_EMITTED_CSE_CAPACITY][ZLANG_EMITTED_NAME_CAPACITY];
    size_t cse_count_;
    size_t cse_pointer_indices_[ZLANG_EMITTED_CSE_CAPACITY];
    char cse_pointer_names_[ZLANG_EMITTED_CSE_CAPACITY][ZLANG_EMITTED_NAME_CAPACITY];
    const char *cse_pointer_base_names_[ZLANG_EMITTED_CSE_CAPACITY];
    const char *cse_pointer_dependency_names_[ZLANG_EMITTED_CSE_CAPACITY];
    size_t cse_pointer_count_;
    bool emitted_comments_[ZLANG_EMITTED_COMMENT_CAPACITY];
    bool emit_comments_;
    zlang_emitted_array_helper_t requested_array_helpers_[ZLANG_EMITTED_ARRAY_HELPER_CAPACITY];
    size_t requested_array_helper_count_;
    bool collecting_array_helpers_;
    zlang_emitted_function_t emitted_functions_[ZLANG_EMITTED_SYMBOL_CAPACITY];
    size_t emitted_function_count_;
    zlang_array_specialization_t array_specializations_[ZLANG_MAX_ARRAY_SPECIALIZATIONS];
    size_t array_specialization_count_;
    const zlang_array_specialization_t *active_specialization_;
    bool uses_stateful_std_;
    bool in_function_;
    bool has_explicit_main_;
    const zlang_program_t *program_;
    const zlang_ast_statement_t *current_struct_;
    const char *current_self_name_;
    bool current_self_is_pointer_;
    char activated_macros_[ZLANG_EMITTED_SYMBOL_CAPACITY][ZLANG_EMITTED_NAME_CAPACITY];
    size_t activated_macro_count_;
} zlang_emitter_t;

void zlang_emitter_init(zlang_emitter_t *emitter, char *buffer, size_t capacity,
                        zlang_diagnostics_t *diagnostics, const zlang_memory_plan_t *plan);

bool zlang_emit_program(const zlang_program_t *program, zlang_emitter_t *emitter);

#endif /* ZLANG_COMPILER_EMITTER_H */
