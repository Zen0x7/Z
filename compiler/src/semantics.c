#include "zlang/compiler/semantics.h"

#include <string.h>

void zlang_semantic_context_init(zlang_semantic_context_t *context, zlang_symbol_t *symbols,
                                 size_t symbol_capacity, zlang_diagnostics_t *diagnostics,
                                 const zlang_program_t *program) {
    context->symbols_ = symbols;
    context->symbol_capacity_ = symbol_capacity;
    context->symbol_count_ = 0U;
    context->diagnostics_ = diagnostics;
    context->program_ = program;
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

static bool zlang_report(zlang_semantic_context_t *context, uint16_t code,
                         const zlang_ast_statement_t *statement, const char *message) {
    (void)zlang_diagnostics_add(context->diagnostics_, code, statement->line_, statement->column_,
                                message);
    return false;
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

    (void)zlang_diagnostics_add(context->diagnostics_, code, _line_k, _column_k, message);
    return false;
}

static bool zlang_add_symbol(zlang_semantic_context_t *context,
                             const zlang_ast_statement_t *statement, uint32_t capacity,
                             bool is_sequence) {
    if (zlang_find_symbol(context, statement->name_) != NULL) {
        return zlang_report(context, ZLANG_DIAGNOSTIC_DUPLICATE_DECLARATION, statement,
                            "declaration redefines an existing name");
    }
    if (context->symbol_count_ >= context->symbol_capacity_) {
        return zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                            "symbol table capacity exceeded");
    }
    zlang_symbol_t *const _symbol_k = &context->symbols_[context->symbol_count_++];
    _symbol_k->name_ = statement->name_;
    _symbol_k->type_name_ = statement->type_name_;
    _symbol_k->capacity_ = capacity;
    _symbol_k->is_sequence_ = is_sequence;
    return true;
}

static const char *zlang_analyze_expression(zlang_semantic_context_t *context, size_t index,
                                            const zlang_ast_statement_t *statement);

/* Expression trees are bounded by the caller-owned expression array. */
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
        if (_symbol_k == NULL) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement,
                                          "expression refers to an undeclared identifier");
            return NULL;
        }
        return _symbol_k->type_name_;
    }
    case zlang_expression_add_: {
        const char *const _left_k =
            zlang_analyze_expression(context, _expression_k->left_, statement);
        (void)zlang_analyze_expression(context, _expression_k->right_, statement);
        return _left_k;
    }
    case zlang_expression_equal_:
        (void)zlang_analyze_expression(context, _expression_k->left_, statement);
        (void)zlang_analyze_expression(context, _expression_k->right_, statement);
        return "boolean";
    case zlang_expression_assignment_: {
        const zlang_ast_expression_t *const _left_k =
            index < context->program_->expression_count_
                ? &context->program_->expressions_[_expression_k->left_]
                : NULL;
        if (_left_k == NULL || _left_k->kind_ != zlang_expression_identifier_ ||
            zlang_find_symbol(context, _left_k->name_) == NULL) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement,
                                          "assignment target is not a declared identifier");
            return NULL;
        }
        return zlang_analyze_expression(context, _expression_k->right_, statement);
    }
    case zlang_expression_print_call_:
        (void)zlang_analyze_expression(context, _expression_k->left_, statement);
        return "void";
    case zlang_expression_append_call_: {
        const zlang_symbol_t *const _target_k = zlang_find_symbol(context, _expression_k->name_);
        if (_target_k == NULL || !_target_k->is_sequence_) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement, "append target is not a declared sequence");
            return NULL;
        }
        (void)zlang_analyze_expression(context, _expression_k->right_, statement);
        return "void";
    }
    case zlang_expression_index_: {
        const zlang_symbol_t *const _target_k =
            _expression_k->left_ < context->program_->expression_count_
                ? zlang_find_symbol(context,
                                    context->program_->expressions_[_expression_k->left_].name_)
                : NULL;
        if (_target_k == NULL || !_target_k->is_sequence_) {
            (void)zlang_report_expression(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, index,
                                          statement, "index target is not a declared sequence");
            return NULL;
        }
        (void)zlang_analyze_expression(context, _expression_k->right_, statement);
        return _target_k->type_name_;
    }
    default:
        return NULL;
    }
}

