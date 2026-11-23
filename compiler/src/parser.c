/** @file
 * @brief Recursive-descent parser for the Zlang 0.1 grammar.
 */

#include "zlang/compiler/parser.h"

#include <string.h>

static const zlang_token_t *zlang_parser_current(const zlang_parser_t *parser) {
    return &parser->tokens_[parser->position_];
}

static void zlang_parser_advance(zlang_parser_t *parser) {
    if (zlang_parser_current(parser)->kind_ != zlang_token_end_of_file_) {
        ++parser->position_;
    }
}

static bool zlang_parser_expect(zlang_parser_t *parser, const zlang_token_kind_t kind,
                                const char *message) {
    if (zlang_parser_current(parser)->kind_ == kind) {
        zlang_parser_advance(parser);
        return true;
    }
    (void)zlang_diagnostics_add(parser->diagnostics_, ZLANG_DIAGNOSTIC_UNEXPECTED_TOKEN,
                                zlang_parser_current(parser)->line_,
                                zlang_parser_current(parser)->column_, message);
    parser->failed_ = true;
    return false;
}

static void zlang_parser_unsupported(zlang_parser_t *parser, const char *message) {
    (void)zlang_diagnostics_add(parser->diagnostics_, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT,
                                zlang_parser_current(parser)->line_,
                                zlang_parser_current(parser)->column_, message);
    parser->failed_ = true;
}

static size_t zlang_parser_add_expression(zlang_parser_t *parser) {
    if (parser->expression_count_ >= parser->expression_capacity_) {
        zlang_parser_unsupported(parser, "expression capacity exceeded");
        return ZLANG_NO_EXPRESSION;
    }
    zlang_ast_expression_t *const _expression_k = &parser->expressions_[parser->expression_count_];
    memset(_expression_k, 0, sizeof(*_expression_k));
    _expression_k->left_ = ZLANG_NO_EXPRESSION;
    _expression_k->right_ = ZLANG_NO_EXPRESSION;
    _expression_k->line_ = zlang_parser_current(parser)->line_;
    _expression_k->column_ = zlang_parser_current(parser)->column_;
    return parser->expression_count_++;
}

static size_t zlang_parser_add_statement(zlang_parser_t *parser) {
    if (parser->statement_count_ >= parser->statement_capacity_) {
        zlang_parser_unsupported(parser, "statement capacity exceeded");
        return (size_t)-1;
    }
    zlang_ast_statement_t *const _statement_k = &parser->statements_[parser->statement_count_];
    memset(_statement_k, 0, sizeof(*_statement_k));
    _statement_k->expression_ = ZLANG_NO_EXPRESSION;
    return parser->statement_count_++;
}

static bool zlang_token_is_type(const zlang_token_t *token) {
    return token->kind_ == zlang_token_integer_ || token->kind_ == zlang_token_unsigned_ ||
           token->kind_ == zlang_token_boolean_ || token->kind_ == zlang_token_string_;
}

static bool zlang_parse_expression(zlang_parser_t *parser, size_t *expression);
static bool zlang_parse_if(zlang_parser_t *parser);
static bool zlang_parse_simple_block_statement(zlang_parser_t *parser, zlang_statement_kind_t kind);
static bool zlang_parse_foreach(zlang_parser_t *parser);
static bool zlang_parse_function(zlang_parser_t *parser);
static bool zlang_parse_return(zlang_parser_t *parser);
static bool zlang_parse_block(zlang_parser_t *parser, size_t statement);
static bool zlang_parse_statements(zlang_parser_t *parser);

/* Recursive descent mirrors Annex A directly. NOLINT keeps the policy check
   local: nesting is bounded by the caller-owned AST capacity. */
// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_primary(zlang_parser_t *parser,
                                size_t *expression) { // NOLINT(misc-no-recursion)
    const zlang_token_t *const _token_k = zlang_parser_current(parser);
    size_t _expression_k = ZLANG_NO_EXPRESSION;

    switch (_token_k->kind_) {
    case zlang_token_integer_literal_:
        _expression_k = zlang_parser_add_expression(parser);
        if (_expression_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_expression_k].kind_ = zlang_expression_integer_literal_;
        parser->expressions_[_expression_k].integer_value_ = _token_k->integer_value_;
        parser->expressions_[_expression_k].is_unsigned_ = _token_k->is_unsigned_;
        zlang_parser_advance(parser);
        break;
    case zlang_token_true_:
    case zlang_token_false_:
        _expression_k = zlang_parser_add_expression(parser);
        if (_expression_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_expression_k].kind_ = zlang_expression_boolean_literal_;
        parser->expressions_[_expression_k].bool_value_ = _token_k->kind_ == zlang_token_true_;
        zlang_parser_advance(parser);
        break;
    case zlang_token_string_literal_:
        _expression_k = zlang_parser_add_expression(parser);
        if (_expression_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_expression_k].kind_ = zlang_expression_string_literal_;
        parser->expressions_[_expression_k].string_.data_ = _token_k->text_;
        parser->expressions_[_expression_k].string_.length_ = _token_k->value_length_;
        zlang_parser_advance(parser);
        break;
    case zlang_token_print_: {
        zlang_parser_advance(parser);
        if (!zlang_parser_expect(parser, zlang_token_left_paren_, "expected '(' after print")) {
            return false;
        }
        size_t _argument_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_expression(parser, &_argument_k)) {
            return false;
        }
        if (!zlang_parser_expect(parser, zlang_token_right_paren_,
                                 "expected ')' after print argument")) {
            return false;
        }
        _expression_k = zlang_parser_add_expression(parser);
        if (_expression_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_expression_k].kind_ = zlang_expression_print_call_;
        parser->expressions_[_expression_k].left_ = _argument_k;
        break;
    }
    case zlang_token_identifier_: {
        const bool _is_append_k = zlang_token_is(_token_k, "append");
        if (_is_append_k && parser->position_ + 1U < parser->token_count_ &&
            parser->tokens_[parser->position_ + 1U].kind_ == zlang_token_left_paren_) {
            zlang_parser_advance(parser);
            zlang_parser_advance(parser);
            const zlang_token_t *const _target_k = zlang_parser_current(parser);
            if (_target_k->kind_ != zlang_token_identifier_) {
                zlang_parser_unsupported(parser, "append expects a sequence name");
                return false;
            }
            zlang_parser_advance(parser);
            if (!zlang_parser_expect(parser, zlang_token_comma_,
                                     "expected ',' after append target")) {
                return false;
            }
            size_t _value_k = ZLANG_NO_EXPRESSION;
            if (!zlang_parse_expression(parser, &_value_k)) {
                return false;
            }
            if (!zlang_parser_expect(parser, zlang_token_right_paren_,
                                     "expected ')' after append arguments")) {
                return false;
            }
            _expression_k = zlang_parser_add_expression(parser);
            if (_expression_k == ZLANG_NO_EXPRESSION) {
                return false;
            }
            parser->expressions_[_expression_k].kind_ = zlang_expression_append_call_;
            parser->expressions_[_expression_k].name_ = _target_k->text_;
            parser->expressions_[_expression_k].right_ = _value_k;
            break;
        }

        zlang_parser_advance(parser);
        _expression_k = zlang_parser_add_expression(parser);
        if (_expression_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_expression_k].kind_ = zlang_expression_identifier_;
        parser->expressions_[_expression_k].name_ = _token_k->text_;
        parser->expressions_[_expression_k].line_ = _token_k->line_;
        parser->expressions_[_expression_k].column_ = _token_k->column_;

        if (zlang_parser_current(parser)->kind_ == zlang_token_left_bracket_) {
            zlang_parser_advance(parser);
            size_t _index_k = ZLANG_NO_EXPRESSION;
            if (!zlang_parse_expression(parser, &_index_k)) {
                return false;
            }
            if (!zlang_parser_expect(parser, zlang_token_right_bracket_,
                                     "expected ']' after index expression")) {
                return false;
            }
            const size_t _identifier_k = _expression_k;
            _expression_k = zlang_parser_add_expression(parser);
            if (_expression_k == ZLANG_NO_EXPRESSION) {
                return false;
            }
            parser->expressions_[_expression_k].kind_ = zlang_expression_index_;
            parser->expressions_[_expression_k].left_ = _identifier_k;
            parser->expressions_[_expression_k].right_ = _index_k;
        }
        break;
    }
    default:
        zlang_parser_unsupported(parser, "unsupported expression");
        return false;
    }

    *expression = _expression_k;
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_additive(zlang_parser_t *parser, size_t *expression) {
    size_t _left_k = ZLANG_NO_EXPRESSION;
    if (!zlang_parse_primary(parser, &_left_k)) {
        return false;
    }
    while (zlang_parser_current(parser)->kind_ == zlang_token_plus_) {
        zlang_parser_advance(parser);
        size_t _right_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_primary(parser, &_right_k)) {
            return false;
        }
        const size_t _binary_k = zlang_parser_add_expression(parser);
        if (_binary_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_binary_k].kind_ = zlang_expression_add_;
        parser->expressions_[_binary_k].left_ = _left_k;
        parser->expressions_[_binary_k].right_ = _right_k;
        _left_k = _binary_k;
    }
    *expression = _left_k;
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_equality(zlang_parser_t *parser, size_t *expression) {
    size_t _left_k = ZLANG_NO_EXPRESSION;
    if (!zlang_parse_additive(parser, &_left_k)) {
        return false;
    }
    while (zlang_parser_current(parser)->kind_ == zlang_token_equal_equal_) {
        zlang_parser_advance(parser);
        size_t _right_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_additive(parser, &_right_k)) {
            return false;
        }
        const size_t _binary_k = zlang_parser_add_expression(parser);
        if (_binary_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_binary_k].kind_ = zlang_expression_equal_;
        parser->expressions_[_binary_k].left_ = _left_k;
        parser->expressions_[_binary_k].right_ = _right_k;
        _left_k = _binary_k;
    }
    *expression = _left_k;
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_expression(zlang_parser_t *parser, size_t *expression) {
    size_t _left_k = ZLANG_NO_EXPRESSION;
    if (!zlang_parse_equality(parser, &_left_k)) {
        return false;
    }
    if (zlang_parser_current(parser)->kind_ == zlang_token_equal_) {
        zlang_parser_advance(parser);
        size_t _right_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_expression(parser, &_right_k)) { // NOLINT(misc-no-recursion)
            return false;
        }
        const size_t _assignment_k = zlang_parser_add_expression(parser);
        if (_assignment_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_assignment_k].kind_ = zlang_expression_assignment_;
        parser->expressions_[_assignment_k].left_ = _left_k;
        parser->expressions_[_assignment_k].right_ = _right_k;
        _left_k = _assignment_k;
    }
    *expression = _left_k;
    return true;
}

