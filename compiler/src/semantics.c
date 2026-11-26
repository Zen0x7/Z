/** @file
 * @brief Scope, binding, type and resource checks for Zlang.
 */

#include "zlang/compiler/semantics.h"
#include "zlang/compiler/generics.h"
#include "zlang/compiler/stdlib.h"

#include <string.h>

static const char *zlang_semantic_std_domain(const char *specifier) {
    if (specifier == NULL) {
        return NULL;
    }
    const char *const _prefix_k = "@std/";
    const size_t _prefix_length_k = strlen(_prefix_k);
    return strncmp(specifier, _prefix_k, _prefix_length_k) == 0 ? specifier + _prefix_length_k
                                                                : NULL;
}

void zlang_semantic_context_init(zlang_semantic_context_t *context, zlang_symbol_t *symbols,
                                 size_t symbol_capacity, zlang_diagnostics_t *diagnostics,
                                 const zlang_program_t *program) {
    context->symbols_ = symbols;
    context->symbol_capacity_ = symbol_capacity;
    context->symbol_count_ = 0U;
    context->scope_depth_ = 0U;
    context->loop_depth_ = 0U;
    context->switch_depth_ = 0U;
    context->current_return_type_ = NULL;
    context->has_error_ = false;
    context->diagnostics_ = diagnostics;
    context->program_ = program;
    context->current_struct_ = NULL;
    context->current_generic_declaration_ = NULL;
}

static zlang_symbol_t *zlang_find_mutable_symbol(zlang_semantic_context_t *context,
                                                 const char *name) {
    if (name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < context->symbol_count_; ++_index) {
        zlang_symbol_t *const _symbol_k = &context->symbols_[_index];
        if (strcmp(_symbol_k->name_, name) == 0) {
            return _symbol_k;
        }
    }
    return NULL;
}

static const zlang_symbol_t *zlang_find_symbol(const zlang_semantic_context_t *context,
                                               const char *name) {
    if (name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < context->symbol_count_; ++_index) {
        const zlang_symbol_t *const _symbol_k = &context->symbols_[_index];
        if (strcmp(_symbol_k->name_, name) == 0) {
            return _symbol_k;
        }
    }
    return NULL;
}

