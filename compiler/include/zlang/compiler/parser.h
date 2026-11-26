/** @file
 * @brief Parser API for the Zlang grammar.
 */

#ifndef ZLANG_COMPILER_PARSER_H
#define ZLANG_COMPILER_PARSER_H

#include "zlang/compiler/ast.h"
#include "zlang/compiler/diagnostics.h"
#include "zlang/compiler/generics.h"
#include "zlang/compiler/lexer.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct zlang_parse_result_t {
    zlang_program_t program_;
    size_t token_count_;
} zlang_parse_result_t;

typedef struct zlang_parser_t {
    zlang_lexer_t lexer_;
    zlang_token_t *tokens_;
    size_t token_capacity_;
    size_t token_count_;
    zlang_ast_expression_t *expressions_;
    size_t expression_capacity_;
    size_t expression_count_;
    zlang_ast_statement_t *statements_;
    size_t statement_capacity_;
    size_t statement_count_;
    zlang_parameter_t *parameters_;
    size_t parameter_capacity_;
    size_t parameter_count_;
    zlang_diagnostics_t *diagnostics_;
    const char *string_names_[32];
    size_t string_count_;
    zlang_generic_application_t generic_applications_[ZLANG_MAX_GENERIC_APPLICATIONS];
    size_t generic_application_count_;
    const char *active_generic_parameters_[ZLANG_GENERIC_PARAMETER_CAPACITY];
    size_t active_generic_parameter_count_;
    size_t position_;
    size_t switch_depth_;
    bool failed_;
} zlang_parser_t;

void zlang_parser_init(zlang_parser_t *parser, zlang_token_t *tokens, size_t token_capacity,
                       zlang_ast_expression_t *expressions, size_t expression_capacity,
                       zlang_ast_statement_t *statements, size_t statement_capacity,
                       zlang_parameter_t *parameters, size_t parameter_capacity,
                       zlang_comment_t *comments, size_t comment_capacity,
                       zlang_diagnostics_t *diagnostics);

bool zlang_parser_record_generic_application(zlang_parser_t *parser, const char *name,
                                             const char *base_name, const char *const *arguments,
                                             size_t argument_count, uint32_t line, uint32_t column,
                                             bool is_function);
const zlang_generic_application_t *
zlang_parser_find_generic_application(const zlang_parser_t *parser, const char *name);

/* Parses source into caller-owned AST arrays. Returns false when parsing or
   lexing produces a diagnostic; result is only valid on success. */
bool zlang_parse(zlang_parser_t *parser, const char *source, char *scratch, size_t scratch_capacity,
                 zlang_parse_result_t *result);

#endif /* ZLANG_COMPILER_PARSER_H */