static bool zlang_type_matches(const char *expected, const char *actual) {
    return expected != NULL && actual != NULL && strcmp(expected, actual) == 0;
}

static bool zlang_analyze_body(zlang_semantic_context_t *context, size_t start, size_t count);

static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_analyze_statement(zlang_semantic_context_t *context,
                        const zlang_ast_statement_t *statement) { // NOLINT(misc-no-recursion)
    bool _result_k = true;
    switch (statement->kind_) {
    case zlang_statement_declaration_: {
        const char *const _actual_k =
            zlang_analyze_expression(context, statement->expression_, statement);
        if (statement->expression_ != ZLANG_NO_EXPRESSION &&
            !zlang_type_matches(statement->type_name_, _actual_k)) {
            _result_k = zlang_report_expression(
                context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, statement->expression_, statement,
                "initializer type does not match the declared type");
        }
        if (_result_k) {
            _result_k = zlang_add_symbol(context, statement, 0U, false);
        }
        break;
    }
    case zlang_statement_sequence_declaration_:
        if (statement->capacity_ == 0U) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNBOUNDED_SEQUENCE, statement,
                                     "sequence declaration requires a fixed capacity");
        } else {
            _result_k = zlang_add_symbol(context, statement, statement->capacity_, true);
        }
        break;
    case zlang_statement_expression_:
    case zlang_statement_return_:
        (void)zlang_analyze_expression(context, statement->expression_, statement);
        break;
    case zlang_statement_if_:
    case zlang_statement_while_:
    case zlang_statement_repeat_:
        (void)zlang_analyze_expression(context, statement->expression_, statement);
        _result_k = zlang_analyze_body(context, statement->body_start_, statement->body_count_);
        if (_result_k && statement->else_count_ > 0U) {
            _result_k = zlang_analyze_body(context, statement->else_start_, statement->else_count_);
        }
        break;
    case zlang_statement_foreach_: {
        if (zlang_find_symbol(context, statement->iterable_name_) == NULL) {
            return zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                                "foreach iterable refers to an undeclared name");
        }
        const size_t _scope_start_k = context->symbol_count_;
        const zlang_ast_statement_t _item_k = {
            .kind_ = zlang_statement_declaration_,
            .type_name_ = "integer",
            .name_ = statement->name_,
            .line_ = statement->line_,
            .column_ = statement->column_,
        };
        if (!zlang_add_symbol(context, &_item_k, 0U, false)) {
            return false;
        }
        _result_k = zlang_analyze_body(context, statement->body_start_, statement->body_count_);
        context->symbol_count_ = _scope_start_k;
        break;
    }
    case zlang_statement_function_: {
        const size_t _scope_start_k = context->symbol_count_;
        for (size_t _index = 0U; _index < statement->parameter_count_; ++_index) {
            const zlang_parameter_t *const _parameter_k =
                &context->program_->parameters_[statement->parameter_start_ + _index];
            const zlang_ast_statement_t _parameter_statement_k = {
                .kind_ = zlang_statement_declaration_,
                .type_name_ = _parameter_k->type_name_,
                .name_ = _parameter_k->name_,
                .line_ = _parameter_k->line_,
                .column_ = _parameter_k->column_,
            };
            if (!zlang_add_symbol(context, &_parameter_statement_k, 0U, false)) {
                context->symbol_count_ = _scope_start_k;
                return false;
            }
        }
        _result_k = zlang_analyze_body(context, statement->body_start_, statement->body_count_);
        context->symbol_count_ = _scope_start_k;
        break;
    }
    case zlang_statement_block_: {
        const size_t _scope_start_k = context->symbol_count_;
        _result_k = zlang_analyze_body(context, statement->body_start_, statement->body_count_);
        context->symbol_count_ = _scope_start_k;
        break;
    }
    default:
        break;
    }
    return _result_k;
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
        if (!zlang_analyze_statement(context,
                                     &context->program_->statements_[_statement_index_k])) {
            _result_k = false;
        }
    }
    return _result_k;
}

bool zlang_analyze_program(const zlang_program_t *program, zlang_semantic_context_t *context) {
    context->program_ = program;
    return zlang_analyze_body(context, 0U, program->statement_count_);
}
