/** @file
 * @brief Recursive-descent parser for the Zlang grammar.
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

static bool zlang_parse_call_arguments(zlang_parser_t *parser, size_t *arguments, size_t *count,
                                       size_t capacity);
static bool zlang_parse_expression(zlang_parser_t *parser, size_t *expression);
static bool zlang_parse_index_tuple(zlang_parser_t *parser, size_t *expression);

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
    _statement_k->initializer_statement_ = ZLANG_NO_STATEMENT;
    _statement_k->update_expression_ = ZLANG_NO_EXPRESSION;
    return parser->statement_count_++;
}

static bool zlang_token_is_type(const zlang_token_t *token) {
    return token->kind_ == zlang_token_integer_ || token->kind_ == zlang_token_unsigned_ ||
           token->kind_ == zlang_token_boolean_ || token->kind_ == zlang_token_string_;
}

static bool zlang_parser_is_user_type(const zlang_parser_t *parser, const zlang_token_t *token) {
    if (token == NULL || token->kind_ != zlang_token_identifier_ || token->text_ == NULL) {
        return false;
    }
    for (size_t _index = 0U; _index < parser->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &parser->statements_[_index];
        if ((_statement_k->kind_ == zlang_statement_record_ ||
             _statement_k->kind_ == zlang_statement_enum_) &&
            _statement_k->name_ != NULL && strcmp(_statement_k->name_, token->text_) == 0) {
            return true;
        }
    }
    return false;
}

static bool zlang_parser_is_declaration_type(const zlang_parser_t *parser,
                                             const zlang_token_t *token) {
    return zlang_token_is_type(token) || zlang_parser_is_user_type(parser, token);
}

static bool zlang_parser_starts_declaration(const zlang_parser_t *parser) {
    const zlang_token_t *const _token_k = zlang_parser_current(parser);
    if (_token_k->kind_ == zlang_token_const_ || _token_k->kind_ == zlang_token_array_ ||
        zlang_token_is_type(_token_k)) {
        return true;
    }
    return _token_k->kind_ == zlang_token_identifier_ &&
           parser->position_ + 1U < parser->token_count_ &&
           parser->tokens_[parser->position_ + 1U].kind_ == zlang_token_identifier_;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_call_arguments(zlang_parser_t *parser, size_t *arguments, size_t *count,
                                       size_t capacity) { // NOLINT(misc-no-recursion)
    if (!zlang_parser_expect(parser, zlang_token_left_paren_, "expected '(' after call")) {
        return false;
    }
    *count = 0U;
    if (zlang_parser_current(parser)->kind_ == zlang_token_right_paren_) {
        zlang_parser_advance(parser);
        return true;
    }
    for (;;) {
        size_t _argument_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_expression(parser, &_argument_k)) { // NOLINT(misc-no-recursion)
            return false;
        }
        if (*count >= capacity) {
            zlang_parser_unsupported(parser, "call argument capacity exceeded");
            return false;
        }
        arguments[*count] = _argument_k;
        ++*count;
        if (zlang_parser_current(parser)->kind_ != zlang_token_comma_) {
            break;
        }
        zlang_parser_advance(parser);
    }
    return zlang_parser_expect(parser, zlang_token_right_paren_,
                               "expected ')' after call arguments");
}

static void zlang_parser_remember_string(zlang_parser_t *parser, const char *name) {
    if (name != NULL &&
        parser->string_count_ < sizeof(parser->string_names_) / sizeof(parser->string_names_[0])) {
        parser->string_names_[parser->string_count_++] = name;
    }
}

static bool zlang_parser_is_string_name(const zlang_parser_t *parser, const char *name) {
    if (name == NULL) {
        return false;
    }
    for (size_t _index = 0U; _index < parser->string_count_; ++_index) {
        if (strcmp(parser->string_names_[_index], name) == 0) {
            return true;
        }
    }
    return false;
}

static bool zlang_parse_if(zlang_parser_t *parser);
static bool zlang_parse_for(zlang_parser_t *parser);
static bool zlang_parse_record(zlang_parser_t *parser);
static bool zlang_parse_enum(zlang_parser_t *parser);
static bool zlang_parse_simple_block_statement(zlang_parser_t *parser, zlang_statement_kind_t kind);
static bool zlang_parse_foreach(zlang_parser_t *parser);
static bool zlang_parse_function(zlang_parser_t *parser);
static bool zlang_parse_return(zlang_parser_t *parser);
static bool zlang_parse_jump_statement(zlang_parser_t *parser, zlang_statement_kind_t kind);
static bool zlang_parse_block(zlang_parser_t *parser, size_t statement);
static bool zlang_parse_statements(zlang_parser_t *parser);

/* Recursive descent mirrors Annex A directly. NOLINT keeps the policy check
   local: nesting is bounded by the caller-owned AST capacity. */
