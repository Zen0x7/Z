/** @file
 * @brief Standard library registry used by semantic analysis and emission.
 */

#ifndef ZLANG_COMPILER_STDLIB_H
#define ZLANG_COMPILER_STDLIB_H

#include <stdbool.h>
#include <stddef.h>

#define ZLANG_STD_MAX_PARAMETERS 4U

typedef struct zlang_std_function_t {
    const char *domain_;
    const char *name_;
    const char *return_type_;
    const char *c_symbol_;
    const char *activation_macro_;
    const char *parameter_types_[ZLANG_STD_MAX_PARAMETERS];
    bool parameter_is_reference_[ZLANG_STD_MAX_PARAMETERS];
    size_t parameter_count_;
    bool is_stateful_;
    bool is_opaque_;
} zlang_std_function_t;

const zlang_std_function_t *zlang_std_find(const char *domain, const char *name);
const zlang_std_function_t *zlang_std_find_by_name(const char *name);

#endif /* ZLANG_COMPILER_STDLIB_H */