static const zlang_ast_statement_t *zlang_find_label(const zlang_program_t *program,
                                                     const char *name) {
    if (program == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_label_ && _statement_k->name_ != NULL &&
            strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

static const zlang_ast_statement_t *zlang_find_struct(const zlang_program_t *program,
                                                      const char *name) {
    if (program == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_struct_ && _statement_k->name_ != NULL &&
            strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

static const zlang_ast_statement_t *zlang_find_trait(const zlang_program_t *program,
                                                     const char *name) {
    if (program == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_trait_ && _statement_k->name_ != NULL &&
            strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

static const zlang_ast_statement_t *zlang_find_enum(const zlang_program_t *program,
                                                    const char *name) {
    if (program == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_enum_ && _statement_k->name_ != NULL &&
            strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

static bool zlang_validate_generic_declaration(zlang_semantic_context_t *context,
                                               const zlang_ast_statement_t *statement);

static const zlang_ast_statement_t *zlang_find_struct_method(const zlang_program_t *program,
                                                             const char *struct_name,
                                                             const char *method_name);

static bool zlang_method_signature_matches(const zlang_ast_statement_t *left,
                                           const zlang_ast_statement_t *right,
                                           const zlang_program_t *program) {
    if (left == NULL || right == NULL || left->parameter_count_ != right->parameter_count_ ||
        strcmp(left->type_name_, right->type_name_) != 0 || left->is_static_ != right->is_static_ ||
        left->is_constructor_ != right->is_constructor_) {
        return false;
    }
    for (size_t _index = 0U; _index < left->parameter_count_; ++_index) {
        if (strcmp(program->parameters_[left->parameter_start_ + _index].type_name_,
                   program->parameters_[right->parameter_start_ + _index].type_name_) != 0) {
            return false;
        }
    }
    return true;
}

static bool zlang_inherited_method_mismatch(const zlang_program_t *program,
                                            const zlang_ast_statement_t *structure,
                                            const zlang_ast_statement_t *method) {
    if (structure->base_name_ == NULL || method->is_constructor_) {
        return false;
    }
    const zlang_ast_statement_t *const _base_k = zlang_find_struct(program, structure->base_name_);
    const zlang_ast_statement_t *const _inherited_k =
        zlang_find_struct_method(program, _base_k != NULL ? _base_k->name_ : NULL, method->name_);
    return _inherited_k != NULL && !zlang_method_signature_matches(method, _inherited_k, program);
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_has_inherited_method(const zlang_program_t *program,
                                       const zlang_ast_statement_t *structure,
                                       const char *method_name) {
    if (structure == NULL || structure->base_name_ == NULL || method_name == NULL) {
        return false;
    }
    const zlang_ast_statement_t *const _base_k = zlang_find_struct(program, structure->base_name_);
    if (_base_k == NULL) {
        return false;
    }
    return zlang_find_struct_method(program, _base_k->name_, method_name) != NULL ||
           zlang_has_inherited_method(program, _base_k, method_name);
}

static const zlang_ast_statement_t *zlang_find_owner_method(const zlang_program_t *program,
                                                            const char *owner_name,
                                                            const char *method_name) {
    const zlang_ast_statement_t *const _owner_k = zlang_find_struct(program, owner_name) != NULL
                                                      ? zlang_find_struct(program, owner_name)
                                                      : zlang_find_trait(program, owner_name);
    if (_owner_k == NULL || method_name == NULL) {
        return NULL;
    }
    const size_t _end_k = _owner_k->body_start_ + _owner_k->body_count_;
    for (size_t _index = _owner_k->body_start_;
         _index < _end_k && _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _member_k = &program->statements_[_index];
        if (_member_k->kind_ == zlang_statement_function_ && _member_k->name_ != NULL &&
            strcmp(_member_k->name_, method_name) == 0) {
            return _member_k;
        }
    }
    return NULL;
}

static const zlang_ast_statement_t *zlang_find_struct_method(const zlang_program_t *program,
                                                             const char *struct_name,
                                                             const char *method_name) {
    const zlang_ast_statement_t *const _structure_k = zlang_find_struct(program, struct_name);
    if (_structure_k == NULL || method_name == NULL) {
        return NULL;
    }
    const size_t _end_k = _structure_k->body_start_ + _structure_k->body_count_;
    for (size_t _index = _structure_k->body_start_;
         _index < _end_k && _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _member_k = &program->statements_[_index];
        if (_member_k->kind_ == zlang_statement_function_ && _member_k->name_ != NULL &&
            strcmp(_member_k->name_, method_name) == 0) {
            return _member_k;
        }
    }
    return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
static const zlang_ast_statement_t *zlang_find_struct_method_any(const zlang_program_t *program,
                                                                 const char *struct_name,
                                                                 const char *method_name) {
    const zlang_ast_statement_t *const _structure_k = zlang_find_struct(program, struct_name);
    if (_structure_k == NULL || method_name == NULL) {
        return NULL;
    }
    const zlang_ast_statement_t *const _own_k =
        zlang_find_struct_method(program, struct_name, method_name);
    if (_own_k != NULL) {
        return _own_k;
    }
    for (size_t _trait = 0U; _trait < _structure_k->uses_count_; ++_trait) {
        const zlang_ast_statement_t *const _trait_k =
            zlang_find_trait(program, _structure_k->uses_names_[_trait]);
        if (_trait_k == NULL) {
            continue;
        }
        const zlang_ast_statement_t *const _method_k =
            zlang_find_owner_method(program, _trait_k->name_, method_name);
        if (_method_k != NULL) {
            return _method_k;
        }
    }
    if (_structure_k->base_name_ != NULL) {
        return zlang_find_struct_method_any(program, _structure_k->base_name_, method_name);
    }
    return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_type_matches(const char *expected, const char *actual);

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_is_ancestor(const zlang_program_t *program, const char *base_name,
                              const char *derived_name, uint32_t depth) {
    if (base_name == NULL || derived_name == NULL || depth > 16U) {
        return false;
    }
    if (strcmp(base_name, derived_name) == 0) {
        return true;
    }
    const zlang_ast_statement_t *const _derived_k = zlang_find_struct(program, derived_name);
    if (_derived_k == NULL || _derived_k->base_name_ == NULL) {
        return false;
    }
    return zlang_is_ancestor(program, base_name, _derived_k->base_name_, depth + 1U);
}

static bool zlang_type_accepts(const zlang_program_t *program, const char *expected,
                               const char *actual) {
    return zlang_type_matches(expected, actual) || zlang_is_ancestor(program, expected, actual, 0U);
}

static bool zlang_is_user_type(const zlang_program_t *program, const char *name) {
    return zlang_find_struct(program, name) != NULL || zlang_find_enum(program, name) != NULL;
}

static bool zlang_is_builtin_type(const char *name) {
    return name != NULL && (strcmp(name, "int32") == 0 || strcmp(name, "uint32") == 0 ||
                            strcmp(name, "int64") == 0 || strcmp(name, "uint64") == 0 ||
                            strcmp(name, "float") == 0 || strcmp(name, "double") == 0 ||
                            strcmp(name, "boolean") == 0 || strcmp(name, "string") == 0 ||
                            strcmp(name, "arguments") == 0 || zlang_std_is_native_type(name));
}

static bool zlang_is_numeric_type(const char *name) {
    return name != NULL && (strcmp(name, "int32") == 0 || strcmp(name, "uint32") == 0 ||
                            strcmp(name, "int64") == 0 || strcmp(name, "uint64") == 0 ||
                            strcmp(name, "float") == 0 || strcmp(name, "double") == 0);
}

static bool zlang_is_integral_type(const char *name) {
    return name != NULL && (strcmp(name, "int32") == 0 || strcmp(name, "uint32") == 0 ||
                            strcmp(name, "int64") == 0 || strcmp(name, "uint64") == 0);
}

static bool zlang_is_known_type(const zlang_semantic_context_t *context, const char *name) {
    if (zlang_is_builtin_type(name) || zlang_is_user_type(context->program_, name)) {
        return true;
    }
    if (context->current_generic_declaration_ != NULL &&
        zlang_generic_parameter_index(context->current_generic_declaration_, name, NULL)) {
        return true;
    }
    if (name == NULL || strchr(name, '<') == NULL) {
        return false;
    }
    if (zlang_std_find_result(name) != NULL) {
        return true;
    }
    const zlang_generic_application_t *const _application_k =
        zlang_generic_find_application(context->program_, name);
    if (_application_k == NULL) {
        return false;
    }
    const zlang_ast_statement_t *const _declaration_k = zlang_generic_find_declaration(
        context->program_, _application_k->base_name_, _application_k->is_function_);
    if (_declaration_k == NULL ||
        _application_k->argument_count_ != _declaration_k->generic_parameter_count_) {
        return false;
    }
    for (size_t _index = 0U; _index < _application_k->argument_count_; ++_index) {
        const char *const _argument_k = _application_k->arguments_[_index];
        if (!zlang_is_builtin_type(_argument_k) &&
            !zlang_is_user_type(context->program_, _argument_k)) {
            return false;
        }
    }
    return true;
}

static bool zlang_is_enum_type(const zlang_program_t *program, const char *name) {
    return zlang_find_enum(program, name) != NULL;
}

static const zlang_ast_statement_t *zlang_find_enum_owner(const zlang_program_t *program,
                                                          const char *name) {
    if (program == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ != zlang_statement_enum_) {
            continue;
        }
        const size_t _end_k = _statement_k->initializer_start_ + _statement_k->initializer_count_;
        for (size_t _item = _statement_k->initializer_start_;
             _item < _end_k && _item < program->expression_count_; ++_item) {
            const zlang_ast_expression_t *const _item_k = &program->expressions_[_item];
            if (_item_k->kind_ == zlang_expression_enum_item_ && _item_k->name_ != NULL &&
                strcmp(_item_k->name_, name) == 0) {
                return _statement_k;
            }
        }
    }
    return NULL;
}

static const zlang_ast_statement_t *zlang_find_field_local(const zlang_program_t *program,
                                                           const zlang_ast_statement_t *structure,
                                                           const char *name) {
    if (structure == NULL || name == NULL) {
        return NULL;
    }
    const size_t _end_k = structure->body_start_ + structure->body_count_;
    for (size_t _index = structure->body_start_;
         _index < _end_k && _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _field_k = &program->statements_[_index];
        if ((_field_k->kind_ == zlang_statement_declaration_ ||
             _field_k->kind_ == zlang_statement_array_declaration_) &&
            _field_k->name_ != NULL && strcmp(_field_k->name_, name) == 0) {
            return _field_k;
        }
    }
    return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
static const zlang_ast_statement_t *zlang_find_field(const zlang_program_t *program,
                                                     const zlang_ast_statement_t *structure,
                                                     const char *name) {
    if (structure == NULL || name == NULL) {
        return NULL;
    }
    const zlang_ast_statement_t *const _local_k = zlang_find_field_local(program, structure, name);
    if (_local_k != NULL) {
        return _local_k;
    }
    if (structure->base_name_ == NULL) {
        return NULL;
    }
    return zlang_find_field(program, zlang_find_struct(program, structure->base_name_), name);
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_struct_has_cycle(const zlang_program_t *program, const char *name,
                                   uint32_t depth) {
    if (name == NULL || depth > 16U) {
        return true;
    }
    const zlang_ast_statement_t *const _structure_k = zlang_find_struct(program, name);
    if (_structure_k == NULL) {
        return false;
    }
    if (_structure_k->base_name_ == NULL) {
        return false;
    }
    if (strcmp(_structure_k->base_name_, name) == 0) {
        return true;
    }
    return zlang_struct_has_cycle(program, _structure_k->base_name_, depth + 1U);
}

static bool zlang_report(zlang_semantic_context_t *context, uint16_t code,
                         const zlang_ast_statement_t *statement, const char *message) {
    context->has_error_ = true;
    (void)zlang_diagnostics_add(context->diagnostics_, code, statement->line_, statement->column_,
                                message);
    return false;
}

static void zlang_report_warning(zlang_semantic_context_t *context, uint16_t code,
                                 const zlang_ast_statement_t *statement, const char *message) {
    (void)zlang_diagnostics_add_warning(context->diagnostics_, code, statement->line_,
                                        statement->column_, message);
}

static bool zlang_report_expression(zlang_semantic_context_t *context, uint16_t code,
                                    size_t expression_index, const zlang_ast_statement_t *statement,
                                    const char *message) {
    const zlang_ast_expression_t *const _expression_k =
        expression_index < context->program_->expression_count_
            ? &context->program_->expressions_[expression_index]
            : NULL;
    const uint32_t _line_k = _expression_k != NULL ? _expression_k->line_ : statement->line_;
    const uint32_t _column_k = _expression_k != NULL ? _expression_k->column_ : statement->column_;

    context->has_error_ = true;
    (void)zlang_diagnostics_add(context->diagnostics_, code, _line_k, _column_k, message);
    return false;
}

static bool zlang_statement_in_program(const zlang_program_t *program,
                                       const zlang_ast_statement_t *statement) {
    if (program == NULL || statement == NULL) {
        return false;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        if (&program->statements_[_index] == statement) {
            return true;
        }
    }
    return false;
}

static bool zlang_add_symbol(zlang_semantic_context_t *context,
                             const zlang_ast_statement_t *statement, uint32_t capacity,
                             bool is_array, bool is_function) {
    const zlang_symbol_t *const _existing_k = zlang_find_symbol(context, statement->name_);
    if (_existing_k != NULL) {
        return zlang_report(context,
                            _existing_k->scope_depth_ == context->scope_depth_
                                ? ZLANG_DIAGNOSTIC_DUPLICATE_DECLARATION
                                : ZLANG_DIAGNOSTIC_SHADOWED_DECLARATION,
                            statement,
                            _existing_k->scope_depth_ == context->scope_depth_
                                ? "declaration redefines an existing name in this scope"
                                : "declaration shadows a visible name");
    }
    if (context->symbol_count_ >= context->symbol_capacity_) {
        return zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                            "symbol table capacity exceeded");
    }
    zlang_symbol_t *const _symbol_k = &context->symbols_[context->symbol_count_++];
    _symbol_k->name_ = statement->name_;
    _symbol_k->type_name_ = statement->type_name_;
    _symbol_k->declaration_ =
        zlang_statement_in_program(context->program_, statement) ? statement : NULL;
    _symbol_k->capacity_ = capacity;
    _symbol_k->rank_ = statement->rank_;
    memset(_symbol_k->extents_, 0, sizeof(_symbol_k->extents_));
    for (uint32_t _axis = 0U; _axis < statement->rank_ && _axis < ZLANG_ARRAY_MAX_RANK; ++_axis) {
        _symbol_k->extents_[_axis] = statement->extents_[_axis];
    }
    _symbol_k->scope_depth_ = context->scope_depth_;
    _symbol_k->parameter_start_ = statement->parameter_start_;
    _symbol_k->parameter_count_ = statement->parameter_count_;
    _symbol_k->is_sequence_ = is_array;
    _symbol_k->is_function_ = is_function;
    _symbol_k->has_explicit_capacity_ = statement->has_explicit_capacity_;
    _symbol_k->is_mutated_ = false;
    _symbol_k->is_const_ = statement->is_const_;
    _symbol_k->is_reference_ = false;
    return true;
}

static bool zlang_add_function_symbol(zlang_semantic_context_t *context,
                                      const zlang_ast_statement_t *statement) {
    const zlang_symbol_t *const _existing_k = zlang_find_symbol(context, statement->name_);
    if (_existing_k != NULL && _existing_k->is_function_ &&
        _existing_k->scope_depth_ == context->scope_depth_) {
        return true;
    }
    return zlang_add_symbol(context, statement, 0U, false, true);
}

static void zlang_mark_mutated(zlang_semantic_context_t *context, const char *name) {
    zlang_symbol_t *const _symbol_k = zlang_find_mutable_symbol(context, name);
    if (_symbol_k != NULL) {
        _symbol_k->is_mutated_ = true;
    }
}

static bool zlang_type_matches(const char *expected, const char *actual) {
    return expected != NULL && actual != NULL && strcmp(expected, actual) == 0;
}

static bool zlang_is_small_value_type(const zlang_semantic_context_t *context,
                                      const char *type_name) {
    return type_name != NULL &&
           (zlang_is_builtin_type(type_name) || zlang_is_enum_type(context->program_, type_name));
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_is_literal_string(const zlang_program_t *program,
                                    size_t index) { // NOLINT(misc-no-recursion)
    if (index == ZLANG_NO_EXPRESSION || index >= program->expression_count_) {
        return false;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ == zlang_expression_string_literal_) {
        return true;
    }
    if (_expression_k->kind_ != zlang_expression_string_concat_) {
        return false;
    }
    return zlang_is_literal_string(program, _expression_k->left_) &&
           zlang_is_literal_string(program, _expression_k->right_);
}

static const char *zlang_analyze_expression(zlang_semantic_context_t *context, size_t index,
                                            const zlang_ast_statement_t *statement);

/* Expression trees are bounded by the caller-owned expression array. */
static bool zlang_expression_is_lvalue(const zlang_semantic_context_t *context, size_t index,
                                       bool *is_const) {
    if (index == ZLANG_NO_EXPRESSION || index >= context->program_->expression_count_) {
        return false;
    }
    const zlang_ast_expression_t *const _expression_k = &context->program_->expressions_[index];
    const char *_name_k = NULL;
    if (_expression_k->kind_ == zlang_expression_identifier_ ||
        _expression_k->kind_ == zlang_expression_index_) {
        _name_k = _expression_k->name_;
    } else if (_expression_k->kind_ == zlang_expression_field_ &&
               _expression_k->base_name_ != NULL) {
        _name_k = _expression_k->base_name_;
    }
    if (_name_k == NULL) {
        return false;
    }
    const zlang_symbol_t *const _symbol_k = zlang_find_symbol(context, _name_k);
    if (_symbol_k == NULL || _symbol_k->is_function_) {
        return false;
    }
    if (is_const != NULL) {
        *is_const = _symbol_k->is_const_;
    }
    return true;
}

static const char *zlang_native_result_field_type(const char *record_name, const char *field_name) {
    const zlang_std_result_t *const _result_k = zlang_std_find_result(record_name);
    return zlang_std_native_result_field_type(_result_k, field_name);
}

static const char *
// NOLINTNEXTLINE(misc-no-recursion)
zlang_analyze_expression(zlang_semantic_context_t *context, size_t index,
                         const zlang_ast_statement_t *statement) { // NOLINT(misc-no-recursion)
    if (index == ZLANG_NO_EXPRESSION || index >= context->program_->expression_count_) {
        return NULL;
    }
    const zlang_ast_expression_t *const _expression_k = &context->program_->expressions_[index];
    switch (_expression_k->kind_) {
    case zlang_expression_integer_literal_:
        if (_expression_k->is_int64_) {
            return "int64";
        }
        if (_expression_k->is_uint64_) {
            return "uint64";
        }
        return _expression_k->is_unsigned_ ? "uint32" : "int32";
    case zlang_expression_float_literal_:
        return _expression_k->is_float_ ? "float" : "double";
    case zlang_expression_conversion_: {
        const char *const _source_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        const bool _enum_source_k = zlang_is_enum_type(context->program_, _source_k);
        const bool _integer_target_k = zlang_type_matches("int32", _expression_k->name_) ||
                                       zlang_type_matches("uint32", _expression_k->name_) ||
                                       zlang_type_matches("int64", _expression_k->name_) ||
                                       zlang_type_matches("uint64", _expression_k->name_);
        if ((!zlang_is_numeric_type(_source_k) && !(_enum_source_k && _integer_target_k)) ||
            !zlang_is_numeric_type(_expression_k->name_)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "conversion requires a numeric operand and target");
            return NULL;
        }
        return _expression_k->name_;
    }
    case zlang_expression_range_: {
        const char *const _start_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        const char *const _end_k =
            zlang_analyze_expression(context, _expression_k->right_, statement);
        if (!zlang_type_matches("uint32", _start_k) || !zlang_type_matches("uint32", _end_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "range endpoints must be uint32");
            return NULL;
        }
        return "uint32_range";
    }
    case zlang_expression_wildcard_:
        return NULL;
    case zlang_expression_match_: {
        const char *const _selector_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        const size_t _count_k = _expression_k->argument_count_;
        bool _has_wildcard_k = false;
        const char *_result_k = NULL;
        bool _valid_k = _selector_k != NULL && _count_k >= 2U && (_count_k % 2U) == 0U;
        for (size_t _arm = 0U; _valid_k && _arm < _count_k; _arm += 2U) {
            const size_t _pattern_index_k = _expression_k->argument_indices_[_arm];
            const size_t _value_index_k = _expression_k->argument_indices_[_arm + 1U];
            const zlang_ast_expression_t *const _pattern_k =
                _pattern_index_k < context->program_->expression_count_
                    ? &context->program_->expressions_[_pattern_index_k]
                    : NULL;
            const bool _arm_wildcard_k =
                _pattern_k != NULL && _pattern_k->kind_ == zlang_expression_wildcard_;
            _has_wildcard_k = _has_wildcard_k || _arm_wildcard_k;
            const char *const _pattern_type_k =
                _arm_wildcard_k ? _selector_k
                                : zlang_analyze_expression(context, _pattern_index_k, statement);
            const char *const _value_type_k =
                zlang_analyze_expression(context, _value_index_k, statement);
            _valid_k = _valid_k && _pattern_type_k != NULL &&
                       zlang_type_matches(_selector_k, _pattern_type_k) && _value_type_k != NULL &&
                       (_result_k == NULL || zlang_type_matches(_result_k, _value_type_k));
            if (_result_k == NULL) {
                _result_k = _value_type_k;
            }
        }
        if (!_valid_k || !_has_wildcard_k) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "match requires matching arms and a wildcard arm");
            return NULL;
        }
        return _result_k;
    }
    case zlang_expression_ternary_: {
        const char *const _condition_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        const char *const _then_k =
            zlang_analyze_expression(context, _expression_k->right_, statement);
        const char *const _else_k =
            _expression_k->argument_count_ == 1U
                ? zlang_analyze_expression(context, _expression_k->argument_indices_[0], statement)
                : NULL;
        if (!zlang_type_matches("boolean", _condition_k) || _then_k == NULL ||
            !zlang_type_matches(_then_k, _else_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "ternary branches must have the same type");
            return NULL;
        }
        return _then_k;
    }
    case zlang_expression_boolean_literal_:
        return "boolean";
    case zlang_expression_string_literal_:
        return "string";
    case zlang_expression_identifier_: {
        const zlang_symbol_t *const _symbol_k = zlang_find_symbol(context, _expression_k->name_);
        if (_symbol_k != NULL) {
            if (_symbol_k->declaration_ != NULL &&
                _symbol_k->declaration_->kind_ == zlang_statement_import_) {
                return NULL;
            }
            return _symbol_k->type_name_;
        }
        if (context->current_struct_ != NULL) {
            const zlang_ast_statement_t *const _field_k =
                zlang_find_field(context->program_, context->current_struct_, _expression_k->name_);
            if (_field_k != NULL) {
                return _field_k->type_name_;
            }
        }
        if (zlang_std_is_status_constant(_expression_k->name_)) {
            return "status_code";
        }
        if (zlang_std_is_stream_constant(_expression_k->name_)) {
            return "stream";
        }
        if (zlang_std_is_stream_constant(_expression_k->name_)) {
            return "stream";
        }
        const zlang_ast_statement_t *const _enum_k =
            zlang_find_enum_owner(context->program_, _expression_k->name_);
        if (_enum_k != NULL) {
            return _enum_k->name_;
        }
        (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                      statement, "expression refers to an undeclared identifier");
        return NULL;
    }
    case zlang_expression_add_: {
        const char *const _left_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        const char *const _right_k =
            zlang_analyze_expression(context, _expression_k->right_, statement);
        if (!zlang_is_numeric_type(_left_k) || !zlang_type_matches(_left_k, _right_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "arithmetic requires two matching numeric operands");
            return NULL;
        }
        return _left_k;
    }
    case zlang_expression_negative_: {
        const char *const _actual_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        const bool _signed_k =
            zlang_type_matches("int32", _actual_k) || zlang_type_matches("int64", _actual_k) ||
            zlang_type_matches("float", _actual_k) || zlang_type_matches("double", _actual_k);
        if (!_signed_k) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "unary negation requires a signed numeric operand");
            return NULL;
        }
        return _actual_k;
    }
    case zlang_expression_bit_and_:
    case zlang_expression_bit_or_:
    case zlang_expression_bit_xor_: {
        const char *const _left_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        const char *const _right_k =
            zlang_analyze_expression(context, _expression_k->right_, statement);
        if (!zlang_is_integral_type(_left_k) || !zlang_type_matches(_left_k, _right_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "bitwise operators require matching integral operands");
            return NULL;
        }
        return _left_k;
    }
    case zlang_expression_bit_not_: {
        const char *const _actual_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        if (!zlang_is_integral_type(_actual_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "bitwise negation requires an integral operand");
            return NULL;
        }
        return _actual_k;
    }
    case zlang_expression_shift_left_:
    case zlang_expression_shift_right_: {
        const char *const _left_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        const char *const _right_k =
            zlang_analyze_expression(context, _expression_k->right_, statement);
        if (!zlang_is_integral_type(_left_k) || !zlang_type_matches("uint32", _right_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "shift requires an integral value and uint32 count");
            return NULL;
        }
        return _left_k;
    }
    case zlang_expression_sub_:
    case zlang_expression_mul_:
    case zlang_expression_div_:
    case zlang_expression_mod_: {
        const char *const _left_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        const char *const _right_k =
            zlang_analyze_expression(context, _expression_k->right_, statement);
        if (_expression_k->kind_ == zlang_expression_mod_ && !zlang_is_integral_type(_left_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "modulo requires integral operands");
            return NULL;
        }
        const bool _numeric_k = zlang_is_numeric_type(_left_k);
        if (!_numeric_k || !zlang_type_matches(_left_k, _right_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "arithmetic requires two matching numeric operands");
            return NULL;
        }
        return _left_k;
    }
    case zlang_expression_compound_assignment_: {
        const zlang_ast_expression_t *const _target_k =
            _expression_k->left_ < context->program_->expression_count_
                ? &context->program_->expressions_[_expression_k->left_]
                : NULL;
        const zlang_symbol_t *const _symbol_k =
            _target_k != NULL && _target_k->kind_ == zlang_expression_identifier_
                ? zlang_find_symbol(context, _target_k->name_)
                : NULL;
        if (_symbol_k == NULL && _target_k != NULL && context->current_struct_ != NULL &&
            _target_k->kind_ == zlang_expression_identifier_ &&
            zlang_find_field(context->program_, context->current_struct_, _target_k->name_) !=
                NULL) {
            const zlang_ast_statement_t *const _field_k =
                zlang_find_field(context->program_, context->current_struct_, _target_k->name_);
            const char *const _actual_k =
                zlang_analyze_expression(context, _expression_k->right_, statement);
            if (!zlang_type_matches(_field_k->type_name_, _actual_k)) {
                (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index,
                                              statement,
                                              "compound assignment requires a matching field type");
                return NULL;
            }
            return _actual_k;
        }
        if (_symbol_k == NULL) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement,
                                          "compound assignment target is not a declared scalar");
            return NULL;
        }
        if (_symbol_k->is_const_) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "cannot assign to a const declaration");
            return NULL;
        }
        const char *const _actual_k =
            zlang_analyze_expression(context, _expression_k->right_, statement);
        if (_expression_k->operator_ == zlang_expression_mod_ &&
            !zlang_is_integral_type(_symbol_k->type_name_)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "modulo requires an integral target");
            return NULL;
        }
        const bool _numeric_k = zlang_is_numeric_type(_symbol_k->type_name_);
        if (!_numeric_k || !zlang_type_matches(_symbol_k->type_name_, _actual_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "compound assignment requires a matching numeric type");
            return NULL;
        }
        zlang_mark_mutated(context, _symbol_k->name_);
        return _actual_k;
    }
    case zlang_expression_length_: {
        const zlang_symbol_t *const _target_k = zlang_find_symbol(context, _expression_k->name_);
        if (_target_k == NULL || !_target_k->is_sequence_) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement, "length target is not a declared array");
            return NULL;
        }
        return "uint32";
    }
    case zlang_expression_equal_:
    case zlang_expression_less_:
    case zlang_expression_greater_:
    case zlang_expression_less_equal_:
    case zlang_expression_greater_equal_:
    case zlang_expression_not_equal_: {
        const char *const _left_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        const char *const _right_k =
            zlang_analyze_expression(context, _expression_k->right_, statement);
        if (!zlang_type_matches(_left_k, _right_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "comparison requires two operands of the same type");
            return NULL;
        }
        const bool _ordering_k = _expression_k->kind_ != zlang_expression_equal_ &&
                                 _expression_k->kind_ != zlang_expression_not_equal_;
        const bool _comparable_k =
            zlang_is_numeric_type(_left_k) || zlang_type_matches("boolean", _left_k) ||
            zlang_type_matches("string", _left_k) || zlang_is_enum_type(context->program_, _left_k);
        if (!_comparable_k || (_ordering_k && zlang_type_matches("string", _left_k))) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "unsupported comparison operand type");
            return NULL;
        }
        return "boolean";
    }
    case zlang_expression_not_: {
        const char *const _actual_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        if (!zlang_type_matches("boolean", _actual_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "logical negation requires a boolean operand");
            return NULL;
        }
        return "boolean";
    }
    case zlang_expression_and_:
    case zlang_expression_or_: {
        const char *const _left_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        const char *const _right_k =
            zlang_analyze_expression(context, _expression_k->right_, statement);
        if (!zlang_type_matches("boolean", _left_k) || !zlang_type_matches("boolean", _right_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "logical operators require boolean operands");
            return NULL;
        }
        return "boolean";
    }
    case zlang_expression_string_concat_: {
        const char *const _left_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        const char *const _right_k =
            zlang_analyze_expression(context, _expression_k->right_, statement);
        if (!zlang_type_matches("string", _left_k) || !zlang_type_matches("string", _right_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "string concatenation requires string operands");
            return NULL;
        }
        return "string";
    }
    case zlang_expression_assignment_: {
        const zlang_ast_expression_t *const _target_k =
            _expression_k->left_ < context->program_->expression_count_
                ? &context->program_->expressions_[_expression_k->left_]
                : NULL;
        const zlang_symbol_t *const _symbol_k =
            _target_k != NULL ? zlang_find_symbol(context, _target_k->name_) : NULL;
        const zlang_ast_statement_t *_field_k = NULL;
        const bool _field_target_k =
            _target_k != NULL && _symbol_k == NULL && context->current_struct_ != NULL &&
            _target_k->kind_ == zlang_expression_identifier_ &&
            zlang_find_field(context->program_, context->current_struct_, _target_k->name_) != NULL;
        if (_field_target_k) {
            _field_k =
                zlang_find_field(context->program_, context->current_struct_, _target_k->name_);
            const char *const _actual_k =
                zlang_analyze_expression(context, _expression_k->right_, statement);
            if (!zlang_type_matches(_field_k->type_name_, _actual_k)) {
                (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index,
                                              statement,
                                              "field value does not match the declared field type");
                return NULL;
            }
            return _actual_k;
        }
        if (_target_k == NULL || _target_k->kind_ != zlang_expression_identifier_ ||
            _symbol_k == NULL) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement,
                                          "assignment target is not a declared identifier");
            return NULL;
        }
        if (_symbol_k->is_const_) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "cannot assign to a const declaration");
            return NULL;
        }
        const zlang_ast_expression_t *const _value_k =
            _expression_k->right_ < context->program_->expression_count_
                ? &context->program_->expressions_[_expression_k->right_]
                : NULL;
        if (strcmp(_symbol_k->type_name_, "string") == 0 && _value_k != NULL &&
            _value_k->kind_ == zlang_expression_string_concat_ &&
            !_symbol_k->has_explicit_capacity_ &&
            !zlang_is_literal_string(context->program_, _expression_k->right_)) {
            (void)zlang_report_expression(
                context, ZLANG_DIAGNOSTIC_STRING_CAPACITY_REQUIRED, index, statement,
                "concatenation with a variable requires an explicit string capacity");
            return NULL;
        }
        const char *const _actual_k =
            zlang_analyze_expression(context, _expression_k->right_, statement);
        if (!zlang_type_accepts(context->program_, _symbol_k->type_name_, _actual_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "assignment value does not match the declared type");
            return NULL;
        }
        if (_symbol_k->is_sequence_) {
            const zlang_symbol_t *const _value_symbol_k =
                _value_k != NULL && _value_k->kind_ == zlang_expression_identifier_
                    ? zlang_find_symbol(context, _value_k->name_)
                    : NULL;
            if (_value_symbol_k == NULL || !_value_symbol_k->is_sequence_ ||
                _value_symbol_k->rank_ != _symbol_k->rank_) {
                (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index,
                                              statement,
                                              "whole-array copy requires the same array shape");
                return NULL;
            }
            for (uint32_t _axis = 0U; _axis < _symbol_k->rank_; ++_axis) {
                if (_symbol_k->extents_[_axis] != _value_symbol_k->extents_[_axis]) {
                    (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index,
                                                  statement,
                                                  "whole-array copy requires the same array shape");
                    return NULL;
                }
            }
        }
        zlang_mark_mutated(context, _symbol_k->name_);
        return _actual_k;
    }
    case zlang_expression_index_assignment_: {
        const zlang_ast_expression_t *const _target_k =
            _expression_k->left_ < context->program_->expression_count_
                ? &context->program_->expressions_[_expression_k->left_]
                : NULL;
        const zlang_symbol_t *const _symbol_k =
            _target_k != NULL && _target_k->kind_ == zlang_expression_index_
                ? zlang_find_symbol(context, _target_k->name_)
                : NULL;
        if (_symbol_k == NULL || !_symbol_k->is_sequence_) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement,
                                          "index assignment target is not a declared array");
            return NULL;
        }
        if (_symbol_k->is_const_) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "cannot assign to a const array");
            return NULL;
        }
        (void)zlang_analyze_expression(context, _target_k->right_, statement);
        const char *const _actual_k =
            zlang_analyze_expression(context, _expression_k->right_, statement);
        if (!zlang_type_matches(_symbol_k->type_name_, _actual_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "indexed value does not match the array element type");
            return NULL;
        }
        zlang_mark_mutated(context, _symbol_k->name_);
        return _actual_k;
    }
    case zlang_expression_append_call_: {
        const zlang_symbol_t *const _target_k = zlang_find_symbol(context, _expression_k->name_);
        if (_target_k == NULL || !_target_k->is_sequence_) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement, "append target is not a declared array");
            return NULL;
        }
        const char *const _actual_k =
            zlang_analyze_expression(context, _expression_k->right_, statement);
        if (!zlang_type_matches(_target_k->type_name_, _actual_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "appended value does not match the array element type");
            return NULL;
        }
        zlang_mark_mutated(context, _target_k->name_);
        return "void";
    }
    case zlang_expression_field_: {
        const char *_record_name_k = NULL;
        if (_expression_k->base_name_ != NULL) {
            const zlang_symbol_t *const _base_k =
                zlang_find_symbol(context, _expression_k->base_name_);
            _record_name_k = _base_k != NULL ? _base_k->type_name_ : NULL;
        } else {
            _record_name_k = zlang_analyze_expression(context, _expression_k->left_, statement);
        }
        const char *const _native_field_k =
            zlang_native_result_field_type(_record_name_k, _expression_k->name_);
        const zlang_ast_statement_t *const _record_k =
            _native_field_k != NULL ? NULL : zlang_find_struct(context->program_, _record_name_k);
        const zlang_ast_statement_t *const _field_k =
            _native_field_k != NULL
                ? NULL
                : zlang_find_field(context->program_, _record_k, _expression_k->name_);
        if (_native_field_k == NULL && _field_k == NULL) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement, "field is not declared on the struct");
            return NULL;
        }
        if (_native_field_k == NULL && _field_k->kind_ == zlang_statement_array_declaration_ &&
            _expression_k->argument_count_ > 0U) {
            if (_expression_k->argument_count_ != _field_k->rank_) {
                (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index,
                                              statement, "array field index rank mismatch");
                return NULL;
            }
            for (size_t _axis = 0U; _axis < _expression_k->argument_count_; ++_axis) {
                (void)zlang_analyze_expression(context, _expression_k->argument_indices_[_axis],
                                               statement);
            }
        }
        return _native_field_k != NULL ? _native_field_k : _field_k->type_name_;
    }
    case zlang_expression_field_assignment_: {
        const zlang_ast_expression_t *const _target_k =
            _expression_k->left_ < context->program_->expression_count_
                ? &context->program_->expressions_[_expression_k->left_]
                : NULL;
        if (_target_k == NULL || _target_k->kind_ != zlang_expression_field_) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement,
                                          "field assignment target is not a field access");
            return NULL;
        }
        const char *_record_name_k = NULL;
        if (_target_k->base_name_ != NULL) {
            const zlang_symbol_t *const _base_k = zlang_find_symbol(context, _target_k->base_name_);
            _record_name_k = _base_k != NULL ? _base_k->type_name_ : NULL;
        } else {
            _record_name_k = zlang_analyze_expression(context, _target_k->left_, statement);
        }
        const char *const _native_field_k =
            zlang_native_result_field_type(_record_name_k, _target_k->name_);
        const zlang_ast_statement_t *const _record_k =
            _native_field_k != NULL ? NULL : zlang_find_struct(context->program_, _record_name_k);
        const zlang_ast_statement_t *const _field_k =
            _native_field_k != NULL
                ? NULL
                : zlang_find_field(context->program_, _record_k, _target_k->name_);
        if (_native_field_k == NULL && _field_k == NULL) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement, "field is not declared on the struct");
            return NULL;
        }
        if (_native_field_k == NULL && _field_k->kind_ == zlang_statement_array_declaration_ &&
            _target_k->argument_count_ > 0U) {
            if (_target_k->argument_count_ != _field_k->rank_) {
                (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index,
                                              statement, "array field index rank mismatch");
                return NULL;
            }
            for (size_t _axis = 0U; _axis < _target_k->argument_count_; ++_axis) {
                (void)zlang_analyze_expression(context, _target_k->argument_indices_[_axis],
                                               statement);
            }
        }
        const char *const _actual_k =
            zlang_analyze_expression(context, _expression_k->right_, statement);
        const char *const _field_type_k =
            _native_field_k != NULL ? _native_field_k : _field_k->type_name_;
        if (!zlang_type_matches(_field_type_k, _actual_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "field value does not match the declared field type");
            return NULL;
        }
        if (_target_k->base_name_ != NULL) {
            const zlang_symbol_t *const _base_symbol_k =
                zlang_find_symbol(context, _target_k->base_name_);
            if (_base_symbol_k != NULL && _base_symbol_k->is_const_) {
                (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index,
                                              statement,
                                              "cannot assign to a field of a const value");
                return NULL;
            }
            if (_base_symbol_k != NULL &&
                (_base_symbol_k->is_sequence_ || _base_symbol_k->is_reference_)) {
                zlang_mark_mutated(context, _base_symbol_k->name_);
            }
        } else if (_target_k->left_ < context->program_->expression_count_) {
            const zlang_ast_expression_t *const _base_k =
                &context->program_->expressions_[_target_k->left_];
            if (_base_k->kind_ == zlang_expression_index_) {
                const zlang_symbol_t *const _base_symbol_k =
                    zlang_find_symbol(context, _base_k->name_);
                if (_base_symbol_k != NULL && _base_symbol_k->is_sequence_) {
                    zlang_mark_mutated(context, _base_symbol_k->name_);
                }
            }
        }
        return _actual_k;
    }
    case zlang_expression_index_: {
        const zlang_symbol_t *const _target_k = zlang_find_symbol(context, _expression_k->name_);
        const bool _is_string_k = _target_k != NULL && !_target_k->is_sequence_ &&
                                  strcmp(_target_k->type_name_, "string") == 0;
        if (_target_k == NULL || (!_target_k->is_sequence_ && !_is_string_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement,
                                          "index target is not a declared array or string");
            return NULL;
        }
        (void)zlang_analyze_expression(context, _expression_k->right_, statement);
        return _is_string_k ? "uint32" : _target_k->type_name_;
    }
    case zlang_expression_construct_: {
        const zlang_ast_statement_t *const _structure_k =
            zlang_find_struct(context->program_, _expression_k->name_);
        const zlang_ast_statement_t *const _constructor_k =
            zlang_find_struct_method(context->program_, _expression_k->name_, "constructor");
        if (_structure_k == NULL || _constructor_k == NULL) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement, "struct has no constructor");
            return NULL;
        }
        for (size_t _argument = 0U;
             _argument < _expression_k->argument_count_ && _argument < ZLANG_CALL_ARGUMENT_CAPACITY;
             ++_argument) {
            (void)zlang_analyze_expression(context, _expression_k->argument_indices_[_argument],
                                           statement);
        }
        return _expression_k->name_;
    }
    case zlang_expression_method_call_: {
        const char *_struct_name_k = NULL;
        bool _static_k = false;
        const zlang_symbol_t *const _base_k = zlang_find_symbol(context, _expression_k->base_name_);
        if (_base_k != NULL) {
            _struct_name_k = _base_k->type_name_;
        } else if (zlang_find_struct(context->program_, _expression_k->base_name_) != NULL) {
            _struct_name_k = _expression_k->base_name_;
            _static_k = true;
        }
        const zlang_std_result_t *const _native_result_k =
            _struct_name_k != NULL ? zlang_std_find_result(_struct_name_k) : NULL;
        if (_native_result_k != NULL) {
            const char *const _method_type_k =
                zlang_std_native_result_method_type(_native_result_k, _expression_k->name_);
            if (_expression_k->argument_count_ != 0U || _method_type_k == NULL) {
                (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                              statement, "result method is not declared");
                return NULL;
            }
            return _method_type_k;
        }
        const zlang_ast_statement_t *const _method_k =
            zlang_find_struct_method_any(context->program_, _struct_name_k, _expression_k->name_);
        if (_struct_name_k == NULL || _method_k == NULL || _method_k->is_static_ != _static_k) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement, "method is not declared on the struct");
            return NULL;
        }
        for (size_t _argument = 0U;
             _argument < _expression_k->argument_count_ && _argument < ZLANG_CALL_ARGUMENT_CAPACITY;
             ++_argument) {
            (void)zlang_analyze_expression(context, _expression_k->argument_indices_[_argument],
                                           statement);
        }
        return _method_k->type_name_;
    }
    case zlang_expression_call_: {
        const zlang_symbol_t *_function_k = zlang_find_symbol(context, _expression_k->name_);
        const bool _imported_k = _function_k != NULL && _function_k->declaration_ != NULL &&
                                 _function_k->declaration_->kind_ == zlang_statement_import_;
        if (!_imported_k && (_function_k == NULL || !_function_k->is_function_)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement, "call target is not a declared function");
            return NULL;
        }
        if (_imported_k) {
            const char *_argument_types_k[ZLANG_STD_MAX_PARAMETERS] = {NULL};
            const size_t _argument_count_k =
                _expression_k->argument_count_ < ZLANG_STD_MAX_PARAMETERS
                    ? _expression_k->argument_count_
                    : ZLANG_STD_MAX_PARAMETERS;
            for (size_t _argument = 0U; _argument < _argument_count_k; ++_argument) {
                const size_t _argument_index_k = _expression_k->argument_indices_[_argument];
                _argument_types_k[_argument] =
                    zlang_analyze_expression(context, _argument_index_k, statement);
                if (_argument_types_k[_argument] != NULL &&
                    _argument_index_k < context->program_->expression_count_) {
                    zlang_ast_expression_t *const _argument_expression_k =
                        &context->program_->expressions_[_argument_index_k];
                    _argument_expression_k->resolved_type_name_ = _argument_types_k[_argument];
                }
            }
            const zlang_std_function_t *const _std_k = zlang_std_resolve_by_name(
                _expression_k->name_, _argument_types_k, _argument_count_k);
            if (_std_k == NULL) {
                (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index,
                                              statement,
                                              "no standard overload matches the argument types");
                return NULL;
            }
            size_t _destination_parameter_k = ZLANG_CALL_ARGUMENT_CAPACITY;
            for (size_t _parameter = 0U; _parameter < _std_k->parameter_count_; ++_parameter) {
                if (!_std_k->parameter_is_reference_[_parameter] ||
                    _std_k->parameter_is_const_[_parameter] ||
                    _std_k->parameter_types_[_parameter] == NULL ||
                    strcmp(_std_k->parameter_types_[_parameter], "string") != 0) {
                    continue;
                }
                _destination_parameter_k = _parameter;
                break;
            }
            const bool _needs_bounded_destination_k =
                _destination_parameter_k < ZLANG_CALL_ARGUMENT_CAPACITY &&
                _destination_parameter_k < _expression_k->argument_count_ &&
                strcmp(_std_k->domain_, "io") == 0 &&
                (strcmp(_std_k->name_, "read") == 0 || strcmp(_std_k->name_, "read_line") == 0);
            if (_needs_bounded_destination_k) {
                const size_t _argument_index_k =
                    _expression_k->argument_indices_[_destination_parameter_k];
                const zlang_ast_expression_t *const _argument_expression_k =
                    _argument_index_k < context->program_->expression_count_
                        ? &context->program_->expressions_[_argument_index_k]
                        : NULL;
                const zlang_symbol_t *const _destination_k =
                    _argument_expression_k != NULL &&
                            _argument_expression_k->kind_ == zlang_expression_identifier_
                        ? zlang_find_symbol(context, _argument_expression_k->name_)
                        : NULL;
                if (_destination_k == NULL || !_destination_k->has_explicit_capacity_ ||
                    _destination_k->is_const_ || _destination_k->capacity_ == 0U) {
                    (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index,
                                                  statement,
                                                  "read requires a mutable bounded string");
                    return NULL;
                }
            }
            return _std_k->return_type_;
        }
        if (_expression_k->argument_count_ != _function_k->parameter_count_) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "call argument count does not match the declaration");
        }
        const size_t _argument_limit_k =
            _expression_k->argument_count_ < ZLANG_CALL_ARGUMENT_CAPACITY
                ? _expression_k->argument_count_
                : ZLANG_CALL_ARGUMENT_CAPACITY;
        const size_t _count_k = _argument_limit_k < _function_k->parameter_count_
                                    ? _argument_limit_k
                                    : _function_k->parameter_count_;
        for (size_t _argument = 0U; _argument < _count_k; ++_argument) {
            const size_t _expression_index_k = _expression_k->argument_indices_[_argument];
            const size_t _parameter_index_k = _function_k->parameter_start_ + _argument;
            if (_parameter_index_k >= context->program_->parameter_count_) {
                (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH,
                                              _expression_index_k, statement,
                                              "call parameter table is incomplete");
                continue;
            }
            const zlang_parameter_t *const _parameter_k =
                &context->program_->parameters_[_parameter_index_k];
            const char *const _expected_k = _parameter_k->type_name_;
            bool _argument_is_const_k = false;
            if (_parameter_k->is_reference_ &&
                !zlang_expression_is_lvalue(context, _expression_index_k, &_argument_is_const_k)) {
                (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH,
                                              _expression_index_k, statement,
                                              "reference argument must be an lvalue");
            } else if (_parameter_k->is_reference_ && !_parameter_k->is_const_ &&
                       _argument_is_const_k) {
                (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH,
                                              _expression_index_k, statement,
                                              "mutable reference cannot bind a const argument");
            }
            const char *const _actual_k =
                zlang_analyze_expression(context, _expression_index_k, statement);
            if (!zlang_type_accepts(context->program_, _expected_k, _actual_k)) {
                (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH,
                                              _expression_index_k, statement,
                                              "call argument does not match the parameter type");
            }
        }
        return _function_k->type_name_;
    }
    default:
        return NULL;
    }
}

