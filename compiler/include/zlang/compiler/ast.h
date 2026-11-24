/** @file
 * @brief Flat abstract syntax tree used by the compiler.
 */

#ifndef ZLANG_COMPILER_AST_H
#define ZLANG_COMPILER_AST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* The AST is deliberately inert data. It carries no ownership and no source
   text, so a parser can fill it in without pulling in the emitter. Expressions
   and statements refer to each other by index into caller-owned arrays. */

#define ZLANG_NO_EXPRESSION ((size_t) - 1)

typedef enum zlang_expression_kind_t {
    zlang_expression_integer_literal_,
    zlang_expression_boolean_literal_,
    zlang_expression_string_literal_,
    zlang_expression_identifier_,
    zlang_expression_add_,
    zlang_expression_equal_,
    zlang_expression_assignment_,
    zlang_expression_print_call_,
    zlang_expression_append_call_,
    zlang_expression_index_,
    zlang_expression_index_assignment_,
    zlang_expression_string_concat_,
    zlang_expression_call_,
    zlang_expression_less_,
    zlang_expression_greater_,
    zlang_expression_less_equal_,
    zlang_expression_greater_equal_,
    zlang_expression_not_equal_
} zlang_expression_kind_t;

typedef struct zlang_ast_string_value_t {
    const char *data_;
    uint32_t length_;
} zlang_ast_string_value_t;

typedef struct zlang_ast_expression_t {
    zlang_expression_kind_t kind_;
    int32_t integer_value_;
    bool bool_value_;
    bool is_unsigned_;
    bool is_string_;
    size_t left_;
    size_t right_;
    size_t argument_start_;
    size_t argument_count_;
    zlang_ast_string_value_t string_;
    const char *name_;
    uint32_t line_;
    uint32_t column_;
} zlang_ast_expression_t;

typedef enum zlang_statement_kind_t {
    zlang_statement_declaration_,
    zlang_statement_sequence_declaration_,
    zlang_statement_expression_,
    zlang_statement_if_,
    zlang_statement_while_,
    zlang_statement_foreach_,
    zlang_statement_repeat_,
    zlang_statement_function_,
    zlang_statement_return_,
    zlang_statement_block_
} zlang_statement_kind_t;

typedef struct zlang_parameter_t {
    const char *type_name_;
    const char *name_;
    uint32_t line_;
    uint32_t column_;
} zlang_parameter_t;

typedef struct zlang_ast_statement_t {
    const char *type_name_;
    const char *name_;
    const char *iterable_name_;
    size_t expression_;
    size_t body_start_;
    size_t body_count_;
    size_t else_start_;
    size_t else_count_;
    size_t parameter_start_;
    size_t parameter_count_;
    size_t initializer_start_;
    size_t initializer_count_;
    zlang_statement_kind_t kind_;
    uint32_t capacity_;
    bool has_explicit_capacity_;
    uint32_t line_;
    uint32_t column_;
} zlang_ast_statement_t;

typedef struct zlang_program_t {
    const zlang_ast_statement_t *statements_;
    size_t statement_count_;
    const zlang_ast_expression_t *expressions_;
    size_t expression_count_;
    const zlang_parameter_t *parameters_;
    size_t parameter_count_;
} zlang_program_t;

#endif /* ZLANG_COMPILER_AST_H */