static bool zlang_parse_declaration(zlang_parser_t *parser) {
    const zlang_token_t *const _type_k = zlang_parser_current(parser);
    zlang_parser_advance(parser);
    const zlang_token_t *const _name_k = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected a declaration name")) {
        return false;
    }

    const size_t _statement_k = zlang_parser_add_statement(parser);
    if (_statement_k == (size_t)-1) {
        return false;
    }
    zlang_ast_statement_t *const _statement_ptr_k = &parser->statements_[_statement_k];
    _statement_ptr_k->type_name_ = _type_k->text_;
    _statement_ptr_k->name_ = _name_k->text_;
    _statement_ptr_k->line_ = _type_k->line_;
    _statement_ptr_k->column_ = _type_k->column_;

    if (zlang_parser_current(parser)->kind_ == zlang_token_left_bracket_) {
        zlang_parser_advance(parser);
        _statement_ptr_k->kind_ = zlang_statement_sequence_declaration_;
        if (zlang_parser_current(parser)->kind_ == zlang_token_integer_literal_) {
            _statement_ptr_k->capacity_ = (uint32_t)zlang_parser_current(parser)->integer_value_;
            zlang_parser_advance(parser);
        }
        if (!zlang_parser_expect(parser, zlang_token_right_bracket_,
                                 "expected ']' after sequence capacity")) {
            return false;
        }
    } else {
        _statement_ptr_k->kind_ = zlang_statement_declaration_;
        if (zlang_parser_current(parser)->kind_ == zlang_token_equal_) {
            zlang_parser_advance(parser);
            if (!zlang_parse_expression(parser, &_statement_ptr_k->expression_)) {
                return false;
            }
        }
    }
    return zlang_parser_expect(parser, zlang_token_semicolon_, "expected ';' after declaration");
}

