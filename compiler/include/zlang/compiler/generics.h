/** @file
 * @brief Shared helpers for compile-time generic type applications.
 */

#ifndef ZLANG_COMPILER_GENERICS_H
#define ZLANG_COMPILER_GENERICS_H

#include "zlang/compiler/ast.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct zlang_generic_name_t {
    char name_[ZLANG_GENERIC_NAME_CAPACITY];
    const char *base_name_;
    const char *arguments_[ZLANG_GENERIC_PARAMETER_CAPACITY];
    size_t argument_count_;
} zlang_generic_name_t;

const zlang_generic_application_t *zlang_generic_find_application(const zlang_program_t *program,
                                                                  const char *name);
zlang_generic_application_t *zlang_generic_add_application(zlang_program_t *program,
                                                           const char *name, const char *base_name);
bool zlang_generic_parse_name(const char *name, zlang_generic_name_t *parsed);
bool zlang_generic_build_name(const char *base_name, const char *const *arguments,
                              size_t argument_count, char *output, size_t capacity);
const zlang_ast_statement_t *zlang_generic_find_declaration(const zlang_program_t *program,
                                                            const char *base_name,
                                                            bool is_function);
bool zlang_generic_parameter_index(const zlang_ast_statement_t *declaration, const char *name,
                                   size_t *index);
bool zlang_generic_substitute(const char *type_name, const zlang_ast_statement_t *declaration,
                              const zlang_generic_application_t *application, char *output,
                              size_t capacity);
const char *zlang_generic_store_name(zlang_program_t *program, const char *base_name,
                                     const char *const *arguments, size_t argument_count);
bool zlang_generic_resolve_type(const char *type_name, const zlang_ast_statement_t *declaration,
                                const zlang_generic_application_t *application, char *output,
                                size_t capacity);

#endif /* ZLANG_COMPILER_GENERICS_H */
