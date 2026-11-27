/** @file
 * @brief Standard library registry used by semantic analysis and emission.
 */

#ifndef ZLANG_COMPILER_STDLIB_H
#define ZLANG_COMPILER_STDLIB_H

#include <stdbool.h>
#include <stddef.h>

#define ZLANG_STD_MAX_PARAMETERS 4U

typedef enum zlang_std_return_kind_t {
    zlang_std_return_value_ = 0U,
    zlang_std_return_result_
} zlang_std_return_kind_t;

typedef struct zlang_std_result_t {
    const char *name_;
    const char *value_type_;
    const char *error_type_;
    const char *c_type_;
    const char *c_constructor_;
} zlang_std_result_t;

typedef struct zlang_std_function_t {
    const char *domain_;
    const char *name_;
    const char *return_type_;
    const char *c_symbol_;
    const char *activation_macro_;
    size_t parameter_count_;
    const char *parameter_types_[ZLANG_STD_MAX_PARAMETERS];
    bool is_stateful_;
    bool is_opaque_;
    bool parameter_is_reference_[ZLANG_STD_MAX_PARAMETERS];
    const bool parameter_is_const_[ZLANG_STD_MAX_PARAMETERS];
    zlang_std_return_kind_t return_kind_;
    const char *return_result_value_type_;
    const char *return_result_error_type_;
} zlang_std_function_t;

size_t zlang_std_function_count(void);
const zlang_std_function_t *zlang_std_function_at(size_t index);
const zlang_std_function_t *zlang_std_find(const char *domain, const char *name);
const zlang_std_function_t *zlang_std_find_by_name(const char *name);
const zlang_std_function_t *zlang_std_resolve(const char *domain, const char *name,
                                              const char *const *argument_types, size_t count);
const zlang_std_function_t *
zlang_std_resolve_by_name(const char *name, const char *const *argument_types, size_t count);

size_t zlang_std_result_count(void);
const zlang_std_result_t *zlang_std_result_at(size_t index);
const zlang_std_result_t *zlang_std_find_result(const char *name);
const zlang_std_result_t *zlang_std_find_result_by_value(const char *value_type);
const char *zlang_std_native_c_type(const char *name);
const char *zlang_std_native_result_field_type(const zlang_std_result_t *result,
                                               const char *field_name);
const char *zlang_std_native_result_tag(const zlang_std_result_t *result);
bool zlang_std_is_native_type(const char *name);
bool zlang_std_is_native_generic_base(const char *name);
bool zlang_std_is_view_type(const char *name);
bool zlang_std_view_element_type(const char *name, char *output, size_t capacity);
bool zlang_std_is_known_generic_argument(const char *name);
const char *zlang_std_status_constant_c_name(const char *name);
bool zlang_std_is_status_constant(const char *name);
const char *zlang_std_stream_constant_c_name(const char *name);
bool zlang_std_is_stream_constant(const char *name);
bool zlang_std_is_native_result_method(const zlang_std_result_t *result, const char *method_name);
const char *zlang_std_native_result_method_type(const zlang_std_result_t *result,
                                                const char *method_name);

#endif /* ZLANG_COMPILER_STDLIB_H */
