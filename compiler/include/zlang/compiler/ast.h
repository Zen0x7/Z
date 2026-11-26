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
#define ZLANG_NO_STATEMENT ((size_t) - 1)
#define ZLANG_CALL_ARGUMENT_CAPACITY 32U
#define ZLANG_ARRAY_MAX_RANK 4U
#define ZLANG_MAX_INFERRED_SHAPES 16U
#define ZLANG_GENERIC_PARAMETER_CAPACITY 4U
#define ZLANG_GENERIC_NAME_CAPACITY 128U
#define ZLANG_MAX_GENERIC_APPLICATIONS 32U

typedef struct zlang_generic_application_t {
    const char *name_;
    const char *base_name_;
    const char *arguments_[ZLANG_GENERIC_PARAMETER_CAPACITY];
    size_t argument_count_;
    uint32_t line_;
    uint32_t column_;
    bool is_function_;
} zlang_generic_application_t;

typedef enum zlang_expression_kind_t {
    zlang_expression_integer_literal_,
    zlang_expression_float_literal_,
    zlang_expression_boolean_literal_,
    zlang_expression_string_literal_,
    zlang_expression_identifier_,
    zlang_expression_add_,
    zlang_expression_equal_,
    zlang_expression_assignment_,
    zlang_expression_append_call_,
    zlang_expression_index_,
    zlang_expression_index_assignment_,
    zlang_expression_string_concat_,
    zlang_expression_call_,
    zlang_expression_index_tuple_,
    zlang_expression_array_get_,
    zlang_expression_array_set_,
    zlang_expression_less_,
    zlang_expression_greater_,
    zlang_expression_less_equal_,
    zlang_expression_greater_equal_,
    zlang_expression_not_equal_,
    zlang_expression_not_,
    zlang_expression_and_,
    zlang_expression_or_,
    zlang_expression_negative_,
    zlang_expression_sub_,
    zlang_expression_mul_,
    zlang_expression_div_,
    zlang_expression_mod_,
    zlang_expression_compound_assignment_,
    zlang_expression_length_,
    zlang_expression_enum_item_,
    zlang_expression_field_,
    zlang_expression_field_assignment_,
    zlang_expression_direct_index_assignment_,
    zlang_expression_conversion_,
    zlang_expression_construct_,
    zlang_expression_method_call_,
    zlang_expression_wildcard_,
    zlang_expression_match_,
    zlang_expression_ternary_
} zlang_expression_kind_t;

typedef struct zlang_ast_string_value_t {
    const char *data_;
    uint32_t length_;
} zlang_ast_string_value_t;

typedef struct zlang_ast_expression_t {
    size_t left_;
    size_t right_;
    size_t argument_indices_[ZLANG_CALL_ARGUMENT_CAPACITY];
    size_t argument_count_;
    const char *name_;
    const char *base_name_;
    const char *resolved_type_name_;
    zlang_ast_string_value_t string_;
    zlang_expression_kind_t kind_;
    int32_t integer_value_;
    uint64_t integer64_value_;
    double float_value_;
    zlang_expression_kind_t operator_;
    uint32_t line_;
    uint32_t column_;
    bool bool_value_;
    bool is_unsigned_;
    bool is_int64_;
    bool is_uint64_;
    bool is_float_;
    bool is_string_;
    bool is_proven_;
} zlang_ast_expression_t;

typedef enum zlang_statement_kind_t {
    zlang_statement_declaration_,
    zlang_statement_array_declaration_,
    zlang_statement_expression_,
    zlang_statement_if_,
    zlang_statement_while_,
    zlang_statement_foreach_,
    zlang_statement_repeat_,
    zlang_statement_function_,
    zlang_statement_import_,
    zlang_statement_return_,
    zlang_statement_block_,
    zlang_statement_for_,
    zlang_statement_struct_,
    zlang_statement_trait_,
    zlang_statement_enum_,
    zlang_statement_break_,
    zlang_statement_continue_,
    zlang_statement_label_,
    zlang_statement_jump_,
    zlang_statement_switch_,
    zlang_statement_case_,
    zlang_statement_default_
} zlang_statement_kind_t;

typedef struct zlang_parameter_t {
    const char *type_name_;
    const char *name_;
    uint32_t extents_[ZLANG_ARRAY_MAX_RANK];
    uint32_t rank_;
    uint32_t line_;
    uint32_t column_;
    bool is_const_;
    bool is_reference_;
    bool is_array_;
} zlang_parameter_t;

// NOLINTNEXTLINE(clang-analyzer-optin.performance.Padding)
typedef struct zlang_ast_statement_t {
    const char *type_name_;
    const char *name_;
    const char *base_name_;
    const char *index_name_;
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
    size_t initializer_statement_;
    size_t update_expression_;
    zlang_statement_kind_t kind_;
    uint32_t capacity_;
    uint32_t element_capacity_;
    uint32_t extents_[ZLANG_ARRAY_MAX_RANK];
    uint32_t initializer_extents_[ZLANG_ARRAY_MAX_RANK];
    uint32_t rank_;
    bool has_explicit_capacity_;
    bool has_element_capacity_;
    bool is_const_;
    bool is_inferred_;
    bool use_canonical_type_;
    bool is_array_return_;
    uint32_t return_extents_[ZLANG_ARRAY_MAX_RANK];
    uint32_t return_rank_;
    const char *foreach_names_[ZLANG_ARRAY_MAX_RANK];
    uint32_t foreach_rank_;
    bool is_main_;
    bool is_static_;
    bool is_constructor_;
    const char *owner_name_;
    const char *uses_names_[4];
    size_t uses_count_;
    const char *generic_parameters_[ZLANG_GENERIC_PARAMETER_CAPACITY];
    size_t generic_parameter_count_;
    bool is_generic_;
    const char *requires_names_[4];
    const char *requires_type_names_[4];
    size_t requires_count_;
    uint32_t line_;
    uint32_t column_;
} zlang_ast_statement_t;

typedef struct zlang_inferred_shape_t {
    size_t function_statement_;
    const char *type_name_;
    uint32_t rank_;
    uint32_t extents_[ZLANG_ARRAY_MAX_RANK];
    uint32_t capacity_;
} zlang_inferred_shape_t;

typedef struct zlang_comment_t {
    const char *text_;
    uint32_t length_;
    uint32_t line_;
    uint32_t end_line_;
    uint32_t column_;
    bool is_block_;
} zlang_comment_t;

typedef struct zlang_program_t {
    zlang_ast_statement_t *statements_;
    size_t statement_count_;
    zlang_ast_expression_t *expressions_;
    size_t expression_count_;
    zlang_parameter_t *parameters_;
    size_t parameter_count_;
    const zlang_comment_t *comments_;
    size_t comment_count_;
    zlang_inferred_shape_t inferred_shapes_[ZLANG_MAX_INFERRED_SHAPES];
    size_t inferred_shape_count_;
    zlang_generic_application_t generic_applications_[ZLANG_MAX_GENERIC_APPLICATIONS];
    size_t generic_application_count_;
} zlang_program_t;

#endif /* ZLANG_COMPILER_AST_H */