static bool zlang_parse_header_expression(zlang_parser_t *parser, size_t statement) {
    if (!zlang_parser_expect(parser, zlang_token_left_paren_, "expected '(' after keyword")) {
        return false;
    }
    if (!zlang_parse_expression(parser, &parser->statements_[statement].expression_)) {
        return false;
    }
    return zlang_parser_expect(parser, zlang_token_right_paren_, "expected ')' after expression");
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_if(zlang_parser_t *parser) {
    zlang_parser_advance(parser);
    const size_t _statement_k = zlang_parser_add_statement(parser);
    if (_statement_k == (size_t)-1) {
        return false;
    }
    parser->statements_[_statement_k].kind_ = zlang_statement_if_;
    parser->statements_[_statement_k].line_ = zlang_parser_current(parser)->line_;
    parser->statements_[_statement_k].column_ = zlang_parser_current(parser)->column_;
    if (!zlang_parse_header_expression(parser, _statement_k)) {
        return false;
    }

    const size_t _body_start_k = parser->statement_count_;
    if (!zlang_parse_block(parser, _statement_k)) { // NOLINT(misc-no-recursion)
        return false;
    }
    const size_t _body_count_k = parser->statement_count_ - _body_start_k;
    parser->statements_[_statement_k].body_start_ = _body_start_k;
    parser->statements_[_statement_k].body_count_ = _body_count_k;

    if (zlang_parser_current(parser)->kind_ == zlang_token_else_) {
        zlang_parser_advance(parser);
        if (!zlang_parser_expect(parser, zlang_token_left_brace_, "expected '{' after else")) {
            return false;
        }
        const size_t _else_start_k = parser->statement_count_;
        if (!zlang_parse_statements(parser) ||
            !zlang_parser_expect(parser, zlang_token_right_brace_,
                                 "expected '}' after else block")) {
            return false;
        }
        parser->statements_[_statement_k].else_start_ = _else_start_k;
        parser->statements_[_statement_k].else_count_ = parser->statement_count_ - _else_start_k;
    }
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_simple_block_statement(zlang_parser_t *parser,
                                               const zlang_statement_kind_t kind) {
    const zlang_token_t *const _keyword_k = zlang_parser_current(parser);
    zlang_parser_advance(parser);
    const size_t _statement_k = zlang_parser_add_statement(parser);
    if (_statement_k == (size_t)-1) {
        return false;
    }
    parser->statements_[_statement_k].kind_ = kind;
    parser->statements_[_statement_k].line_ = _keyword_k->line_;
    parser->statements_[_statement_k].column_ = _keyword_k->column_;
    if (!zlang_parse_header_expression(parser, _statement_k)) {
        return false;
    }
    const size_t _body_start_k = parser->statement_count_;
    if (!zlang_parse_block(parser, _statement_k)) { // NOLINT(misc-no-recursion)
        return false;
    }
    parser->statements_[_statement_k].body_start_ = _body_start_k;
    parser->statements_[_statement_k].body_count_ = parser->statement_count_ - _body_start_k;
    return true;
}

static const char *zlang_parser_sequence_type(const zlang_parser_t *parser, const char *name) {
    for (size_t _index = 0U; _index < parser->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &parser->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_sequence_declaration_ &&
            _statement_k->name_ != NULL && strcmp(_statement_k->name_, name) == 0) {
            return _statement_k->type_name_;
        }
    }
    return "integer";
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_foreach(zlang_parser_t *parser) {
    zlang_parser_advance(parser);
    if (!zlang_parser_expect(parser, zlang_token_left_paren_, "expected '(' after foreach")) {
        return false;
    }
    const zlang_token_t *const _item_k = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected an iteration name")) {
        return false;
    }
    if (!zlang_parser_expect(parser, zlang_token_in_, "expected 'in' in foreach")) {
        return false;
    }
    const zlang_token_t *const _iterable_k = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected a sequence name")) {
        return false;
    }
    if (!zlang_parser_expect(parser, zlang_token_right_paren_, "expected ')' in foreach")) {
        return false;
    }

    const size_t _statement_k = zlang_parser_add_statement(parser);
    if (_statement_k == (size_t)-1) {
        return false;
    }
    parser->statements_[_statement_k].kind_ = zlang_statement_foreach_;
    parser->statements_[_statement_k].type_name_ =
        zlang_parser_sequence_type(parser, _iterable_k->text_);
    parser->statements_[_statement_k].name_ = _item_k->text_;
    parser->statements_[_statement_k].iterable_name_ = _iterable_k->text_;
    parser->statements_[_statement_k].line_ = _item_k->line_;
    parser->statements_[_statement_k].column_ = _item_k->column_;

    const size_t _body_start_k = parser->statement_count_;
    if (!zlang_parse_block(parser, _statement_k)) { // NOLINT(misc-no-recursion)
        return false;
    }
    parser->statements_[_statement_k].body_start_ = _body_start_k;
    parser->statements_[_statement_k].body_count_ = parser->statement_count_ - _body_start_k;
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_function(zlang_parser_t *parser) {
    zlang_parser_advance(parser);
    const zlang_token_t *const _name_k = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected a function name")) {
        return false;
    }
    if (!zlang_parser_expect(parser, zlang_token_left_paren_, "expected '(' after function name")) {
        return false;
    }

    const size_t _parameter_start_k = parser->parameter_count_;
    while (zlang_token_is_type(zlang_parser_current(parser))) {
        if (parser->parameter_count_ >= parser->parameter_capacity_) {
            zlang_parser_unsupported(parser, "parameter capacity exceeded");
            return false;
        }
        const zlang_token_t *const _type_k = zlang_parser_current(parser);
        zlang_parser_advance(parser);
        const zlang_token_t *const _parameter_name_k = zlang_parser_current(parser);
        if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected a parameter name")) {
            return false;
        }
        zlang_parameter_t *const _parameter_k = &parser->parameters_[parser->parameter_count_++];
        _parameter_k->type_name_ = _type_k->text_;
        _parameter_k->name_ = _parameter_name_k->text_;
        _parameter_k->line_ = _type_k->line_;
        _parameter_k->column_ = _type_k->column_;
        if (zlang_parser_current(parser)->kind_ != zlang_token_comma_) {
            break;
        }
        zlang_parser_advance(parser);
    }
    if (!zlang_parser_expect(parser, zlang_token_right_paren_, "expected ')' after parameters")) {
        return false;
    }

    const size_t _statement_k = zlang_parser_add_statement(parser);
    if (_statement_k == (size_t)-1) {
        return false;
    }
    zlang_ast_statement_t *const _statement_ptr_k = &parser->statements_[_statement_k];
    _statement_ptr_k->kind_ = zlang_statement_function_;
    _statement_ptr_k->name_ = _name_k->text_;
    _statement_ptr_k->type_name_ = "void";
    _statement_ptr_k->parameter_start_ = _parameter_start_k;
    _statement_ptr_k->parameter_count_ = parser->parameter_count_ - _parameter_start_k;
    _statement_ptr_k->line_ = _name_k->line_;
    _statement_ptr_k->column_ = _name_k->column_;
    if (zlang_parser_current(parser)->kind_ == zlang_token_colon_) {
        zlang_parser_advance(parser);
        if (!zlang_token_is_type(zlang_parser_current(parser))) {
            zlang_parser_unsupported(parser, "expected a return type");
            return false;
        }
        _statement_ptr_k->type_name_ = zlang_parser_current(parser)->text_;
        zlang_parser_advance(parser);
    }

    const size_t _body_start_k = parser->statement_count_;
    if (!zlang_parse_block(parser, _statement_k)) { // NOLINT(misc-no-recursion)
        return false;
    }
    _statement_ptr_k->body_start_ = _body_start_k;
    _statement_ptr_k->body_count_ = parser->statement_count_ - _body_start_k;
    return true;
}

