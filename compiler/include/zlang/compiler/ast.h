#ifndef ZLANG_COMPILER_AST_H
#define ZLANG_COMPILER_AST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* The AST is deliberately inert data. It carries no ownership and no source
   text, so a parser can fill it in without pulling in the emitter. */

typedef enum zlang_ast_expression_kind_t {
    zlang_ast_expression_integer_literal_,
    zlang_ast_expression_string_literal_,
    zlang_ast_expression_identifier_
} zlang_ast_expression_kind_t;

typedef struct zlang_ast_string_value_t {
    const char *data_;
    uint32_t length_;
} zlang_ast_string_value_t;

typedef struct zlang_ast_expression_t {
    zlang_ast_expression_kind_t kind_;
    int32_t integer_value_;
    bool is_unsigned_;
    zlang_ast_string_value_t string_;
    const char *name_;
} zlang_ast_expression_t;

typedef enum zlang_ast_statement_kind_t {
    zlang_ast_statement_declaration_,
    zlang_ast_statement_sequence_declaration_,
    zlang_ast_statement_print_,
    zlang_ast_statement_foreach_
} zlang_ast_statement_kind_t;

struct zlang_ast_statement_t;

typedef struct zlang_ast_statement_t {
    zlang_ast_statement_kind_t kind_;
    const char *type_name_;
    const char *name_;
    const zlang_ast_expression_t *initializer_;
    uint32_t capacity_;
    const char *iterable_name_;
    const struct zlang_ast_statement_t *body_;
    size_t body_count_;
    uint32_t line_;
    uint32_t column_;
} zlang_ast_statement_t;

typedef struct zlang_program_t {
    const zlang_ast_statement_t *statements_;
    size_t statement_count_;
} zlang_program_t;

#endif /* ZLANG_COMPILER_AST_H */