static bool zlang_analyze_initializers(zlang_semantic_context_t *context,
                                       const zlang_ast_statement_t *statement) {
    bool _result_k = true;
    for (size_t _index = 0U; _index < statement->initializer_count_; ++_index) {
        const size_t _expression_index_k = statement->initializer_start_ + _index;
        const char *const _actual_k =
            zlang_analyze_expression(context, _expression_index_k, statement);
        if (!zlang_type_matches(statement->type_name_, _actual_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH,
                                          _expression_index_k, statement,
                                          "initializer element does not match the array type");
            _result_k = false;
        }
    }
    return _result_k;
}

static bool zlang_statement_in_method(const zlang_program_t *program,
                                      const zlang_ast_statement_t *structure, size_t index) {
    const size_t _end_k = structure->body_start_ + structure->body_count_;
    for (size_t _member = structure->body_start_;
         _member < _end_k && _member < program->statement_count_; ++_member) {
        const zlang_ast_statement_t *const _member_k = &program->statements_[_member];
        if (_member_k->kind_ != zlang_statement_function_ || _member_k->body_count_ == 0U) {
            continue;
        }
        const size_t _method_end_k = _member_k->body_start_ + _member_k->body_count_;
        if (index >= _member_k->body_start_ && index < _method_end_k) {
            return true;
        }
    }
    return false;
}