static bool zlang_parse_return(zlang_parser_t *parser) {
    const zlang_token_t *const _keyword_k = zlang_parser_current(parser);
    zlang_parser_advance(parser);
    const size_t _statement_k = zlang_parser_add_statement(parser);
    if (_statement_k == (size_t)-1) {
        return false;
    }
    parser->statements_[_statement_k].kind_ = zlang_statement_return_;
    parser->statements_[_statement_k].line_ = _keyword_k->line_;
    parser->statements_[_statement_k].column_ = _keyword_k->column_;
    if (zlang_parser_current(parser)->kind_ != zlang_token_semicolon_ &&
        !zlang_parse_expression(parser, &parser->statements_[_statement_k].expression_)) {
        return false;
    }
    return zlang_parser_expect(parser, zlang_token_semicolon_, "expected ';' after return");
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_statements(zlang_parser_t *parser) {
    while (zlang_parser_current(parser)->kind_ != zlang_token_right_brace_ &&
           zlang_parser_current(parser)->kind_ != zlang_token_end_of_file_) {
        const zlang_token_kind_t _kind_k = zlang_parser_current(parser)->kind_;
        if (zlang_token_is_type(zlang_parser_current(parser))) {
            if (!zlang_parse_declaration(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_if_) {
            if (!zlang_parse_if(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_while_) {
            if (!zlang_parse_simple_block_statement(parser, zlang_statement_while_)) {
                return false;
            }
        } else if (_kind_k == zlang_token_repeat_) {
            if (!zlang_parse_simple_block_statement(parser, zlang_statement_repeat_)) {
                return false;
            }
        } else if (_kind_k == zlang_token_foreach_) {
            if (!zlang_parse_foreach(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_function_) {
            if (!zlang_parse_function(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_return_) {
            if (!zlang_parse_return(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_left_brace_) {
            const size_t _block_k = zlang_parser_add_statement(parser);
            if (_block_k == (size_t)-1) {
                return false;
            }
            parser->statements_[_block_k].kind_ = zlang_statement_block_;
            const size_t _body_start_k = parser->statement_count_;
            if (!zlang_parse_block(parser, _block_k)) { // NOLINT(misc-no-recursion)
                return false;
            }
            parser->statements_[_block_k].body_start_ = _body_start_k;
            parser->statements_[_block_k].body_count_ = parser->statement_count_ - _body_start_k;
        } else {
            const size_t _statement_k = zlang_parser_add_statement(parser);
            if (_statement_k == (size_t)-1) {
                return false;
            }
            parser->statements_[_statement_k].kind_ = zlang_statement_expression_;
            parser->statements_[_statement_k].line_ = zlang_parser_current(parser)->line_;
            parser->statements_[_statement_k].column_ = zlang_parser_current(parser)->column_;
            if (!zlang_parse_expression(parser, &parser->statements_[_statement_k].expression_)) {
                return false;
            }
            if (!zlang_parser_expect(parser, zlang_token_semicolon_,
                                     "expected ';' after expression")) {
                return false;
            }
        }
    }
    return true;
}

/* The statement parameter is the block owner; a standalone block owns itself. */
// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_block(zlang_parser_t *parser,
                              size_t statement) { // NOLINT(misc-no-recursion)
    (void)statement;
    if (!zlang_parser_expect(parser, zlang_token_left_brace_, "expected '{'")) {
        return false;
    }
    if (!zlang_parse_statements(parser)) {
        return false;
    }
    return zlang_parser_expect(parser, zlang_token_right_brace_, "expected '}' after block");
}

void zlang_parser_init(zlang_parser_t *parser, zlang_token_t *tokens, size_t token_capacity,
                       zlang_ast_expression_t *expressions, size_t expression_capacity,
                       zlang_ast_statement_t *statements, size_t statement_capacity,
                       zlang_parameter_t *parameters, size_t parameter_capacity,
                       zlang_diagnostics_t *diagnostics) {
    memset(parser, 0, sizeof(*parser));
    parser->tokens_ = tokens;
    parser->token_capacity_ = token_capacity;
    parser->expressions_ = expressions;
    parser->expression_capacity_ = expression_capacity;
    parser->statements_ = statements;
    parser->statement_capacity_ = statement_capacity;
    parser->parameters_ = parameters;
    parser->parameter_capacity_ = parameter_capacity;
    parser->diagnostics_ = diagnostics;
}

bool zlang_parse(zlang_parser_t *parser, const char *source, char *scratch, size_t scratch_capacity,
                 zlang_parse_result_t *result) {
    parser->token_count_ = 0U;
    parser->expression_count_ = 0U;
    parser->statement_count_ = 0U;
    parser->parameter_count_ = 0U;
    parser->position_ = 0U;
    parser->failed_ = false;

    zlang_lexer_init(&parser->lexer_, source, scratch, scratch_capacity);
    if (!zlang_lex(&parser->lexer_, parser->tokens_, parser->token_capacity_, &parser->token_count_,
                   parser->diagnostics_)) {
        return false;
    }

    while (zlang_parser_current(parser)->kind_ != zlang_token_end_of_file_) {
        const zlang_token_kind_t _kind_k = zlang_parser_current(parser)->kind_;
        if (zlang_token_is_type(zlang_parser_current(parser))) {
            if (!zlang_parse_declaration(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_if_) {
            if (!zlang_parse_if(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_while_) {
            if (!zlang_parse_simple_block_statement(parser, zlang_statement_while_)) {
                return false;
            }
        } else if (_kind_k == zlang_token_repeat_) {
            if (!zlang_parse_simple_block_statement(parser, zlang_statement_repeat_)) {
                return false;
            }
        } else if (_kind_k == zlang_token_foreach_) {
            if (!zlang_parse_foreach(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_function_) {
            if (!zlang_parse_function(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_return_) {
            if (!zlang_parse_return(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_left_brace_) {
            const size_t _block_k = zlang_parser_add_statement(parser);
            if (_block_k == (size_t)-1) {
                return false;
            }
            parser->statements_[_block_k].kind_ = zlang_statement_block_;
            const size_t _body_start_k = parser->statement_count_;
            if (!zlang_parse_block(parser, _block_k)) { // NOLINT(misc-no-recursion)
                return false;
            }
            parser->statements_[_block_k].body_start_ = _body_start_k;
            parser->statements_[_block_k].body_count_ = parser->statement_count_ - _body_start_k;
        } else {
            const size_t _statement_k = zlang_parser_add_statement(parser);
            if (_statement_k == (size_t)-1) {
                return false;
            }
            parser->statements_[_statement_k].kind_ = zlang_statement_expression_;
            parser->statements_[_statement_k].line_ = zlang_parser_current(parser)->line_;
            parser->statements_[_statement_k].column_ = zlang_parser_current(parser)->column_;
            if (!zlang_parse_expression(parser, &parser->statements_[_statement_k].expression_)) {
                return false;
            }
            if (!zlang_parser_expect(parser, zlang_token_semicolon_,
                                     "expected ';' after expression")) {
                return false;
            }
        }
    }

    if (parser->failed_) {
        return false;
    }

    result->token_count_ = parser->token_count_;
    result->program_.statements_ = parser->statements_;
    result->program_.statement_count_ = parser->statement_count_;
    result->program_.expressions_ = parser->expressions_;
    result->program_.expression_count_ = parser->expression_count_;
    result->program_.parameters_ = parser->parameters_;
    result->program_.parameter_count_ = parser->parameter_count_;
    return true;
}
