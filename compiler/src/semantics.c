/** @file
 * @brief Scope, binding, type and resource checks for Zlang.
 */

#include "zlang/compiler/semantics.h"

#include <string.h>

void zlang_semantic_context_init(zlang_semantic_context_t *context, zlang_symbol_t *symbols,
                                 size_t symbol_capacity, zlang_diagnostics_t *diagnostics,
                                 const zlang_program_t *program) {
    context->symbols_ = symbols;
    context->symbol_capacity_ = symbol_capacity;
    context->symbol_count_ = 0U;
    context->scope_depth_ = 0U;
    context->loop_depth_ = 0U;
    context->current_return_type_ = NULL;
    context->has_error_ = false;
    context->diagnostics_ = diagnostics;
    context->program_ = program;
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

static const zlang_ast_statement_t *zlang_find_record(const zlang_program_t *program,
                                                      const char *name) {
    if (program == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_record_ && _statement_k->name_ != NULL &&
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

static bool zlang_is_user_type(const zlang_program_t *program, const char *name) {
    return zlang_find_record(program, name) != NULL || zlang_find_enum(program, name) != NULL;
}

static bool zlang_is_builtin_type(const char *name) {
    return name != NULL && (strcmp(name, "integer") == 0 || strcmp(name, "unsigned") == 0 ||
                            strcmp(name, "boolean") == 0 || strcmp(name, "string") == 0);
}

static bool zlang_is_known_type(const zlang_program_t *program, const char *name) {
    return zlang_is_builtin_type(name) || zlang_is_user_type(program, name);
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

static const zlang_ast_statement_t *zlang_find_field(const zlang_program_t *program,
                                                     const zlang_ast_statement_t *record,
                                                     const char *name) {
    if (record == NULL || name == NULL) {
        return NULL;
    }
    const size_t _end_k = record->body_start_ + record->body_count_;
    for (size_t _index = record->body_start_; _index < _end_k && _index < program->statement_count_;
         ++_index) {
        const zlang_ast_statement_t *const _field_k = &program->statements_[_index];
        if ((_field_k->kind_ == zlang_statement_declaration_ ||
             _field_k->kind_ == zlang_statement_array_declaration_) &&
            _field_k->name_ != NULL && strcmp(_field_k->name_, name) == 0) {
            return _field_k;
        }
    }
    return NULL;
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
    _symbol_k->declaration_ = statement;
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
        return _expression_k->is_unsigned_ ? "unsigned" : "integer";
    case zlang_expression_boolean_literal_:
        return "boolean";
    case zlang_expression_string_literal_:
        return "string";
    case zlang_expression_identifier_: {
        const zlang_symbol_t *const _symbol_k = zlang_find_symbol(context, _expression_k->name_);
        if (_symbol_k != NULL) {
            return _symbol_k->type_name_;
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
        (void)zlang_analyze_expression(context, _expression_k->right_, statement);
        return _left_k;
    }
    case zlang_expression_negative_: {
        const char *const _actual_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        if (!zlang_type_matches("integer", _actual_k)) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "unary negation requires an integer operand");
            return NULL;
        }
        return "integer";
    }
    case zlang_expression_sub_:
    case zlang_expression_mul_:
    case zlang_expression_div_:
    case zlang_expression_mod_: {
        const char *const _left_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        const char *const _right_k =
            zlang_analyze_expression(context, _expression_k->right_, statement);
        const bool _numeric_k =
            zlang_type_matches("integer", _left_k) || zlang_type_matches("unsigned", _left_k);
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
        const bool _numeric_k = zlang_type_matches("integer", _symbol_k->type_name_) ||
                                zlang_type_matches("unsigned", _symbol_k->type_name_);
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
        return "unsigned";
    }
    case zlang_expression_equal_:
    case zlang_expression_less_:
    case zlang_expression_greater_:
    case zlang_expression_less_equal_:
    case zlang_expression_greater_equal_:
    case zlang_expression_not_equal_:
        (void)zlang_analyze_expression(context, _expression_k->left_, statement);
        (void)zlang_analyze_expression(context, _expression_k->right_, statement);
        return "boolean";
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
        if (!zlang_type_matches(_symbol_k->type_name_, _actual_k)) {
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
    case zlang_expression_print_call_: {
        const char *const _actual_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        const bool _printable_k =
            zlang_type_matches("string", _actual_k) || zlang_type_matches("integer", _actual_k) ||
            zlang_type_matches("unsigned", _actual_k) || zlang_type_matches("boolean", _actual_k) ||
            zlang_is_enum_type(context->program_, _actual_k);
        if (!_printable_k) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, index, statement,
                                          "print argument must be string or scalar");
            return NULL;
        }
        return "void";
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
        const zlang_ast_statement_t *const _record_k =
            zlang_find_record(context->program_, _record_name_k);
        const zlang_ast_statement_t *const _field_k =
            zlang_find_field(context->program_, _record_k, _expression_k->name_);
        if (_field_k == NULL) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement, "field is not declared on the record");
            return NULL;
        }
        if (_field_k->kind_ == zlang_statement_array_declaration_ &&
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
        return _field_k->type_name_;
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
        const zlang_ast_statement_t *const _record_k =
            zlang_find_record(context->program_, _record_name_k);
        const zlang_ast_statement_t *const _field_k =
            zlang_find_field(context->program_, _record_k, _target_k->name_);
        if (_field_k == NULL) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement, "field is not declared on the record");
            return NULL;
        }
        if (_field_k->kind_ == zlang_statement_array_declaration_ &&
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
        if (!zlang_type_matches(_field_k->type_name_, _actual_k)) {
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
        return _is_string_k ? "unsigned" : _target_k->type_name_;
    }
    case zlang_expression_call_: {
        const zlang_symbol_t *const _function_k = zlang_find_symbol(context, _expression_k->name_);
        if (_function_k == NULL || !_function_k->is_function_) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement, "call target is not a declared function");
            return NULL;
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
            if (!zlang_type_matches(_expected_k, _actual_k)) {
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

static bool zlang_analyze_body(zlang_semantic_context_t *context, size_t start, size_t count);

static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_analyze_statement(zlang_semantic_context_t *context,
                        const zlang_ast_statement_t *statement) { // NOLINT(misc-no-recursion)
    bool _result_k = true;
    switch (statement->kind_) {
    case zlang_statement_record_: {
        const size_t _end_k = statement->body_start_ + statement->body_count_;
        for (size_t _index = statement->body_start_;
             _index < _end_k && _index < context->program_->statement_count_; ++_index) {
            const zlang_ast_statement_t *const _field_k = &context->program_->statements_[_index];
            const bool _is_array_field_k = _field_k->kind_ == zlang_statement_array_declaration_;
            if ((!_is_array_field_k && _field_k->kind_ != zlang_statement_declaration_) ||
                !zlang_is_known_type(context->program_, _field_k->type_name_)) {
                _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, _field_k,
                                         "record field has an unknown type");
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
                                         "record fields must be plain scalar or enum declarations");
                continue;
            }
            const zlang_ast_statement_t *const _previous_k =
                zlang_find_field(context->program_, statement, _field_k->name_);
            if (_previous_k != NULL && _previous_k != _field_k) {
                _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_DUPLICATE_DECLARATION, _field_k,
                                         "record field is declared twice");
            }
        }
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
        if (!zlang_is_known_type(context->program_, statement->type_name_)) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                                     "declaration uses an unknown type");
            break;
        }
        const char *const _actual_k =
            zlang_analyze_expression(context, statement->expression_, statement);
        if (statement->expression_ != ZLANG_NO_EXPRESSION &&
            !zlang_type_matches(statement->type_name_, _actual_k)) {
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
        if (!zlang_is_known_type(context->program_, statement->type_name_)) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                                     "array uses an unknown element type");
            break;
        }
        if (zlang_find_record(context->program_, statement->type_name_) != NULL &&
            statement->initializer_count_ > 0U) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, statement,
                                     "record arrays do not support initializer lists");
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
        } else if (!zlang_type_matches(context->current_return_type_, _actual_k)) {
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
        if (!zlang_type_matches("unsigned", _actual_k)) {
            _result_k = zlang_report_expression(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH,
                                                statement->expression_, statement,
                                                "repeat count must be unsigned");
        }
        ++context->loop_depth_;
        _result_k = zlang_analyze_body(context, statement->body_start_, statement->body_count_) &&
                    _result_k;
        --context->loop_depth_;
        break;
    }
    case zlang_statement_break_:
    case zlang_statement_continue_:
        if (context->loop_depth_ == 0U) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, statement,
                                     statement->kind_ == zlang_statement_break_
                                         ? "break is only valid inside a loop"
                                         : "continue is only valid inside a loop");
        }
        break;
    case zlang_statement_foreach_: {
        const zlang_symbol_t *const _iterable_k =
            zlang_find_symbol(context, statement->iterable_name_);
        if (_iterable_k == NULL || !_iterable_k->is_sequence_) {
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
                    .type_name_ = "unsigned",
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
                .type_name_ = "unsigned",
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
        if (!zlang_add_function_symbol(context, statement)) {
            return false;
        }
        const size_t _scope_start_k = context->symbol_count_;
        ++context->scope_depth_;
        for (size_t _index = 0U; _index < statement->parameter_count_; ++_index) {
            if (statement->parameter_start_ + _index >= context->program_->parameter_count_) {
                --context->scope_depth_;
                return zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                                    "function parameter table is incomplete");
            }
            const zlang_parameter_t *const _parameter_k =
                &context->program_->parameters_[statement->parameter_start_ + _index];
            const zlang_ast_statement_t _parameter_statement_k = {
                .kind_ = zlang_statement_declaration_,
                .type_name_ = _parameter_k->type_name_,
                .name_ = _parameter_k->name_,
                .line_ = _parameter_k->line_,
                .column_ = _parameter_k->column_,
                .is_const_ = _parameter_k->is_const_,
            };
            if (!zlang_add_symbol(context, &_parameter_statement_k, 0U, false, false)) {
                context->symbol_count_ = _scope_start_k;
                --context->scope_depth_;
                return false;
            }
            zlang_symbol_t *const _parameter_symbol_k =
                zlang_find_mutable_symbol(context, _parameter_k->name_);
            if (_parameter_symbol_k != NULL) {
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
        context->current_return_type_ = statement->type_name_;
        context->loop_depth_ = 0U;
        _result_k = zlang_analyze_body(context, statement->body_start_, statement->body_count_);
        for (size_t _index = 0U; _index < statement->parameter_count_; ++_index) {
            const zlang_parameter_t *const _parameter_k =
                &context->program_->parameters_[statement->parameter_start_ + _index];
            const zlang_symbol_t *const _symbol_k = zlang_find_symbol(context, _parameter_k->name_);
            if (!_parameter_k->is_reference_ || _symbol_k == NULL ||
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

bool zlang_analyze_program(const zlang_program_t *program, zlang_semantic_context_t *context) {
    context->program_ = program;
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
        if (!zlang_is_nested(program, _index) && !zlang_analyze_statement(context, _statement_k)) {
            _result_k = false;
        }
    }
    zlang_report_unused_capacities(context);
    return _result_k && !context->has_error_;
}