static bool zlang_analyze_body(zlang_semantic_context_t *context, size_t start, size_t count);

static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_analyze_statement(zlang_semantic_context_t *context,
                        const zlang_ast_statement_t *statement) { // NOLINT(misc-no-recursion)
    bool _result_k = true;
    switch (statement->kind_) {
    case zlang_statement_struct_: {
        if (!zlang_validate_generic_declaration(context, statement)) {
            return false;
        }
        const zlang_ast_statement_t *const _previous_generic_k =
            context->current_generic_declaration_;
        if (statement->is_generic_) {
            context->current_generic_declaration_ = statement;
        }
        if (statement->base_name_ != NULL) {
            if (zlang_find_struct(context->program_, statement->base_name_) == NULL) {
                _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                                         "base struct is not declared");
            } else if (zlang_struct_has_cycle(context->program_, statement->name_, 0U)) {
                _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, statement,
                                         "cyclic struct inheritance");
            }
        }
        for (size_t _trait = 0U; _trait < statement->uses_count_; ++_trait) {
            const zlang_ast_statement_t *const _trait_k =
                zlang_find_trait(context->program_, statement->uses_names_[_trait]);
            if (_trait_k == NULL) {
                _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                                         "trait is not declared");
                continue;
            }
            for (size_t _requirement = 0U; _requirement < _trait_k->requires_count_;
                 ++_requirement) {
                const zlang_ast_statement_t *const _field_k = zlang_find_field(
                    context->program_, statement, _trait_k->requires_names_[_requirement]);
                if (_field_k == NULL ||
                    !zlang_type_matches(_trait_k->requires_type_names_[_requirement],
                                        _field_k->type_name_)) {
                    _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, statement,
                                             "struct does not satisfy trait requirements");
                }
            }
        }
        for (size_t _trait = 0U; _trait < statement->uses_count_; ++_trait) {
            const zlang_ast_statement_t *const _trait_k =
                zlang_find_trait(context->program_, statement->uses_names_[_trait]);
            if (_trait_k == NULL) {
                continue;
            }
            const size_t _trait_end_k = _trait_k->body_start_ + _trait_k->body_count_;
            for (size_t _index = _trait_k->body_start_;
                 _index < _trait_end_k && _index < context->program_->statement_count_; ++_index) {
                const zlang_ast_statement_t *const _method_k =
                    &context->program_->statements_[_index];
                if (_method_k->kind_ != zlang_statement_function_ || _method_k->name_ == NULL) {
                    continue;
                }
                if (zlang_has_inherited_method(context->program_, statement, _method_k->name_)) {
                    _result_k =
                        zlang_report(context, ZLANG_DIAGNOSTIC_DUPLICATE_DECLARATION, _method_k,
                                     "trait method conflicts with an inherited method");
                }
                for (size_t _previous = 0U; _previous < _trait; ++_previous) {
                    const zlang_ast_statement_t *const _previous_trait_k =
                        zlang_find_trait(context->program_, statement->uses_names_[_previous]);
                    if (_previous_trait_k == NULL) {
                        continue;
                    }
                    const size_t _previous_end_k =
                        _previous_trait_k->body_start_ + _previous_trait_k->body_count_;
                    for (size_t _previous_index = _previous_trait_k->body_start_;
                         _previous_index < _previous_end_k &&
                         _previous_index < context->program_->statement_count_;
                         ++_previous_index) {
                        const zlang_ast_statement_t *const _previous_method_k =
                            &context->program_->statements_[_previous_index];
                        if (_previous_method_k->kind_ == zlang_statement_function_ &&
                            _previous_method_k->name_ != NULL &&
                            strcmp(_previous_method_k->name_, _method_k->name_) == 0) {
                            _result_k =
                                zlang_report(context, ZLANG_DIAGNOSTIC_DUPLICATE_DECLARATION,
                                             _method_k, "trait methods conflict with each other");
                        }
                    }
                }
            }
        }
        const size_t _end_k = statement->body_start_ + statement->body_count_;
        for (size_t _index = statement->body_start_;
             _index < _end_k && _index < context->program_->statement_count_; ++_index) {
            const zlang_ast_statement_t *const _field_k = &context->program_->statements_[_index];
            if (zlang_statement_in_method(context->program_, statement, _index)) {
                continue;
            }
            if (_field_k->kind_ == zlang_statement_function_) {
                if (zlang_find_field(context->program_, statement, _field_k->name_) != NULL) {
                    _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_DUPLICATE_DECLARATION,
                                             _field_k, "method name conflicts with a field");
                }
                if (zlang_inherited_method_mismatch(context->program_, statement, _field_k)) {
                    _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, _field_k,
                                             "override signature does not match the base method");
                }
                continue;
            }
            const bool _is_array_field_k = _field_k->kind_ == zlang_statement_array_declaration_;
            if ((!_is_array_field_k && _field_k->kind_ != zlang_statement_declaration_) ||
                !zlang_is_known_type(context, _field_k->type_name_)) {
                _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, _field_k,
                                         "struct field has an unknown type");
                continue;
            }
            if (_is_array_field_k) {
                if (_field_k->rank_ == 0U || _field_k->capacity_ == 0U ||
                    strcmp(_field_k->type_name_, "string") == 0 ||
                    _field_k->expression_ != ZLANG_NO_EXPRESSION ||
                    _field_k->initializer_count_ != 0U) {
                    _result_k = zlang_report(
                        context, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, _field_k,
                        "array fields need fixed non-string extents and no initializer");
                }
                continue;
            }
            if (strcmp(_field_k->type_name_, "string") == 0 ||
                _field_k->expression_ != ZLANG_NO_EXPRESSION || _field_k->is_const_) {
                _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, _field_k,
                                         "struct fields must be plain scalar or enum declarations");
                continue;
            }
            const zlang_ast_statement_t *const _previous_k =
                zlang_find_field(context->program_, statement, _field_k->name_);
            if (_previous_k != NULL && _previous_k != _field_k) {
                _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_DUPLICATE_DECLARATION, _field_k,
                                         "struct field is declared twice");
            }
        }
        const zlang_ast_statement_t *const _previous_struct_k = context->current_struct_;
        context->current_struct_ = statement;
        for (size_t _index = statement->body_start_;
             _index < _end_k && _index < context->program_->statement_count_; ++_index) {
            const zlang_ast_statement_t *const _member_k = &context->program_->statements_[_index];
            if (_member_k->kind_ == zlang_statement_function_ &&
                !zlang_analyze_statement(context, _member_k)) {
                _result_k = false;
            }
        }
        for (size_t _trait = 0U; _trait < statement->uses_count_; ++_trait) {
            const zlang_ast_statement_t *const _trait_k =
                zlang_find_trait(context->program_, statement->uses_names_[_trait]);
            if (_trait_k == NULL) {
                continue;
            }
            const size_t _trait_end_k = _trait_k->body_start_ + _trait_k->body_count_;
            for (size_t _index = _trait_k->body_start_;
                 _index < _trait_end_k && _index < context->program_->statement_count_; ++_index) {
                const zlang_ast_statement_t *const _method_k =
                    &context->program_->statements_[_index];
                if (_method_k->kind_ == zlang_statement_function_ &&
                    !zlang_analyze_statement(context, _method_k)) {
                    _result_k = false;
                }
            }
        }
        context->current_struct_ = _previous_struct_k;
        context->current_generic_declaration_ = _previous_generic_k;
        break;
    }
    case zlang_statement_enum_: {
        const size_t _end_k = statement->initializer_start_ + statement->initializer_count_;
        for (size_t _index = statement->initializer_start_;
             _index < _end_k && _index < context->program_->expression_count_; ++_index) {
            const zlang_ast_expression_t *const _item_k = &context->program_->expressions_[_index];
            if (_item_k->kind_ != zlang_expression_enum_item_ || _item_k->name_ == NULL) {
                _result_k =
                    zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, _index,
                                            statement, "enum value must be an identifier");
                continue;
            }
            if (zlang_find_enum_owner(context->program_, _item_k->name_) != statement) {
                _result_k =
                    zlang_report_expression(context, ZLANG_DIAGNOSTIC_DUPLICATE_DECLARATION, _index,
                                            statement, "enum value is declared twice");
            }
        }
        break;
    }
    case zlang_statement_declaration_: {
        if (!zlang_is_known_type(context, statement->type_name_)) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                                     "declaration uses an unknown type");
            break;
        }
        const char *const _actual_k =
            zlang_analyze_expression(context, statement->expression_, statement);
        if (statement->expression_ != ZLANG_NO_EXPRESSION &&
            !zlang_type_accepts(context->program_, statement->type_name_, _actual_k)) {
            _result_k = zlang_report_expression(
                context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, statement->expression_, statement,
                "initializer type does not match the declared type");
        }
        const zlang_ast_expression_t *const _initializer_k =
            statement->expression_ < context->program_->expression_count_
                ? &context->program_->expressions_[statement->expression_]
                : NULL;
        if (_result_k && strcmp(statement->type_name_, "string") == 0 && _initializer_k != NULL &&
            _initializer_k->kind_ == zlang_expression_string_concat_ &&
            !statement->has_explicit_capacity_ &&
            !zlang_is_literal_string(context->program_, statement->expression_)) {
            _result_k = zlang_report_expression(
                context, ZLANG_DIAGNOSTIC_STRING_CAPACITY_REQUIRED, statement->expression_,
                statement, "concatenation with a variable requires an explicit string capacity");
        }
        if (_result_k) {
            _result_k = zlang_add_symbol(context, statement, statement->capacity_, false, false);
        }
        break;
    }
    case zlang_statement_array_declaration_:
        if (!zlang_is_known_type(context, statement->type_name_)) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                                     "array uses an unknown element type");
            break;
        }
        if (zlang_find_struct(context->program_, statement->type_name_) != NULL &&
            statement->initializer_count_ > 0U) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, statement,
                                     "struct arrays do not support initializer lists");
            break;
        }
        if (statement->capacity_ == 0U ||
            (!statement->has_explicit_capacity_ && statement->initializer_count_ == 0U)) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNBOUNDED_SEQUENCE, statement,
                                     "array declaration requires a capacity or an initializer");
            break;
        }
        if (strcmp(statement->type_name_, "string") == 0 &&
            (!statement->has_element_capacity_ || statement->element_capacity_ == 0U)) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_STRING_CAPACITY_REQUIRED, statement,
                                     "string array elements require an explicit capacity");
            break;
        }
        if (statement->initializer_count_ > statement->capacity_) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_INITIALIZER_TOO_LONG, statement,
                                     "initializer has more elements than the declared capacity");
            break;
        }
        if (!zlang_analyze_initializers(context, statement)) {
            _result_k = false;
            break;
        }
        _result_k = zlang_add_symbol(context, statement, statement->capacity_, true, false);
        break;
    case zlang_statement_expression_:
        (void)zlang_analyze_expression(context, statement->expression_, statement);
        break;
    case zlang_statement_return_: {
        if (context->current_return_type_ == NULL) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, statement,
                                     "return is only valid inside a function");
            break;
        }
        if (statement->expression_ == ZLANG_NO_EXPRESSION) {
            if (strcmp(context->current_return_type_, "void") != 0) {
                _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, statement,
                                         "a non-void function must return a value");
            }
            break;
        }
        const char *const _actual_k =
            zlang_analyze_expression(context, statement->expression_, statement);
        if (strcmp(context->current_return_type_, "void") == 0) {
            _result_k = zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH,
                                                statement->expression_, statement,
                                                "a void function cannot return a value");
        } else if (!zlang_type_accepts(context->program_, context->current_return_type_,
                                       _actual_k)) {
            _result_k = zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH,
                                                statement->expression_, statement,
                                                "return type does not match the function type");
        }
        break;
    }
    case zlang_statement_if_: {
        const char *const _actual_k =
            zlang_analyze_expression(context, statement->expression_, statement);
        if (!zlang_type_matches("boolean", _actual_k)) {
            _result_k = zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH,
                                                statement->expression_, statement,
                                                "if condition must be boolean");
        }
        _result_k = zlang_analyze_body(context, statement->body_start_, statement->body_count_) &&
                    _result_k;
        if (statement->else_count_ > 0U) {
            _result_k =
                zlang_analyze_body(context, statement->else_start_, statement->else_count_) &&
                _result_k;
        }
        break;
    }
    case zlang_statement_while_: {
        const char *const _actual_k =
            zlang_analyze_expression(context, statement->expression_, statement);
        if (!zlang_type_matches("boolean", _actual_k)) {
            _result_k = zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH,
                                                statement->expression_, statement,
                                                "while condition must be boolean");
        }
        ++context->loop_depth_;
        _result_k = zlang_analyze_body(context, statement->body_start_, statement->body_count_) &&
                    _result_k;
        --context->loop_depth_;
        break;
    }
    case zlang_statement_for_: {
        const size_t _scope_start_k = context->symbol_count_;
        ++context->scope_depth_;
        if (statement->initializer_statement_ != ZLANG_NO_STATEMENT &&
            statement->initializer_statement_ < context->program_->statement_count_ &&
            !zlang_analyze_statement(
                context, &context->program_->statements_[statement->initializer_statement_])) {
            _result_k = false;
        }
        const char *const _actual_k =
            zlang_analyze_expression(context, statement->expression_, statement);
        if (!zlang_type_matches("boolean", _actual_k)) {
            _result_k = zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH,
                                                statement->expression_, statement,
                                                "for condition must be boolean");
        }
        (void)zlang_analyze_expression(context, statement->update_expression_, statement);
        ++context->loop_depth_;
        _result_k = zlang_analyze_body(context, statement->body_start_, statement->body_count_) &&
                    _result_k;
        --context->loop_depth_;
        context->symbol_count_ = _scope_start_k;
        --context->scope_depth_;
        break;
    }
    case zlang_statement_repeat_: {
        const char *const _actual_k =
            zlang_analyze_expression(context, statement->expression_, statement);
        if (!zlang_type_matches("uint32", _actual_k)) {
            _result_k = zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH,
                                                statement->expression_, statement,
                                                "repeat count must be uint32");
        }
        ++context->loop_depth_;
        _result_k = zlang_analyze_body(context, statement->body_start_, statement->body_count_) &&
                    _result_k;
        --context->loop_depth_;
        break;
    }
    case zlang_statement_do_while_: {
        const char *const _condition_k =
            zlang_analyze_expression(context, statement->expression_, statement);
        if (!zlang_type_matches("boolean", _condition_k)) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, statement,
                                     "do while condition must be boolean");
        }
        ++context->loop_depth_;
        _result_k = zlang_analyze_body(context, statement->body_start_, statement->body_count_) &&
                    _result_k;
        --context->loop_depth_;
        break;
    }
    case zlang_statement_break_:
        if (statement->name_ != NULL) {
            if (zlang_find_label(context->program_, statement->name_) == NULL) {
                _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                                         "break label is not a declared label");
            }
        } else if (context->loop_depth_ == 0U && context->switch_depth_ == 0U) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, statement,
                                     "break is only valid inside a loop or switch");
        }
        break;
    case zlang_statement_continue_:
        if (statement->name_ != NULL) {
            if (zlang_find_label(context->program_, statement->name_) == NULL) {
                _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                                         "continue label is not a declared label");
            }
        } else if (context->loop_depth_ == 0U) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, statement,
                                     "continue is only valid inside a loop");
        }
        break;
    case zlang_statement_switch_: {
        (void)zlang_analyze_expression(context, statement->expression_, statement);
        ++context->switch_depth_;
        const size_t _end_k = statement->body_start_ + statement->body_count_;
        for (size_t _clause = statement->body_start_;
             _clause < _end_k && _clause < context->program_->statement_count_; ++_clause) {
            const zlang_ast_statement_t *const _clause_k = &context->program_->statements_[_clause];
            if (_clause_k->kind_ == zlang_statement_case_ &&
                _clause_k->expression_ != ZLANG_NO_EXPRESSION) {
                (void)zlang_analyze_expression(context, _clause_k->expression_, _clause_k);
            }
            _result_k =
                zlang_analyze_body(context, _clause_k->body_start_, _clause_k->body_count_) &&
                _result_k;
        }
        --context->switch_depth_;
        break;
    }
    case zlang_statement_case_:
    case zlang_statement_default_:
        break;
    case zlang_statement_foreach_: {
        const bool _range_kind_k = statement->expression_ != ZLANG_NO_EXPRESSION;
        const zlang_symbol_t *const _iterable_k =
            _range_kind_k ? NULL : zlang_find_symbol(context, statement->iterable_name_);
        if (_range_kind_k) {
            (void)zlang_analyze_expression(context, statement->expression_, statement);
        } else if (_iterable_k == NULL || !_iterable_k->is_sequence_) {
            return zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                                "foreach iterable refers to an undeclared array");
        }
        const size_t _scope_start_k = context->symbol_count_;
        ++context->scope_depth_;
        if (statement->foreach_rank_ > 1U) {
            for (uint32_t _axis = 0U;
                 _axis < statement->foreach_rank_ && _axis < ZLANG_ARRAY_MAX_RANK; ++_axis) {
                const zlang_ast_statement_t _coordinate_k = {
                    .kind_ = zlang_statement_declaration_,
                    .type_name_ = "uint32",
                    .name_ = statement->foreach_names_[_axis],
                    .line_ = statement->line_,
                    .column_ = statement->column_,
                };
                if (!zlang_add_symbol(context, &_coordinate_k, 0U, false, false)) {
                    context->symbol_count_ = _scope_start_k;
                    --context->scope_depth_;
                    return false;
                }
            }
        } else if (statement->index_name_ != NULL) {
            const zlang_ast_statement_t _index_k = {
                .kind_ = zlang_statement_declaration_,
                .type_name_ = "uint32",
                .name_ = statement->index_name_,
                .line_ = statement->line_,
                .column_ = statement->column_,
            };
            if (!zlang_add_symbol(context, &_index_k, 0U, false, false)) {
                context->symbol_count_ = _scope_start_k;
                --context->scope_depth_;
                return false;
            }
        }
        if (statement->foreach_rank_ <= 1U) {
            const zlang_ast_statement_t _item_k = {
                .kind_ = zlang_statement_declaration_,
                .type_name_ = statement->type_name_,
                .name_ = statement->name_,
                .line_ = statement->line_,
                .column_ = statement->column_,
            };
            if (!zlang_add_symbol(context, &_item_k, 0U, false, false)) {
                context->symbol_count_ = _scope_start_k;
                --context->scope_depth_;
                return false;
            }
        }
        ++context->loop_depth_;
        _result_k = zlang_analyze_body(context, statement->body_start_, statement->body_count_);
        --context->loop_depth_;
        context->symbol_count_ = _scope_start_k;
        --context->scope_depth_;
        break;
    }
    case zlang_statement_function_: {
        if (!zlang_validate_generic_declaration(context, statement)) {
            return false;
        }
        const bool _member_k = statement->owner_name_ != NULL;
        if (!_member_k && !zlang_add_function_symbol(context, statement)) {
            return false;
        }
        const size_t _scope_start_k = context->symbol_count_;
        ++context->scope_depth_;
        if (_member_k && !statement->is_static_) {
            zlang_ast_statement_t _self_statement_k = {0};
            _self_statement_k.kind_ = zlang_statement_declaration_;
            _self_statement_k.type_name_ =
                zlang_find_trait(context->program_, statement->owner_name_) != NULL &&
                        context->current_struct_ != NULL
                    ? context->current_struct_->name_
                    : statement->owner_name_;
            _self_statement_k.name_ = "self";
            if (!zlang_add_symbol(context, &_self_statement_k, 0U, false, false)) {
                context->symbol_count_ = _scope_start_k;
                --context->scope_depth_;
                return false;
            }
        }
        for (size_t _index = 0U; _index < statement->parameter_count_; ++_index) {
            if (statement->parameter_start_ + _index >= context->program_->parameter_count_) {
                --context->scope_depth_;
                return zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                                    "function parameter table is incomplete");
            }
            const zlang_parameter_t *const _parameter_k =
                &context->program_->parameters_[statement->parameter_start_ + _index];
            zlang_ast_statement_t _parameter_statement_k = {0};
            _parameter_statement_k.kind_ = zlang_statement_declaration_;
            _parameter_statement_k.type_name_ = _parameter_k->type_name_;
            _parameter_statement_k.name_ = _parameter_k->name_;
            _parameter_statement_k.line_ = _parameter_k->line_;
            _parameter_statement_k.column_ = _parameter_k->column_;
            _parameter_statement_k.is_const_ = _parameter_k->is_const_;
            if (!zlang_add_symbol(context, &_parameter_statement_k, 0U, false, false)) {
                context->symbol_count_ = _scope_start_k;
                --context->scope_depth_;
                return false;
            }
            zlang_symbol_t *const _parameter_symbol_k =
                zlang_find_mutable_symbol(context, _parameter_k->name_);
            if (_parameter_symbol_k != NULL) {
                _parameter_symbol_k->declaration_ = NULL;
                _parameter_symbol_k->is_reference_ = _parameter_k->is_reference_;
                if (_parameter_k->is_array_) {
                    uint32_t _capacity_k = 1U;
                    for (uint32_t _axis = 0U; _axis < _parameter_k->rank_; ++_axis) {
                        _capacity_k *= _parameter_k->extents_[_axis];
                    }
                    _parameter_symbol_k->is_sequence_ = true;
                    _parameter_symbol_k->capacity_ = _capacity_k;
                    _parameter_symbol_k->declaration_ = statement;
                }
            }
        }
        const char *const _previous_return_k = context->current_return_type_;
        const size_t _previous_loop_k = context->loop_depth_;
        const zlang_ast_statement_t *const _previous_generic_k =
            context->current_generic_declaration_;
        if (statement->is_generic_) {
            context->current_generic_declaration_ = statement;
        }
        context->current_return_type_ = statement->is_constructor_ ? "void" : statement->type_name_;
        context->loop_depth_ = 0U;
        _result_k = zlang_analyze_body(context, statement->body_start_, statement->body_count_);
        for (size_t _index = 0U; _index < statement->parameter_count_; ++_index) {
            const zlang_parameter_t *const _parameter_k =
                &context->program_->parameters_[statement->parameter_start_ + _index];
            const zlang_symbol_t *const _symbol_k = zlang_find_symbol(context, _parameter_k->name_);
            if (statement->is_main_ || !_parameter_k->is_reference_ || _symbol_k == NULL ||
                !zlang_is_small_value_type(context, _parameter_k->type_name_)) {
                continue;
            }
            if (_parameter_k->is_const_ || !_symbol_k->is_mutated_) {
                (void)zlang_diagnostics_add_warning(
                    context->diagnostics_, ZLANG_DIAGNOSTIC_UNNECESSARY_REFERENCE,
                    _parameter_k->line_, _parameter_k->column_,
                    _parameter_k->is_const_
                        ? "const reference to a small value type has no benefit; use const clone"
                        : "reference parameter is never mutated; use clone");
            }
        }
        context->current_return_type_ = _previous_return_k;
        context->loop_depth_ = _previous_loop_k;
        context->current_generic_declaration_ = _previous_generic_k;
        context->symbol_count_ = _scope_start_k;
        --context->scope_depth_;
        break;
    }
    case zlang_statement_block_: {
        const size_t _scope_start_k = context->symbol_count_;
        ++context->scope_depth_;
        _result_k = zlang_analyze_body(context, statement->body_start_, statement->body_count_);
        context->symbol_count_ = _scope_start_k;
        --context->scope_depth_;
        break;
    }
    default:
        break;
    }
    return _result_k;
}