static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_parse_index_tuple(zlang_parser_t *parser, size_t *expression) { // NOLINT(misc-no-recursion)
    const zlang_token_t *const _start_k = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_left_brace_,
                             "expected '{' before array index tuple")) {
        return false;
    }
    const size_t _tuple_k = zlang_parser_add_expression(parser);
    if (_tuple_k == ZLANG_NO_EXPRESSION) {
        return false;
    }
    parser->expressions_[_tuple_k].kind_ = zlang_expression_index_tuple_;
    parser->expressions_[_tuple_k].line_ = _start_k->line_;
    parser->expressions_[_tuple_k].column_ = _start_k->column_;
    while (zlang_parser_current(parser)->kind_ != zlang_token_right_brace_) {
        size_t _index_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_expression(parser, &_index_k)) {
            return false;
        }
        zlang_ast_expression_t *const _tuple_ptr_k = &parser->expressions_[_tuple_k];
        if (_tuple_ptr_k->argument_count_ >= ZLANG_ARRAY_MAX_RANK) {
            zlang_parser_unsupported(parser, "array index tuple exceeds the rank limit");
            return false;
        }
        _tuple_ptr_k->argument_indices_[_tuple_ptr_k->argument_count_++] = _index_k;
        if (zlang_parser_current(parser)->kind_ != zlang_token_comma_) {
            break;
        }
        zlang_parser_advance(parser);
    }
    if (!zlang_parser_expect(parser, zlang_token_right_brace_,
                             "expected '}' after array index tuple")) {
        return false;
    }
    *expression = _tuple_k;
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_primary(zlang_parser_t *parser,
                                size_t *expression) { // NOLINT(misc-no-recursion)
    const zlang_token_t *const _token_k = zlang_parser_current(parser);
    size_t _expression_k = ZLANG_NO_EXPRESSION;

    switch (_token_k->kind_) {
    case zlang_token_array_get_:
    case zlang_token_array_set_: {
        const bool _is_set_k = _token_k->kind_ == zlang_token_array_set_;
        zlang_parser_advance(parser);
        if (!zlang_parser_expect(parser, zlang_token_left_paren_,
                                 _is_set_k ? "expected '(' after array_set"
                                           : "expected '(' after array_get")) {
            return false;
        }
        const zlang_token_t *const _target_k = zlang_parser_current(parser);
        if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected an array name")) {
            return false;
        }
        if (!zlang_parser_expect(parser, zlang_token_comma_, "expected ',' after array name")) {
            return false;
        }
        size_t _tuple_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_index_tuple(parser, &_tuple_k)) {
            return false;
        }
        size_t _value_k = ZLANG_NO_EXPRESSION;
        if (_is_set_k) {
            if (!zlang_parser_expect(parser, zlang_token_comma_,
                                     "expected ',' after array index tuple")) {
                return false;
            }
            if (!zlang_parse_expression(parser, &_value_k)) {
                return false;
            }
        }
        if (!zlang_parser_expect(parser, zlang_token_right_paren_,
                                 "expected ')' after array access")) {
            return false;
        }
        _expression_k = zlang_parser_add_expression(parser);
        if (_expression_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_expression_k].kind_ =
            _is_set_k ? zlang_expression_array_set_ : zlang_expression_array_get_;
        parser->expressions_[_expression_k].name_ = _target_k->text_;
        parser->expressions_[_expression_k].left_ = _tuple_k;
        parser->expressions_[_expression_k].right_ = _value_k;
        parser->expressions_[_expression_k].line_ = _token_k->line_;
        parser->expressions_[_expression_k].column_ = _token_k->column_;
        break;
    }
    case zlang_token_left_paren_: {
        zlang_parser_advance(parser);
        size_t _inner_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_expression(parser, &_inner_k)) { // NOLINT(misc-no-recursion)
            return false;
        }
        if (!zlang_parser_expect(parser, zlang_token_right_paren_,
                                 "expected ')' after grouped expression")) {
            return false;
        }
        *expression = _inner_k;
        return true;
    }
    case zlang_token_not_: {
        const uint32_t _line_k = _token_k->line_;
        const uint32_t _column_k = _token_k->column_;
        zlang_parser_advance(parser);
        size_t _operand_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_primary(parser, &_operand_k)) { // NOLINT(misc-no-recursion)
            return false;
        }
        _expression_k = zlang_parser_add_expression(parser);
        if (_expression_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_expression_k].kind_ = zlang_expression_not_;
        parser->expressions_[_expression_k].left_ = _operand_k;
        parser->expressions_[_expression_k].line_ = _line_k;
        parser->expressions_[_expression_k].column_ = _column_k;
        break;
    }
    case zlang_token_minus_: {
        const uint32_t _line_k = _token_k->line_;
        const uint32_t _column_k = _token_k->column_;
        zlang_parser_advance(parser);
        size_t _operand_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_primary(parser, &_operand_k)) { // NOLINT(misc-no-recursion)
            return false;
        }
        _expression_k = zlang_parser_add_expression(parser);
        if (_expression_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_expression_k].kind_ = zlang_expression_negative_;
        parser->expressions_[_expression_k].left_ = _operand_k;
        parser->expressions_[_expression_k].line_ = _line_k;
        parser->expressions_[_expression_k].column_ = _column_k;
        break;
    }
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
        parser->expressions_[_expression_k].is_string_ = true;
        zlang_parser_advance(parser);
        break;
    case zlang_token_length_: {
        zlang_parser_advance(parser);
        if (!zlang_parser_expect(parser, zlang_token_left_paren_, "expected '(' after length")) {
            return false;
        }
        const zlang_token_t *const _name_k = zlang_parser_current(parser);
        if (!zlang_parser_expect(parser, zlang_token_identifier_,
                                 "expected a sequence name in length")) {
            return false;
        }
        if (!zlang_parser_expect(parser, zlang_token_right_paren_,
                                 "expected ')' after length argument")) {
            return false;
        }
        _expression_k = zlang_parser_add_expression(parser);
        if (_expression_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_expression_k].kind_ = zlang_expression_length_;
        parser->expressions_[_expression_k].name_ = _name_k->text_;
        parser->expressions_[_expression_k].line_ = _token_k->line_;
        parser->expressions_[_expression_k].column_ = _token_k->column_;
        break;
    }
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
        if (parser->position_ + 1U < parser->token_count_ &&
            parser->tokens_[parser->position_ + 1U].kind_ == zlang_token_left_paren_) {
            zlang_parser_advance(parser);
            size_t _arguments_k[ZLANG_CALL_ARGUMENT_CAPACITY];
            size_t _argument_count_k = 0U;
            if (!zlang_parse_call_arguments(parser, _arguments_k, &_argument_count_k,
                                            ZLANG_CALL_ARGUMENT_CAPACITY)) {
                return false;
            }
            _expression_k = zlang_parser_add_expression(parser);
            if (_expression_k == ZLANG_NO_EXPRESSION) {
                return false;
            }
            parser->expressions_[_expression_k].kind_ = zlang_expression_call_;
            parser->expressions_[_expression_k].name_ = _token_k->text_;
            for (size_t _argument = 0U; _argument < _argument_count_k; ++_argument) {
                parser->expressions_[_expression_k].argument_indices_[_argument] =
                    _arguments_k[_argument];
            }
            parser->expressions_[_expression_k].argument_count_ = _argument_count_k;
            parser->expressions_[_expression_k].is_string_ =
                zlang_parser_is_string_name(parser, _token_k->text_);
            break;
        }

        zlang_parser_advance(parser);
        _expression_k = zlang_parser_add_expression(parser);
        if (_expression_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_expression_k].kind_ = zlang_expression_identifier_;
        parser->expressions_[_expression_k].name_ = _token_k->text_;
        parser->expressions_[_expression_k].is_string_ =
            zlang_parser_is_string_name(parser, _token_k->text_);
        parser->expressions_[_expression_k].line_ = _token_k->line_;
        parser->expressions_[_expression_k].column_ = _token_k->column_;

        while (zlang_parser_current(parser)->kind_ == zlang_token_left_bracket_) {
            zlang_parser_advance(parser);
            size_t _index_k = ZLANG_NO_EXPRESSION;
            if (!zlang_parse_expression(parser, &_index_k)) {
                return false;
            }
            if (!zlang_parser_expect(parser, zlang_token_right_bracket_,
                                     "expected ']' after index expression")) {
                return false;
            }
            zlang_ast_expression_t *const _index_ptr_k = &parser->expressions_[_expression_k];
            if (_index_ptr_k->argument_count_ >= ZLANG_CALL_ARGUMENT_CAPACITY) {
                zlang_parser_unsupported(parser, "array rank exceeds the supported limit");
                return false;
            }
            if (_index_ptr_k->argument_count_ == 0U) {
                _index_ptr_k->right_ = _index_k;
            }
            _index_ptr_k->kind_ = zlang_expression_index_;
            _index_ptr_k->argument_indices_[_index_ptr_k->argument_count_++] = _index_k;
        }
        while (zlang_parser_current(parser)->kind_ == zlang_token_dot_) {
            const zlang_token_t *const _dot_k = zlang_parser_current(parser);
            zlang_parser_advance(parser);
            const zlang_token_t *const _field_k = zlang_parser_current(parser);
            if (!zlang_parser_expect(parser, zlang_token_identifier_,
                                     "expected a field name after '.'")) {
                return false;
            }
            zlang_ast_expression_t *const _base_ptr_k = &parser->expressions_[_expression_k];
            const zlang_expression_kind_t _base_kind_k = _base_ptr_k->kind_;
            const char *const _base_name_k = _base_ptr_k->name_;
            if (_base_kind_k == zlang_expression_identifier_ ||
                _base_kind_k == zlang_expression_index_) {
                _base_ptr_k->kind_ = zlang_expression_field_;
                _base_ptr_k->base_name_ = _base_name_k;
                _base_ptr_k->name_ = _field_k->text_;
                if (_base_kind_k == zlang_expression_identifier_) {
                    _base_ptr_k->right_ = ZLANG_NO_EXPRESSION;
                }
                _base_ptr_k->line_ = _dot_k->line_;
                _base_ptr_k->column_ = _dot_k->column_;
                continue;
            }
            const size_t _base_k = _expression_k;
            _expression_k = zlang_parser_add_expression(parser);
            if (_expression_k == ZLANG_NO_EXPRESSION) {
                return false;
            }
            parser->expressions_[_expression_k].kind_ = zlang_expression_field_;
            parser->expressions_[_expression_k].left_ = _base_k;
            parser->expressions_[_expression_k].name_ = _field_k->text_;
            parser->expressions_[_expression_k].line_ = _dot_k->line_;
            parser->expressions_[_expression_k].column_ = _dot_k->column_;
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
static bool zlang_parse_multiplicative(zlang_parser_t *parser, size_t *expression) {
    size_t _left_k = ZLANG_NO_EXPRESSION;
    if (!zlang_parse_primary(parser, &_left_k)) {
        return false;
    }
    for (;;) {
        const zlang_token_kind_t _kind_k = zlang_parser_current(parser)->kind_;
        zlang_expression_kind_t _operator_k = zlang_expression_mul_;
        if (_kind_k == zlang_token_star_) {
            _operator_k = zlang_expression_mul_;
        } else if (_kind_k == zlang_token_slash_) {
            _operator_k = zlang_expression_div_;
        } else if (_kind_k == zlang_token_percent_) {
            _operator_k = zlang_expression_mod_;
        } else {
            break;
        }
        zlang_parser_advance(parser);
        size_t _right_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_primary(parser, &_right_k)) {
            return false;
        }
        const size_t _binary_k = zlang_parser_add_expression(parser);
        if (_binary_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_binary_k].kind_ = _operator_k;
        parser->expressions_[_binary_k].left_ = _left_k;
        parser->expressions_[_binary_k].right_ = _right_k;
        _left_k = _binary_k;
    }
    *expression = _left_k;
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_additive(zlang_parser_t *parser, size_t *expression) {
    size_t _left_k = ZLANG_NO_EXPRESSION;
    if (!zlang_parse_multiplicative(parser, &_left_k)) {
        return false;
    }
    while (zlang_parser_current(parser)->kind_ == zlang_token_plus_ ||
           zlang_parser_current(parser)->kind_ == zlang_token_minus_) {
        const bool _is_plus_k = zlang_parser_current(parser)->kind_ == zlang_token_plus_;
        zlang_parser_advance(parser);
        size_t _right_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_multiplicative(parser, &_right_k)) {
            return false;
        }
        const size_t _binary_k = zlang_parser_add_expression(parser);
        if (_binary_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        const bool _is_string_k = _is_plus_k && (parser->expressions_[_left_k].is_string_ ||
                                                 parser->expressions_[_right_k].is_string_);
        zlang_expression_kind_t _operator_k = zlang_expression_sub_;
        if (_is_string_k) {
            _operator_k = zlang_expression_string_concat_;
        } else if (_is_plus_k) {
            _operator_k = zlang_expression_add_;
        }
        parser->expressions_[_binary_k].kind_ = _operator_k;
        parser->expressions_[_binary_k].is_string_ = _is_string_k;
        parser->expressions_[_binary_k].left_ = _left_k;
        parser->expressions_[_binary_k].right_ = _right_k;
        _left_k = _binary_k;
    }
    *expression = _left_k;
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_relational(zlang_parser_t *parser, size_t *expression) {
    size_t _left_k = ZLANG_NO_EXPRESSION;
    if (!zlang_parse_additive(parser, &_left_k)) {
        return false;
    }
    for (;;) {
        const zlang_token_kind_t _kind_k = zlang_parser_current(parser)->kind_;
        zlang_expression_kind_t _expression_kind_k = zlang_expression_less_;
        if (_kind_k == zlang_token_less_) {
            _expression_kind_k = zlang_expression_less_;
        } else if (_kind_k == zlang_token_greater_) {
            _expression_kind_k = zlang_expression_greater_;
        } else if (_kind_k == zlang_token_less_equal_) {
            _expression_kind_k = zlang_expression_less_equal_;
        } else if (_kind_k == zlang_token_greater_equal_) {
            _expression_kind_k = zlang_expression_greater_equal_;
        } else {
            break;
        }
        zlang_parser_advance(parser);
        size_t _right_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_additive(parser, &_right_k)) {
            return false;
        }
        const size_t _binary_k = zlang_parser_add_expression(parser);
        if (_binary_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_binary_k].kind_ = _expression_kind_k;
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
    if (!zlang_parse_relational(parser, &_left_k)) {
        return false;
    }
    while (zlang_parser_current(parser)->kind_ == zlang_token_equal_equal_ ||
           zlang_parser_current(parser)->kind_ == zlang_token_not_equal_) {
        const bool _not_equal_k = zlang_parser_current(parser)->kind_ == zlang_token_not_equal_;
        zlang_parser_advance(parser);
        size_t _right_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_additive(parser, &_right_k)) {
            return false;
        }
        const size_t _binary_k = zlang_parser_add_expression(parser);
        if (_binary_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_binary_k].kind_ =
            _not_equal_k ? zlang_expression_not_equal_ : zlang_expression_equal_;
        parser->expressions_[_binary_k].left_ = _left_k;
        parser->expressions_[_binary_k].right_ = _right_k;
        _left_k = _binary_k;
    }
    *expression = _left_k;
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_logical_and(zlang_parser_t *parser, size_t *expression) {
    size_t _left_k = ZLANG_NO_EXPRESSION;
    if (!zlang_parse_equality(parser, &_left_k)) {
        return false;
    }
    while (zlang_parser_current(parser)->kind_ == zlang_token_and_) {
        zlang_parser_advance(parser);
        size_t _right_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_equality(parser, &_right_k)) {
            return false;
        }
        const size_t _binary_k = zlang_parser_add_expression(parser);
        if (_binary_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_binary_k].kind_ = zlang_expression_and_;
        parser->expressions_[_binary_k].left_ = _left_k;
        parser->expressions_[_binary_k].right_ = _right_k;
        _left_k = _binary_k;
    }
    *expression = _left_k;
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_logical_or(zlang_parser_t *parser, size_t *expression) {
    size_t _left_k = ZLANG_NO_EXPRESSION;
    if (!zlang_parse_logical_and(parser, &_left_k)) {
        return false;
    }
    while (zlang_parser_current(parser)->kind_ == zlang_token_or_) {
        zlang_parser_advance(parser);
        size_t _right_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_logical_and(parser, &_right_k)) {
            return false;
        }
        const size_t _binary_k = zlang_parser_add_expression(parser);
        if (_binary_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_binary_k].kind_ = zlang_expression_or_;
        parser->expressions_[_binary_k].left_ = _left_k;
        parser->expressions_[_binary_k].right_ = _right_k;
        _left_k = _binary_k;
    }
    *expression = _left_k;
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static zlang_expression_kind_t zlang_compound_operator(zlang_token_kind_t kind) {
    switch (kind) {
    case zlang_token_plus_equal_:
        return zlang_expression_add_;
    case zlang_token_minus_equal_:
        return zlang_expression_sub_;
    case zlang_token_star_equal_:
        return zlang_expression_mul_;
    case zlang_token_slash_equal_:
        return zlang_expression_div_;
    case zlang_token_percent_equal_:
        return zlang_expression_mod_;
    default:
        return zlang_expression_add_;
    }
}

static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_parse_expression(zlang_parser_t *parser,
                       size_t *expression) { // NOLINT(misc-no-recursion)
    size_t _left_k = ZLANG_NO_EXPRESSION;
    if (!zlang_parse_logical_or(parser, &_left_k)) {
        return false;
    }
    const zlang_token_kind_t _assignment_kind_k = zlang_parser_current(parser)->kind_;
    const bool _is_assignment_k = _assignment_kind_k == zlang_token_equal_ ||
                                  _assignment_kind_k == zlang_token_plus_equal_ ||
                                  _assignment_kind_k == zlang_token_minus_equal_ ||
                                  _assignment_kind_k == zlang_token_star_equal_ ||
                                  _assignment_kind_k == zlang_token_slash_equal_ ||
                                  _assignment_kind_k == zlang_token_percent_equal_;
    if (_is_assignment_k) {
        zlang_parser_advance(parser);
        size_t _right_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_expression(parser, &_right_k)) { // NOLINT(misc-no-recursion)
            return false;
        }
        const size_t _assignment_k = zlang_parser_add_expression(parser);
        if (_assignment_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        const zlang_expression_kind_t _left_kind_k = parser->expressions_[_left_k].kind_;
        if (_assignment_kind_k != zlang_token_equal_ &&
            (_left_kind_k == zlang_expression_index_ || _left_kind_k == zlang_expression_field_)) {
            zlang_parser_unsupported(parser, "compound assignment requires a scalar target");
            return false;
        }
        zlang_expression_kind_t _assignment_expression_k = zlang_expression_compound_assignment_;
        if (_assignment_kind_k == zlang_token_equal_) {
            if (_left_kind_k == zlang_expression_index_) {
                _assignment_expression_k = zlang_expression_index_assignment_;
            } else if (_left_kind_k == zlang_expression_field_) {
                _assignment_expression_k = zlang_expression_field_assignment_;
            } else {
                _assignment_expression_k = zlang_expression_assignment_;
            }
        }
        parser->expressions_[_assignment_k].kind_ = _assignment_expression_k;
        parser->expressions_[_assignment_k].operator_ = zlang_compound_operator(_assignment_kind_k);
        parser->expressions_[_assignment_k].left_ = _left_k;
        parser->expressions_[_assignment_k].right_ = _right_k;
        _left_k = _assignment_k;
    }
    *expression = _left_k;
    return true;
}

static bool zlang_parse_declaration(zlang_parser_t *parser) {
    const zlang_token_t *const _start_k = zlang_parser_current(parser);
    bool _is_const_k = false;
    if (zlang_parser_current(parser)->kind_ == zlang_token_const_) {
        _is_const_k = true;
        zlang_parser_advance(parser);
    }
    bool _is_array_k = false;
    uint32_t _extents_k[ZLANG_ARRAY_MAX_RANK];
    uint32_t _rank_k = 0U;
    bool _has_explicit_extents_k = true;
    if (zlang_parser_current(parser)->kind_ == zlang_token_array_) {
        zlang_parser_advance(parser);
        _is_array_k = true;
        while (zlang_parser_current(parser)->kind_ == zlang_token_left_bracket_) {
            if (_rank_k >= ZLANG_ARRAY_MAX_RANK) {
                zlang_parser_unsupported(parser, "array rank exceeds the supported limit");
                return false;
            }
            zlang_parser_advance(parser);
            if (zlang_parser_current(parser)->kind_ != zlang_token_integer_literal_) {
                if (_rank_k != 0U) {
                    zlang_parser_unsupported(parser, "only the first array extent may be inferred");
                    return false;
                }
                _has_explicit_extents_k = false;
                _extents_k[_rank_k++] = 0U;
            } else {
                const uint64_t _extent_k = (uint64_t)zlang_parser_current(parser)->integer_value_;
                if (_extent_k == 0U || _extent_k > UINT32_MAX) {
                    zlang_parser_unsupported(parser, "array extent must fit in uint32_t");
                    return false;
                }
                _extents_k[_rank_k++] = (uint32_t)_extent_k;
                zlang_parser_advance(parser);
            }
            if (!zlang_parser_expect(parser, zlang_token_right_bracket_,
                                     "expected ']' after array extent")) {
                return false;
            }
        }
    }

    const zlang_token_t *const _type_k = zlang_parser_current(parser);
    bool _has_element_type_k = zlang_parser_is_declaration_type(parser, _type_k);
    if (_has_element_type_k) {
        zlang_parser_advance(parser);
    } else if (!_is_array_k) {
        zlang_parser_unsupported(parser, "expected a declaration type");
        return false;
    }

    const bool _is_string_k = _has_element_type_k && zlang_token_is(_type_k, "string");
    bool _has_element_capacity_k = false;
    uint32_t _element_capacity_k = 0U;
    if (zlang_parser_current(parser)->kind_ == zlang_token_left_bracket_) {
        zlang_parser_advance(parser);
        if (zlang_parser_current(parser)->kind_ != zlang_token_integer_literal_) {
            zlang_parser_unsupported(parser, "type capacity must be an integer literal");
            return false;
        }
        _element_capacity_k = (uint32_t)zlang_parser_current(parser)->integer_value_;
        _has_element_capacity_k = true;
        zlang_parser_advance(parser);
        if (!zlang_parser_expect(parser, zlang_token_right_bracket_,
                                 "expected ']' after type capacity")) {
            return false;
        }
        if (!_is_string_k) {
            zlang_parser_unsupported(parser, "type capacity is only valid for string");
            return false;
        }
    }

    const zlang_token_t *const _name_k = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected a declaration name")) {
        return false;
    }
    if (zlang_parser_current(parser)->kind_ == zlang_token_left_bracket_ && !_is_string_k) {
        zlang_parser_unsupported(parser, "array syntax uses a leading array[N] modifier");
        return false;
    }

    const size_t _statement_k = zlang_parser_add_statement(parser);
    if (_statement_k == (size_t)-1) {
        return false;
    }
    zlang_ast_statement_t *const _statement_ptr_k = &parser->statements_[_statement_k];
    _statement_ptr_k->type_name_ = _has_element_type_k ? _type_k->text_ : NULL;
    _statement_ptr_k->name_ = _name_k->text_;
    _statement_ptr_k->line_ = _start_k->line_;
    _statement_ptr_k->column_ = _start_k->column_;
    _statement_ptr_k->kind_ =
        _is_array_k ? zlang_statement_array_declaration_ : zlang_statement_declaration_;
    _statement_ptr_k->is_const_ = _is_const_k;
    _statement_ptr_k->rank_ = _rank_k;
    for (uint32_t _index = 0U; _index < _rank_k; ++_index) {
        _statement_ptr_k->extents_[_index] = _extents_k[_index];
    }
    uint64_t _capacity_k = 1U;
    for (uint32_t _index = 0U; _index < _rank_k; ++_index) {
        _capacity_k *= (uint64_t)_extents_k[_index];
        if (_capacity_k > UINT32_MAX) {
            zlang_parser_unsupported(parser, "array total size exceeds uint32_t");
            return false;
        }
    }
    _statement_ptr_k->capacity_ = _is_array_k ? (uint32_t)_capacity_k : _element_capacity_k;
    _statement_ptr_k->has_explicit_capacity_ =
        _is_array_k ? _has_explicit_extents_k : _has_element_capacity_k;
    _statement_ptr_k->is_inferred_ = _is_array_k && !_has_explicit_extents_k;
    _statement_ptr_k->use_canonical_type_ = _statement_ptr_k->is_inferred_;
    _statement_ptr_k->element_capacity_ = _element_capacity_k;
    _statement_ptr_k->has_element_capacity_ = _has_element_capacity_k;

    if (_is_string_k && !_is_array_k) {
        zlang_parser_remember_string(parser, _name_k->text_);
    }

    if (zlang_parser_current(parser)->kind_ == zlang_token_equal_) {
        zlang_parser_advance(parser);
        if (_is_array_k && zlang_parser_current(parser)->kind_ == zlang_token_left_brace_) {
            zlang_parser_advance(parser);
            _statement_ptr_k->initializer_start_ = parser->expression_count_;
            if (zlang_parser_current(parser)->kind_ != zlang_token_right_brace_) {
                for (;;) {
                    size_t _initializer_k = ZLANG_NO_EXPRESSION;
                    if (!zlang_parse_expression(parser, &_initializer_k)) {
                        return false;
                    }
                    ++_statement_ptr_k->initializer_count_;
                    if (zlang_parser_current(parser)->kind_ != zlang_token_comma_) {
                        break;
                    }
                    zlang_parser_advance(parser);
                }
            }
            if (!zlang_parser_expect(parser, zlang_token_right_brace_,
                                     "expected '}' after sequence initializer")) {
                return false;
            }
            if (!_statement_ptr_k->has_explicit_capacity_) {
                _statement_ptr_k->extents_[0] = (uint32_t)_statement_ptr_k->initializer_count_;
                _statement_ptr_k->capacity_ = (uint32_t)_statement_ptr_k->initializer_count_;
            }
        } else if (!zlang_parse_expression(parser, &_statement_ptr_k->expression_)) {
            return false;
        }
    }
    return zlang_parser_expect(parser, zlang_token_semicolon_, "expected ';' after declaration");
}

static bool zlang_parse_record(zlang_parser_t *parser) {
    const zlang_token_t *const _keyword_k = zlang_parser_current(parser);
    zlang_parser_advance(parser);
    const zlang_token_t *const _name_k = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected a record name")) {
        return false;
    }
    if (!zlang_parser_expect(parser, zlang_token_left_brace_, "expected '{' after record name")) {
        return false;
    }
    const size_t _statement_k = zlang_parser_add_statement(parser);
    if (_statement_k == (size_t)-1) {
        return false;
    }
    parser->statements_[_statement_k].kind_ = zlang_statement_record_;
    parser->statements_[_statement_k].name_ = _name_k->text_;
    parser->statements_[_statement_k].line_ = _keyword_k->line_;
    parser->statements_[_statement_k].column_ = _keyword_k->column_;
    const size_t _field_start_k = parser->statement_count_;
    while (zlang_parser_current(parser)->kind_ != zlang_token_right_brace_ &&
           zlang_parser_current(parser)->kind_ != zlang_token_end_of_file_) {
        if (!zlang_parser_starts_declaration(parser) || !zlang_parse_declaration(parser)) {
            return false;
        }
    }
    parser->statements_[_statement_k].body_start_ = _field_start_k;
    parser->statements_[_statement_k].body_count_ = parser->statement_count_ - _field_start_k;
    return zlang_parser_expect(parser, zlang_token_right_brace_,
                               "expected '}' after record fields");
}

