/** @file
 * @brief Token definitions and the Zlang lexer API.
 */

#ifndef ZLANG_COMPILER_LEXER_H
#define ZLANG_COMPILER_LEXER_H

#include "zlang/compiler/ast.h"
#include "zlang/compiler/diagnostics.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum zlang_token_kind_t {
    zlang_token_end_of_file_,
    zlang_token_identifier_,
    zlang_token_integer_literal_,
    zlang_token_float_literal_,
    zlang_token_string_literal_,
    zlang_token_boolean_,
    zlang_token_clone_,
    zlang_token_const_,
    zlang_token_else_,
    zlang_token_enum_,
    zlang_token_false_,
    zlang_token_for_,
    zlang_token_from_,
    zlang_token_foreach_,
    zlang_token_function_,
    zlang_token_import_,
    zlang_token_if_,
    zlang_token_in_,
    zlang_token_integer_,
    zlang_token_int64_,
    zlang_token_uint64_,
    zlang_token_float_,
    zlang_token_double_,
    zlang_token_constructor_,
    zlang_token_static_,
    zlang_token_struct_,
    zlang_token_trait_,
    zlang_token_uses_,
    zlang_token_requires_,
    zlang_token_reference_,
    zlang_token_repeat_,
    zlang_token_return_,
    zlang_token_length_,
    zlang_token_array_,
    zlang_token_arguments_,
    zlang_token_string_,
    zlang_token_true_,
    zlang_token_unsigned_,
    zlang_token_while_,
    zlang_token_break_,
    zlang_token_continue_,
    zlang_token_jump_,
    zlang_token_left_paren_,
    zlang_token_right_paren_,
    zlang_token_left_brace_,
    zlang_token_right_brace_,
    zlang_token_left_bracket_,
    zlang_token_right_bracket_,
    zlang_token_semicolon_,
    zlang_token_comma_,
    zlang_token_colon_,
    zlang_token_dot_,
    zlang_token_equal_,
    zlang_token_equal_equal_,
    zlang_token_not_equal_,
    zlang_token_less_,
    zlang_token_greater_,
    zlang_token_less_equal_,
    zlang_token_greater_equal_,
    zlang_token_not_,
    zlang_token_and_,
    zlang_token_or_,
    zlang_token_plus_,
    zlang_token_minus_,
    zlang_token_star_,
    zlang_token_slash_,
    zlang_token_percent_,
    zlang_token_plus_equal_,
    zlang_token_minus_equal_,
    zlang_token_star_equal_,
    zlang_token_slash_equal_,
    zlang_token_percent_equal_
} zlang_token_kind_t;

// NOLINTNEXTLINE(clang-analyzer-optin.performance.Padding)
typedef struct zlang_token_t {
    const char *text_;
    uint64_t integer64_value_;
    double float_value_;
    size_t offset_;
    zlang_token_kind_t kind_;
    uint32_t length_;
    uint32_t value_length_;
    int32_t integer_value_;
    uint32_t line_;
    uint32_t column_;
    bool is_unsigned_;
    bool is_int64_;
    bool is_uint64_;
    bool is_float_;
} zlang_token_t;

typedef struct zlang_lexer_t {
    const char *source_;
    char *scratch_;
    size_t scratch_capacity_;
    size_t scratch_length_;
    size_t position_;
    uint32_t line_;
    uint32_t column_;
    zlang_comment_t *comments_;
    size_t comment_capacity_;
    size_t comment_count_;
} zlang_lexer_t;

void zlang_lexer_init(zlang_lexer_t *lexer, const char *source, char *scratch,
                      size_t scratch_capacity, zlang_comment_t *comments, size_t comment_capacity);

/* Fills tokens with lexical elements and appends diagnostics for invalid
   input. Returns false when a fatal lexical error occurred. */
bool zlang_lex(zlang_lexer_t *lexer, zlang_token_t *tokens, size_t token_capacity,
               size_t *token_count, zlang_diagnostics_t *diagnostics);

bool zlang_token_is(const zlang_token_t *token, const char *text);

#endif /* ZLANG_COMPILER_LEXER_H */