static bool zlang_is_nested_in_range(const zlang_program_t *program, size_t index, size_t start,
                                     size_t count) {
    const size_t _end_k = start + count;
    for (size_t _owner = start; _owner < _end_k && _owner < program->statement_count_; ++_owner) {
        if (_owner == index) {
            continue;
        }
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_owner];
        if (index == _statement_k->initializer_statement_) {
            return true;
        }
        if (index >= _statement_k->body_start_ &&
            index < _statement_k->body_start_ + _statement_k->body_count_) {
            return true;
        }
        if (index >= _statement_k->else_start_ &&
            index < _statement_k->else_start_ + _statement_k->else_count_) {
            return true;
        }
    }
    return false;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_analyze_body(zlang_semantic_context_t *context, size_t start,
                               size_t count) { // NOLINT(misc-no-recursion)
    bool _result_k = true;
    for (size_t _index = 0U; _index < count; ++_index) {
        const size_t _statement_index_k = start + _index;
        if (_statement_index_k >= context->program_->statement_count_) {
            return false;
        }
        if (zlang_is_nested_in_range(context->program_, _statement_index_k, start, count)) {
            continue;
        }
        if (!zlang_analyze_statement(context,
                                     &context->program_->statements_[_statement_index_k])) {
            _result_k = false;
        }
    }
    return _result_k;
}