static bool zlang_parse_enum(zlang_parser_t *parser) {
    const zlang_token_t *const _keyword_k = zlang_parser_current(parser);
    zlang_parser_advance(parser);
    const zlang_token_t *const _name_k = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected an enum name")) {
        return false;
    }
    if (!zlang_parser_expect(parser, zlang_token_left_brace_, "expected '{' after enum name")) {
        return false;
    }
    const size_t _statement_k = zlang_parser_add_statement(parser);
    if (_statement_k == (size_t)-1) {
        return false;
    }
    parser->statements_[_statement_k].kind_ = zlang_statement_enum_;
    parser->statements_[_statement_k].name_ = _name_k->text_;
    parser->statements_[_statement_k].line_ = _keyword_k->line_;
    parser->statements_[_statement_k].column_ = _keyword_k->column_;
    const size_t _item_start_k = parser->expression_count_;
    uint32_t _value_k = 0U;
    while (zlang_parser_current(parser)->kind_ == zlang_token_identifier_) {
        const zlang_token_t *const _item_k = zlang_parser_current(parser);
        zlang_parser_advance(parser);
        const size_t _expression_k = zlang_parser_add_expression(parser);
        if (_expression_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_expression_k].kind_ = zlang_expression_enum_item_;
        parser->expressions_[_expression_k].name_ = _item_k->text_;
        parser->expressions_[_expression_k].integer_value_ = (int32_t)_value_k++;
        parser->expressions_[_expression_k].line_ = _item_k->line_;
        parser->expressions_[_expression_k].column_ = _item_k->column_;
        if (zlang_parser_current(parser)->kind_ != zlang_token_comma_) {
            break;
        }
        zlang_parser_advance(parser);
    }
    parser->statements_[_statement_k].initializer_start_ = _item_start_k;
    parser->statements_[_statement_k].initializer_count_ =
        parser->expression_count_ - _item_start_k;
    return zlang_parser_expect(parser, zlang_token_right_brace_, "expected '}' after enum values");
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
        const size_t _else_start_k = parser->statement_count_;
        size_t _else_count_k = 0U;
        if (zlang_parser_current(parser)->kind_ == zlang_token_if_) {
            if (!zlang_parse_if(parser)) { // NOLINT(misc-no-recursion)
                return false;
            }
            _else_count_k = 1U;
        } else {
            if (!zlang_parser_expect(parser, zlang_token_left_brace_,
                                     "expected '{' or 'if' after else")) {
                return false;
            }
            if (!zlang_parse_statements(parser) ||
                !zlang_parser_expect(parser, zlang_token_right_brace_,
                                     "expected '}' after else block")) {
                return false;
            }
            _else_count_k = parser->statement_count_ - _else_start_k;
        }
        parser->statements_[_statement_k].else_start_ = _else_start_k;
        parser->statements_[_statement_k].else_count_ = _else_count_k;
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
        if (_statement_k->kind_ == zlang_statement_array_declaration_ &&
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
    const zlang_token_t *const _first_k = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected an iteration name")) {
        return false;
    }
    const zlang_token_t *const _index_k =
        zlang_parser_current(parser)->kind_ == zlang_token_comma_ ? _first_k : NULL;
    const zlang_token_t *_item_k = _first_k;
    if (_index_k != NULL) {
        zlang_parser_advance(parser);
        _item_k = zlang_parser_current(parser);
        if (!zlang_parser_expect(parser, zlang_token_identifier_,
                                 "expected an item name after ','")) {
            return false;
        }
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
    if (_first_k == NULL || _item_k == NULL || _iterable_k == NULL) {
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
    parser->statements_[_statement_k].index_name_ = _index_k != NULL ? _index_k->text_ : NULL;
    parser->statements_[_statement_k].iterable_name_ = _iterable_k->text_;
    parser->statements_[_statement_k].line_ = _first_k->line_;
    parser->statements_[_statement_k].column_ = _first_k->column_;

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
    for (;;) {
        const zlang_token_kind_t _qualifier_kind_k = zlang_parser_current(parser)->kind_;
        if (_qualifier_kind_k != zlang_token_const_ && _qualifier_kind_k != zlang_token_clone_ &&
            _qualifier_kind_k != zlang_token_reference_ &&
            _qualifier_kind_k != zlang_token_array_ &&
            !zlang_parser_is_declaration_type(parser, zlang_parser_current(parser))) {
            break;
        }
        if (parser->parameter_count_ >= parser->parameter_capacity_) {
            zlang_parser_unsupported(parser, "parameter capacity exceeded");
            return false;
        }
        const zlang_token_t *const _start_k = zlang_parser_current(parser);
        bool _is_const_k = false;
        if (_qualifier_kind_k == zlang_token_const_) {
            _is_const_k = true;
            zlang_parser_advance(parser);
        }
        const zlang_token_kind_t _mode_kind_k = zlang_parser_current(parser)->kind_;
        if (_mode_kind_k != zlang_token_clone_ && _mode_kind_k != zlang_token_reference_) {
            zlang_parser_unsupported(parser, "expected 'clone' or 'reference' parameter qualifier");
            return false;
        }
        const bool _is_reference_k = _mode_kind_k == zlang_token_reference_;
        zlang_parser_advance(parser);
        const zlang_token_t *const _type_k = zlang_parser_current(parser);
        const bool _is_array_type_k = _type_k->kind_ == zlang_token_array_;
        if (!_is_array_type_k && !zlang_parser_is_declaration_type(parser, _type_k)) {
            zlang_parser_unsupported(parser, "expected a declaration type");
            return false;
        }
        zlang_parser_advance(parser);
        const zlang_token_t *const _parameter_name_k = zlang_parser_current(parser);
        if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected a parameter name")) {
            return false;
        }
        zlang_parameter_t *const _parameter_k = &parser->parameters_[parser->parameter_count_++];
        _parameter_k->type_name_ = _is_array_type_k ? NULL : _type_k->text_;
        _parameter_k->name_ = _parameter_name_k->text_;
        _parameter_k->is_array_ = _is_array_type_k;
        _parameter_k->line_ = _start_k->line_;
        _parameter_k->column_ = _start_k->column_;
        _parameter_k->is_const_ = _is_const_k;
        _parameter_k->is_reference_ = _is_reference_k;
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
        const zlang_token_t *const _return_k = zlang_parser_current(parser);
        const bool _is_array_return_k = _return_k->kind_ == zlang_token_array_;
        if (!_is_array_return_k && !zlang_parser_is_declaration_type(parser, _return_k)) {
            zlang_parser_unsupported(parser, "expected a return type");
            return false;
        }
        _statement_ptr_k->type_name_ = _is_array_return_k ? NULL : _return_k->text_;
        _statement_ptr_k->is_array_return_ = _is_array_return_k;
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

static bool zlang_parse_jump_statement(zlang_parser_t *parser, zlang_statement_kind_t kind) {
    const zlang_token_t *const _keyword_k = zlang_parser_current(parser);
    zlang_parser_advance(parser);
    const size_t _statement_k = zlang_parser_add_statement(parser);
    if (_statement_k == (size_t)-1) {
        return false;
    }
    parser->statements_[_statement_k].kind_ = kind;
    parser->statements_[_statement_k].line_ = _keyword_k->line_;
    parser->statements_[_statement_k].column_ = _keyword_k->column_;
    return zlang_parser_expect(parser, zlang_token_semicolon_,
                               kind == zlang_statement_break_ ? "expected ';' after break"
                                                              : "expected ';' after continue");
}

static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_parse_for(zlang_parser_t *parser) { // NOLINT(misc-no-recursion)
    const zlang_token_t *const _keyword_k = zlang_parser_current(parser);
    zlang_parser_advance(parser);
    if (!zlang_parser_expect(parser, zlang_token_left_paren_, "expected '(' after for")) {
        return false;
    }

    size_t _initializer_k = ZLANG_NO_STATEMENT;
    if (zlang_parser_current(parser)->kind_ == zlang_token_semicolon_) {
        zlang_parser_advance(parser);
    } else if (zlang_token_is_type(zlang_parser_current(parser)) ||
               zlang_parser_current(parser)->kind_ == zlang_token_array_ ||
               zlang_parser_current(parser)->kind_ == zlang_token_const_) {
        _initializer_k = parser->statement_count_;
        if (!zlang_parse_declaration(parser)) {
            return false;
        }
    } else {
        _initializer_k = parser->statement_count_;
        const size_t _statement_k = zlang_parser_add_statement(parser);
        if (_statement_k == (size_t)-1) {
            return false;
        }
        parser->statements_[_statement_k].kind_ = zlang_statement_expression_;
        parser->statements_[_statement_k].line_ = zlang_parser_current(parser)->line_;
        parser->statements_[_statement_k].column_ = zlang_parser_current(parser)->column_;
        if (!zlang_parse_expression(parser, &parser->statements_[_statement_k].expression_) ||
            !zlang_parser_expect(parser, zlang_token_semicolon_,
                                 "expected ';' after for initializer")) {
            return false;
        }
    }

    size_t _condition_k = ZLANG_NO_EXPRESSION;
    if (!zlang_parse_expression(parser, &_condition_k) ||
        !zlang_parser_expect(parser, zlang_token_semicolon_, "expected ';' after for condition")) {
        return false;
    }
    size_t _update_k = ZLANG_NO_EXPRESSION;
    if (!zlang_parse_expression(parser, &_update_k) ||
        !zlang_parser_expect(parser, zlang_token_right_paren_, "expected ')' after for update")) {
        return false;
    }

    const size_t _statement_k = zlang_parser_add_statement(parser);
    if (_statement_k == (size_t)-1) {
        return false;
    }
    zlang_ast_statement_t *const _statement_ptr_k = &parser->statements_[_statement_k];
    _statement_ptr_k->kind_ = zlang_statement_for_;
    _statement_ptr_k->expression_ = _condition_k;
    _statement_ptr_k->update_expression_ = _update_k;
    _statement_ptr_k->initializer_statement_ = _initializer_k;
    _statement_ptr_k->line_ = _keyword_k->line_;
    _statement_ptr_k->column_ = _keyword_k->column_;

    const size_t _body_start_k = parser->statement_count_;
    if (!zlang_parse_block(parser, _statement_k)) { // NOLINT(misc-no-recursion)
        return false;
    }
    _statement_ptr_k->body_start_ = _body_start_k;
    _statement_ptr_k->body_count_ = parser->statement_count_ - _body_start_k;
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_statements(zlang_parser_t *parser) {
    while (zlang_parser_current(parser)->kind_ != zlang_token_right_brace_ &&
           zlang_parser_current(parser)->kind_ != zlang_token_end_of_file_) {
        const zlang_token_kind_t _kind_k = zlang_parser_current(parser)->kind_;
        if (zlang_parser_starts_declaration(parser)) {
            if (!zlang_parse_declaration(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_record_) {
            if (!zlang_parse_record(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_enum_) {
            if (!zlang_parse_enum(parser)) {
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
        } else if (_kind_k == zlang_token_for_) {
            if (!zlang_parse_for(parser)) {
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
        } else if (_kind_k == zlang_token_break_) {
            if (!zlang_parse_jump_statement(parser, zlang_statement_break_)) {
                return false;
            }
        } else if (_kind_k == zlang_token_continue_) {
            if (!zlang_parse_jump_statement(parser, zlang_statement_continue_)) {
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
                       zlang_comment_t *comments, size_t comment_capacity,
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
    parser->lexer_.comments_ = comments;
    parser->lexer_.comment_capacity_ = comment_capacity;
    parser->diagnostics_ = diagnostics;
}

static const zlang_ast_statement_t *zlang_find_array_statement(const zlang_program_t *program,
                                                               const char *name) {
    if (name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_array_declaration_ &&
            _statement_k->name_ != NULL && strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_expression_constant_int(const zlang_program_t *program, size_t index,
                              int32_t *value) { // NOLINT(misc-no-recursion)
    if (index == ZLANG_NO_EXPRESSION || index >= program->expression_count_) {
        return false;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ == zlang_expression_integer_literal_ && !_expression_k->is_unsigned_) {
        *value = _expression_k->integer_value_;
        return true;
    }
    if (_expression_k->kind_ == zlang_expression_identifier_ && _expression_k->name_ != NULL) {
        for (size_t _statement = 0U; _statement < program->statement_count_; ++_statement) {
            const zlang_ast_statement_t *const _statement_k = &program->statements_[_statement];
            if (_statement_k->kind_ == zlang_statement_declaration_ && _statement_k->is_const_ &&
                _statement_k->name_ != NULL &&
                strcmp(_statement_k->name_, _expression_k->name_) == 0) {
                return zlang_expression_constant_int(program, _statement_k->expression_, value);
            }
        }
    }
    return false;
}

static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_expression_contains(const zlang_program_t *program, size_t root, size_t index) {
    if (root == ZLANG_NO_EXPRESSION || root >= program->expression_count_) {
        return false;
    }
    if (root == index) {
        return true;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[root];
    if (zlang_expression_contains(program, _expression_k->left_, index) ||
        zlang_expression_contains(program, _expression_k->right_, index)) {
        return true;
    }
    for (size_t _argument = 0U;
         _argument < _expression_k->argument_count_ && _argument < ZLANG_CALL_ARGUMENT_CAPACITY;
         ++_argument) {
        if (zlang_expression_contains(program, _expression_k->argument_indices_[_argument],
                                      index)) {
            return true;
        }
    }
    return false;
}

static size_t zlang_find_owner_statement(const zlang_program_t *program, size_t index) {
    for (size_t _statement = 0U; _statement < program->statement_count_; ++_statement) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_statement];
        if (zlang_expression_contains(program, _statement_k->expression_, index) ||
            zlang_expression_contains(program, _statement_k->update_expression_, index)) {
            return _statement;
        }
        const size_t _end_k = _statement_k->initializer_start_ + _statement_k->initializer_count_;
        for (size_t _item = _statement_k->initializer_start_;
             _item < _end_k && _item < program->expression_count_; ++_item) {
            if (_item == index) {
                return _statement;
            }
        }
    }
    return ZLANG_NO_STATEMENT;
}

static bool zlang_loop_variable_max(const zlang_program_t *program, size_t owner, const char *name,
                                    int32_t *max) {
    if (name == NULL) {
        return false;
    }
    for (size_t _statement = 0U; _statement < program->statement_count_; ++_statement) {
        const zlang_ast_statement_t *const _loop_k = &program->statements_[_statement];
        if (_loop_k->kind_ != zlang_statement_for_) {
            continue;
        }
        const size_t _end_k = _loop_k->body_start_ + _loop_k->body_count_;
        if (owner < _loop_k->body_start_ || owner >= _end_k) {
            continue;
        }
        if (_loop_k->initializer_statement_ == ZLANG_NO_STATEMENT ||
            _loop_k->initializer_statement_ >= program->statement_count_) {
            continue;
        }
        const zlang_ast_statement_t *const _init_k =
            &program->statements_[_loop_k->initializer_statement_];
        if (_init_k->name_ == NULL || strcmp(_init_k->name_, name) != 0) {
            continue;
        }
        int32_t _start_k = 0;
        if (!zlang_expression_constant_int(program, _init_k->expression_, &_start_k) ||
            _start_k < 0) {
            continue;
        }
        if (_loop_k->expression_ >= program->expression_count_) {
            continue;
        }
        const zlang_ast_expression_t *const _condition_k =
            &program->expressions_[_loop_k->expression_];
        if (_condition_k->kind_ != zlang_expression_less_ &&
            _condition_k->kind_ != zlang_expression_less_equal_) {
            continue;
        }
        if (_condition_k->left_ >= program->expression_count_ ||
            program->expressions_[_condition_k->left_].kind_ != zlang_expression_identifier_ ||
            program->expressions_[_condition_k->left_].name_ == NULL ||
            strcmp(program->expressions_[_condition_k->left_].name_, name) != 0) {
            continue;
        }
        int32_t _bound_k = 0;
        if (!zlang_expression_constant_int(program, _condition_k->right_, &_bound_k)) {
            continue;
        }
        *max = _bound_k - (_condition_k->kind_ == zlang_expression_less_ ? 1 : 0);
        return *max >= 0;
    }
    return false;
}

static bool zlang_infer_array_shapes(zlang_program_t *program, zlang_diagnostics_t *diagnostics) {
    for (size_t _statement = 0U; _statement < program->statement_count_; ++_statement) {
        zlang_ast_statement_t *const _array_k = &program->statements_[_statement];
        if (_array_k->kind_ != zlang_statement_array_declaration_ || _array_k->type_name_ == NULL ||
            (_array_k->rank_ != 0U && _array_k->extents_[0] != 0U)) {
            continue;
        }
        uint32_t _rank_k = _array_k->rank_;
        uint32_t _extents_k[ZLANG_ARRAY_MAX_RANK] = {0};
        if (_array_k->initializer_count_ > 0U && _rank_k == 0U) {
            _rank_k = 1U;
            _extents_k[0] = (uint32_t)_array_k->initializer_count_;
        }
        for (size_t _expression = 0U; _expression < program->expression_count_; ++_expression) {
            const zlang_ast_expression_t *const _expression_k = &program->expressions_[_expression];
            if (_expression_k->kind_ != zlang_expression_index_ || _expression_k->name_ == NULL ||
                _expression_k->argument_count_ == 0U ||
                strcmp(_expression_k->name_, _array_k->name_) != 0) {
                continue;
            }
            if (_expression_k->argument_count_ > _rank_k) {
                _rank_k = (uint32_t)_expression_k->argument_count_;
            }
            const size_t _owner_k = zlang_find_owner_statement(program, _expression);
            for (size_t _axis = 0U;
                 _axis < _expression_k->argument_count_ && _axis < ZLANG_ARRAY_MAX_RANK; ++_axis) {
                const size_t _index_k = _expression_k->argument_indices_[_axis];
                int32_t _max_k = -1;
                if (zlang_expression_constant_int(program, _index_k, &_max_k) && _max_k >= 0) {
                    _extents_k[_axis] = _extents_k[_axis] > (uint32_t)_max_k + 1U
                                            ? _extents_k[_axis]
                                            : (uint32_t)_max_k + 1U;
                    continue;
                }
                if (_index_k < program->expression_count_ &&
                    program->expressions_[_index_k].kind_ == zlang_expression_identifier_ &&
                    zlang_loop_variable_max(program, _owner_k,
                                            program->expressions_[_index_k].name_, &_max_k)) {
                    _extents_k[_axis] = _extents_k[_axis] > (uint32_t)_max_k + 1U
                                            ? _extents_k[_axis]
                                            : (uint32_t)_max_k + 1U;
                }
            }
        }
        if (_rank_k == 0U) {
            (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_UNBOUNDED_SEQUENCE,
                                        _array_k->line_, _array_k->column_,
                                        "cannot infer array rank");
            return false;
        }
        uint32_t _capacity_k = 1U;
        for (uint32_t _axis = 0U; _axis < _rank_k; ++_axis) {
            if (_extents_k[_axis] == 0U) {
                (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_UNBOUNDED_SEQUENCE,
                                            _array_k->line_, _array_k->column_,
                                            "cannot infer array extent");
                return false;
            }
            _capacity_k *= _extents_k[_axis];
        }
        _array_k->rank_ = _rank_k;
        for (uint32_t _axis = 0U; _axis < _rank_k; ++_axis) {
            _array_k->extents_[_axis] = _extents_k[_axis];
        }
        _array_k->capacity_ = _capacity_k;
        _array_k->has_explicit_capacity_ = true;
    }
    return true;
}

static bool zlang_parameter_has_array(const zlang_program_t *program,
                                      const zlang_ast_statement_t *function) {
    const size_t _end_k = function->parameter_start_ + function->parameter_count_;
    for (size_t _index = function->parameter_start_;
         _index < _end_k && _index < program->parameter_count_; ++_index) {
        if (program->parameters_[_index].is_array_) {
            return true;
        }
    }
    return false;
}

static bool zlang_infer_function_array_shapes(zlang_program_t *program,
                                              zlang_diagnostics_t *diagnostics) {
    for (size_t _statement = 0U; _statement < program->statement_count_; ++_statement) {
        zlang_ast_statement_t *const _function_k = &program->statements_[_statement];
        if (_function_k->kind_ != zlang_statement_function_ ||
            (!_function_k->is_array_return_ && !zlang_parameter_has_array(program, _function_k))) {
            continue;
        }
        const zlang_ast_statement_t *_shape_k = NULL;
        for (size_t _expression = 0U; _expression < program->expression_count_; ++_expression) {
            const zlang_ast_expression_t *const _call_k = &program->expressions_[_expression];
            if (_call_k->kind_ != zlang_expression_call_ || _call_k->name_ == NULL ||
                strcmp(_call_k->name_, _function_k->name_) != 0) {
                continue;
            }
            for (size_t _argument = 0U;
                 _argument < _call_k->argument_count_ && _argument < ZLANG_CALL_ARGUMENT_CAPACITY;
                 ++_argument) {
                const size_t _argument_k = _call_k->argument_indices_[_argument];
                if (_argument_k >= program->expression_count_ ||
                    program->expressions_[_argument_k].kind_ != zlang_expression_identifier_) {
                    continue;
                }
                _shape_k =
                    zlang_find_array_statement(program, program->expressions_[_argument_k].name_);
                if (_shape_k != NULL) {
                    break;
                }
            }
            if (_shape_k == NULL) {
                for (size_t _candidate = 0U; _candidate < program->statement_count_; ++_candidate) {
                    const zlang_ast_statement_t *const _candidate_k =
                        &program->statements_[_candidate];
                    if (_candidate_k->kind_ == zlang_statement_array_declaration_ &&
                        _candidate_k->expression_ == _expression) {
                        _shape_k = _candidate_k;
                        break;
                    }
                }
            }
            if (_shape_k != NULL) {
                break;
            }
        }
        if (_shape_k == NULL) {
            (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_UNBOUNDED_SEQUENCE,
                                        _function_k->line_, _function_k->column_,
                                        "cannot infer function array shape");
            return false;
        }
        for (size_t _candidate = 0U; _candidate < program->statement_count_; ++_candidate) {
            zlang_ast_statement_t *const _candidate_k = &program->statements_[_candidate];
            if (_candidate_k->kind_ != zlang_statement_array_declaration_ ||
                _candidate_k->type_name_ == NULL ||
                strcmp(_candidate_k->type_name_, _shape_k->type_name_) != 0 ||
                _candidate_k->rank_ != _shape_k->rank_) {
                continue;
            }
            bool _same_k = true;
            for (uint32_t _axis = 0U; _axis < _shape_k->rank_; ++_axis) {
                if (_candidate_k->extents_[_axis] != _shape_k->extents_[_axis]) {
                    _same_k = false;
                    break;
                }
            }
            if (_same_k) {
                _candidate_k->use_canonical_type_ = true;
            }
        }
        if (_function_k->is_array_return_) {
            _function_k->type_name_ = _shape_k->type_name_;
            _function_k->return_rank_ = _shape_k->rank_;
            for (uint32_t _axis = 0U; _axis < _shape_k->rank_; ++_axis) {
                _function_k->return_extents_[_axis] = _shape_k->extents_[_axis];
            }
        }
        for (size_t _index = _function_k->parameter_start_;
             _index < _function_k->parameter_start_ + _function_k->parameter_count_ &&
             _index < program->parameter_count_;
             ++_index) {
            zlang_parameter_t *const _parameter_k = &program->parameters_[_index];
            if (!_parameter_k->is_array_) {
                continue;
            }
            _parameter_k->type_name_ = _shape_k->type_name_;
            _parameter_k->rank_ = _shape_k->rank_;
            for (uint32_t _axis = 0U; _axis < _shape_k->rank_; ++_axis) {
                _parameter_k->extents_[_axis] = _shape_k->extents_[_axis];
            }
        }
        const size_t _body_end_k = _function_k->body_start_ + _function_k->body_count_;
        for (size_t _body = _function_k->body_start_;
             _body < _body_end_k && _body < program->statement_count_; ++_body) {
            zlang_ast_statement_t *const _local_k = &program->statements_[_body];
            if (_local_k->kind_ != zlang_statement_array_declaration_ ||
                _local_k->type_name_ != NULL) {
                continue;
            }
            _local_k->type_name_ = _shape_k->type_name_;
            _local_k->rank_ = _shape_k->rank_;
            for (uint32_t _axis = 0U; _axis < _shape_k->rank_; ++_axis) {
                _local_k->extents_[_axis] = _shape_k->extents_[_axis];
            }
            _local_k->capacity_ = _shape_k->capacity_;
            _local_k->has_explicit_capacity_ = true;
            _local_k->use_canonical_type_ = true;
        }
    }
    return true;
}

bool zlang_parse(zlang_parser_t *parser, const char *source, char *scratch, size_t scratch_capacity,
                 zlang_parse_result_t *result) {
    parser->token_count_ = 0U;
    parser->expression_count_ = 0U;
    parser->statement_count_ = 0U;
    parser->parameter_count_ = 0U;
    parser->string_count_ = 0U;
    parser->position_ = 0U;
    parser->failed_ = false;

    zlang_lexer_init(&parser->lexer_, source, scratch, scratch_capacity, parser->lexer_.comments_,
                     parser->lexer_.comment_capacity_);
    if (!zlang_lex(&parser->lexer_, parser->tokens_, parser->token_capacity_, &parser->token_count_,
                   parser->diagnostics_)) {
        return false;
    }

    while (zlang_parser_current(parser)->kind_ != zlang_token_end_of_file_) {
        const zlang_token_kind_t _kind_k = zlang_parser_current(parser)->kind_;
        if (zlang_parser_starts_declaration(parser)) {
            if (!zlang_parse_declaration(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_record_) {
            if (!zlang_parse_record(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_enum_) {
            if (!zlang_parse_enum(parser)) {
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
        } else if (_kind_k == zlang_token_for_) {
            if (!zlang_parse_for(parser)) {
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
        } else if (_kind_k == zlang_token_break_) {
            if (!zlang_parse_jump_statement(parser, zlang_statement_break_)) {
                return false;
            }
        } else if (_kind_k == zlang_token_continue_) {
            if (!zlang_parse_jump_statement(parser, zlang_statement_continue_)) {
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

    result->program_.statements_ = parser->statements_;
    result->program_.statement_count_ = parser->statement_count_;
    result->program_.expressions_ = parser->expressions_;
    result->program_.expression_count_ = parser->expression_count_;
    result->program_.parameters_ = parser->parameters_;
    result->program_.parameter_count_ = parser->parameter_count_;
    if (!zlang_infer_array_shapes(&result->program_, parser->diagnostics_) ||
        !zlang_infer_function_array_shapes(&result->program_, parser->diagnostics_)) {
        return false;
    }

    result->token_count_ = parser->token_count_;
    result->program_.statements_ = parser->statements_;
    result->program_.statement_count_ = parser->statement_count_;
    result->program_.expressions_ = parser->expressions_;
    result->program_.expression_count_ = parser->expression_count_;
    result->program_.parameters_ = parser->parameters_;
    result->program_.parameter_count_ = parser->parameter_count_;
    result->program_.comments_ = parser->lexer_.comments_;
    result->program_.comment_count_ = parser->lexer_.comment_count_;
    return true;
}
