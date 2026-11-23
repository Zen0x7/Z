#include "zlang/compiler/semantics.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

void zlang_semantic_context_init(zlang_semantic_context_t *context, zlang_symbol_t *symbols,
                                 size_t symbol_capacity, zlang_diagnostics_t *diagnostics) {
    context->symbols_ = symbols;
    context->symbol_capacity_ = symbol_capacity;
    context->symbol_count_ = 0U;
    context->diagnostics_ = diagnostics;
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

static bool zlang_type_accepts(const zlang_semantic_context_t *context, const char *type_name,
                               const zlang_ast_expression_t *initializer) {
    if (type_name == NULL || initializer == NULL) {
        return false;
    }
    switch (initializer->kind_) {
    case zlang_ast_expression_integer_literal_:
        return initializer->is_unsigned_ ? strcmp(type_name, "unsigned") == 0
                                         : strcmp(type_name, "integer") == 0;
    case zlang_ast_expression_string_literal_:
        return strcmp(type_name, "string") == 0;
    case zlang_ast_expression_identifier_: {
        const zlang_symbol_t *const _symbol_k = zlang_find_symbol(context, initializer->name_);
        return _symbol_k != NULL && strcmp(_symbol_k->type_name_, type_name) == 0;
    }
    default:
        return false;
    }
}

static bool zlang_analyze_initializer(zlang_semantic_context_t *context,
                                      const zlang_ast_statement_t *statement,
                                      const char *expected_type) {
    const zlang_ast_expression_t *const _initializer_k = statement->initializer_;
    if (_initializer_k == NULL) {
        return true;
    }
    if (_initializer_k->kind_ == zlang_ast_expression_identifier_ &&
        zlang_find_symbol(context, _initializer_k->name_) == NULL) {
        return zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                            "initializer refers to an undeclared identifier");
    }
    if (expected_type != NULL && !zlang_type_accepts(context, expected_type, _initializer_k)) {
        return zlang_report(context, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, statement,
                            "initializer type does not match the declared type");
    }
    return true;
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

static bool zlang_analyze_statement(zlang_semantic_context_t *context,
                                    const zlang_ast_statement_t *statement) {
    bool _result_k = true;
    switch (statement->kind_) {
    case zlang_ast_statement_declaration_:
        _result_k = zlang_analyze_initializer(context, statement, statement->type_name_);
        if (_result_k) {
            _result_k = zlang_add_symbol(context, statement, 0U, false);
        }
        break;
    case zlang_ast_statement_sequence_declaration_:
        if (statement->capacity_ == 0U) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNBOUNDED_SEQUENCE, statement,
                                     "sequence declaration requires a fixed capacity");
            break;
        }
        _result_k = zlang_add_symbol(context, statement, statement->capacity_, true);
        break;
    case zlang_ast_statement_print_:
        _result_k = zlang_analyze_initializer(context, statement, NULL);
        break;
    case zlang_ast_statement_foreach_: {
        if (zlang_find_symbol(context, statement->iterable_name_) == NULL) {
            _result_k = zlang_report(context, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, statement,
                                     "foreach iterable refers to an undeclared name");
            break;
        }
        const size_t _scope_start_k = context->symbol_count_;
        const zlang_ast_statement_t _item_k = {
            .kind_ = zlang_ast_statement_declaration_,
            .type_name_ = "integer",
            .name_ = statement->name_,
            .line_ = statement->line_,
            .column_ = statement->column_,
        };
        if (!zlang_add_symbol(context, &_item_k, 0U, false)) {
            break;
        }
        for (size_t _index = 0U; _index < statement->body_count_; ++_index) {
            if (!zlang_analyze_initializer(context, &statement->body_[_index], NULL)) {
                _result_k = false;
            }
        }
        context->symbol_count_ = _scope_start_k;
        break;
    }
    default:
        break;
    }
    return _result_k;
}

bool zlang_analyze_program(const zlang_program_t *program, zlang_semantic_context_t *context) {
    bool _result_k = true;
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        if (!zlang_analyze_statement(context, &program->statements_[_index])) {
            _result_k = false;
        }
    }
    return _result_k;
}