static bool zlang_is_nested(const zlang_program_t *program, size_t index) {
    for (size_t _owner = 0U; _owner < program->statement_count_; ++_owner) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_owner];
        if (index == _statement_k->initializer_statement_) {
            return true;
        }
        if (index >= _statement_k->body_start_ &&
            index < _statement_k->body_start_ + _statement_k->body_count_) {
            return true;
        }
        if (index >= _statement_k->else_start_ &&
            index < _statement_k->else_start_ + _statement_k->else_count_) {
            return true;
        }
    }
    return false;
}

static void zlang_report_unused_capacities(zlang_semantic_context_t *context) {
    for (size_t _index = 0U; _index < context->symbol_count_; ++_index) {
        const zlang_symbol_t *const _symbol_k = &context->symbols_[_index];
        if (_symbol_k->is_sequence_ && _symbol_k->has_explicit_capacity_ &&
            !_symbol_k->is_mutated_ && _symbol_k->declaration_ != NULL) {
            zlang_report_warning(context, ZLANG_DIAGNOSTIC_UNUSED_CAPACITY, _symbol_k->declaration_,
                                 "explicit array capacity is never mutated");
        }
    }
}

static bool zlang_register_imports(const zlang_program_t *program,
                                   zlang_semantic_context_t *context) {
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ != zlang_statement_import_) {
            continue;
        }
        if (zlang_is_nested(program, _index)) {
            return zlang_report(context, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, _statement_k,
                                "imports are only valid at file scope");
        }
        const zlang_std_function_t *const _function_k = zlang_std_find(
            zlang_semantic_std_domain(_statement_k->type_name_), _statement_k->name_);
        if (_function_k == NULL) {
            return zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, _statement_k,
                                "standard function is not in the registry");
        }
        const zlang_symbol_t *const _symbol_k =
            zlang_find_mutable_symbol(context, _statement_k->name_);
        if (_symbol_k != NULL) {
            if (_symbol_k->is_function_ && _symbol_k->declaration_ != NULL &&
                _symbol_k->declaration_->kind_ == zlang_statement_import_ &&
                strcmp(_symbol_k->declaration_->type_name_, _statement_k->type_name_) == 0) {
                continue;
            }
            return zlang_report(context, ZLANG_DIAGNOSTIC_DUPLICATE_DECLARATION, _statement_k,
                                "imported name conflicts with another declaration");
        }
        if (context->symbol_count_ >= context->symbol_capacity_) {
            return zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, _statement_k,
                                "symbol table capacity exceeded");
        }
        zlang_symbol_t *const _entry_k = &context->symbols_[context->symbol_count_++];
        memset(_entry_k, 0, sizeof(*_entry_k));
        _entry_k->name_ = _statement_k->name_;
        _entry_k->type_name_ = _function_k->return_type_;
        _entry_k->declaration_ = _statement_k;
        _entry_k->scope_depth_ = context->scope_depth_;
        _entry_k->is_function_ = true;
    }
    return true;
}

static bool zlang_is_inferred_array_name(const zlang_program_t *program,
                                         const zlang_ast_statement_t *function, const char *name) {
    if (name == NULL) {
        return false;
    }
    const size_t _parameter_end_k = function->parameter_start_ + function->parameter_count_;
    for (size_t _index = function->parameter_start_;
         _index < _parameter_end_k && _index < program->parameter_count_; ++_index) {
        if (program->parameters_[_index].is_array_ &&
            strcmp(program->parameters_[_index].name_, name) == 0) {
            return true;
        }
    }
    const size_t _body_end_k = function->body_start_ + function->body_count_;
    for (size_t _index = function->body_start_;
         _index < _body_end_k && _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_array_declaration_ &&
            _statement_k->is_inferred_ && _statement_k->name_ != NULL &&
            strcmp(_statement_k->name_, name) == 0) {
            return true;
        }
    }
    return false;
}

static bool zlang_validate_shape_index(const zlang_program_t *program, size_t index,
                                       const zlang_ast_statement_t *function,
                                       const zlang_inferred_shape_t *shape,
                                       zlang_diagnostics_t *diagnostics) {
    if (index == ZLANG_NO_EXPRESSION || index >= program->expression_count_) {
        return true;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    const zlang_ast_expression_t *_indexed_k = NULL;
    if (_expression_k->kind_ == zlang_expression_index_) {
        _indexed_k = _expression_k;
    } else if (_expression_k->kind_ == zlang_expression_index_assignment_ &&
               _expression_k->left_ < program->expression_count_) {
        _indexed_k = &program->expressions_[_expression_k->left_];
    }
    if (_indexed_k == NULL || _indexed_k->kind_ != zlang_expression_index_ ||
        !zlang_is_inferred_array_name(program, function, _indexed_k->name_)) {
        return true;
    }
    if (_indexed_k->argument_count_ == 0U || _indexed_k->argument_count_ > shape->rank_) {
        (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, _indexed_k->line_,
                                    _indexed_k->column_,
                                    "array index rank does not match inferred shape");
        return false;
    }
    for (size_t _axis = 0U; _axis < _indexed_k->argument_count_; ++_axis) {
        const size_t _argument_k = _indexed_k->argument_indices_[_axis];
        if (_argument_k >= program->expression_count_) {
            return false;
        }
        const zlang_ast_expression_t *const _argument_expression_k =
            &program->expressions_[_argument_k];
        if (_argument_expression_k->kind_ != zlang_expression_integer_literal_) {
            continue;
        }
        const int64_t _value_k =
            _argument_expression_k->is_unsigned_ || _argument_expression_k->is_uint64_
                ? (int64_t)_argument_expression_k->integer64_value_
                : (int64_t)_argument_expression_k->integer_value_;
        if (_value_k < 0 || (uint64_t)_value_k >= (uint64_t)shape->extents_[_axis]) {
            (void)zlang_diagnostics_add(
                diagnostics, ZLANG_DIAGNOSTIC_UNBOUNDED_SEQUENCE, _argument_expression_k->line_,
                _argument_expression_k->column_, "literal index exceeds inferred array extent");
            return false;
        }
    }
    return true;
}

static bool zlang_validate_inferred_shapes(const zlang_program_t *program,
                                           zlang_diagnostics_t *diagnostics) {
    for (size_t _shape = 0U; _shape < program->inferred_shape_count_; ++_shape) {
        const zlang_inferred_shape_t *const _shape_k = &program->inferred_shapes_[_shape];
        if (_shape_k->function_statement_ >= program->statement_count_) {
            return false;
        }
        const zlang_ast_statement_t *const _function_k =
            &program->statements_[_shape_k->function_statement_];
        for (size_t _index = 0U; _index < program->expression_count_; ++_index) {
            if (!zlang_validate_shape_index(program, _index, _function_k, _shape_k, diagnostics)) {
                return false;
            }
        }
    }
    return true;
}

static bool zlang_validate_generic_declaration(zlang_semantic_context_t *context,
                                               const zlang_ast_statement_t *statement) {
    bool _result_k = true;
    if (!statement->is_generic_ || statement->generic_parameter_count_ == 0U) {
        return true;
    }
    for (size_t _index = 0U; _index < statement->generic_parameter_count_; ++_index) {
        const char *const _parameter_k = statement->generic_parameters_[_index];
        if (_parameter_k == NULL) {
            continue;
        }
        for (size_t _other = 0U; _other < _index; ++_other) {
            const char *const _other_k = statement->generic_parameters_[_other];
            if (_other_k != NULL && strcmp(_parameter_k, _other_k) == 0) {
                _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_DUPLICATE_DECLARATION, statement,
                                         "generic parameter is declared twice");
            }
        }
    }
    if (statement->kind_ == zlang_statement_struct_) {
        if (statement->base_name_ != NULL || statement->uses_count_ > 0U) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, statement,
                                     "generic structs cannot inherit or use traits");
        }
    } else if (statement->kind_ == zlang_statement_function_) {
        if (statement->owner_name_ != NULL || statement->is_main_) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, statement,
                                     "generic methods and generic main are not supported");
        }
    }
    return _result_k;
}

static bool zlang_validate_generic_applications(zlang_semantic_context_t *context) {
    const zlang_program_t *const _program_k = context->program_;
    bool _result_k = true;
    for (size_t _index = 0U; _index < _program_k->generic_application_count_; ++_index) {
        const zlang_generic_application_t *const _application_k =
            &_program_k->generic_applications_[_index];
        if (zlang_std_find_result(_application_k->name_) != NULL) {
            continue;
        }
        const zlang_ast_statement_t *const _declaration_k = zlang_generic_find_declaration(
            _program_k, _application_k->base_name_, _application_k->is_function_);
        if (_declaration_k == NULL) {
            (void)zlang_diagnostics_add(
                context->diagnostics_, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, _application_k->line_,
                _application_k->column_, "generic declaration is not defined");
            _result_k = false;
            continue;
        }
        if (_application_k->argument_count_ != _declaration_k->generic_parameter_count_) {
            (void)zlang_diagnostics_add(context->diagnostics_, ZLANG_DIAGNOSTIC_TYPE_MISMATCH,
                                        _application_k->line_, _application_k->column_,
                                        "generic argument count does not match the declaration");
            _result_k = false;
            continue;
        }
        for (size_t _argument = 0U; _argument < _application_k->argument_count_; ++_argument) {
            const char *const _type_k = _application_k->arguments_[_argument];
            if (!zlang_is_builtin_type(_type_k) && !zlang_is_user_type(_program_k, _type_k)) {
                (void)zlang_diagnostics_add(context->diagnostics_,
                                            ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER,
                                            _application_k->line_, _application_k->column_,
                                            "generic argument is not a known type");
                _result_k = false;
            }
        }
    }
    return _result_k;
}

static bool zlang_validate_jumps(const zlang_program_t *program, zlang_diagnostics_t *diagnostics) {
    bool _result_k = true;
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ != zlang_statement_jump_) {
            continue;
        }
        if (zlang_find_label(program, _statement_k->name_) == NULL) {
            (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER,
                                        _statement_k->line_, _statement_k->column_,
                                        "jump target is not a declared label");
            _result_k = false;
        }
    }
    return _result_k;
}

bool zlang_analyze_program(const zlang_program_t *program, zlang_semantic_context_t *context) {
    context->program_ = program;
    if (!zlang_register_imports(program, context) ||
        !zlang_validate_jumps(program, context->diagnostics_) ||
        !zlang_validate_generic_applications(context) ||
        !zlang_validate_inferred_shapes(program, context->diagnostics_)) {
        return false;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_function_ && !zlang_is_nested(program, _index) &&
            !zlang_add_function_symbol(context, _statement_k)) {
            return false;
        }
    }
    bool _result_k = true;
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ != zlang_statement_import_ && !zlang_is_nested(program, _index) &&
            !zlang_analyze_statement(context, _statement_k)) {
            _result_k = false;
        }
    }
    zlang_report_unused_capacities(context);
    return _result_k && !context->has_error_;
}
