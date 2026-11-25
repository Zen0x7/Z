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
           token->kind_ == zlang_token_int64_ || token->kind_ == zlang_token_uint64_ ||
           token->kind_ == zlang_token_float_ || token->kind_ == zlang_token_double_ ||
           token->kind_ == zlang_token_boolean_ || token->kind_ == zlang_token_string_;
}

static const char *zlang_conversion_target(const char *name) {
    if (name == NULL) {
        return NULL;
    }
    static const char *const targets_k[] = {"to_integer", "to_unsigned", "to_int64",
                                            "to_uint64",  "to_float",    "to_double"};
    static const char *const types_k[] = {"integer", "unsigned", "int64",
                                          "uint64",  "float",    "double"};
    for (size_t _index = 0U; _index < sizeof(targets_k) / sizeof(targets_k[0]); ++_index) {
        if (strcmp(name, targets_k[_index]) == 0) {
            return types_k[_index];
        }
    }
    return NULL;
}

static bool zlang_parser_is_user_type(const zlang_parser_t *parser, const zlang_token_t *token) {
    if (token == NULL || token->kind_ != zlang_token_identifier_ || token->text_ == NULL) {
        return false;
    }
    for (size_t _index = 0U; _index < parser->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &parser->statements_[_index];
        if ((_statement_k->kind_ == zlang_statement_struct_ ||
             _statement_k->kind_ == zlang_statement_enum_) &&
            _statement_k->name_ != NULL && strcmp(_statement_k->name_, token->text_) == 0) {
            return true;
        }
    }
    return false;
}

static const zlang_ast_statement_t *zlang_parser_find_struct(const zlang_parser_t *parser,
                                                             const char *name) {
    if (parser == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < parser->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &parser->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_struct_ && _statement_k->name_ != NULL &&
            strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

static bool zlang_parser_is_declaration_type(const zlang_parser_t *parser,
                                             const zlang_token_t *token) {
    return zlang_token_is_type(token) || token->kind_ == zlang_token_arguments_ ||
           zlang_parser_is_user_type(parser, token);
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
static bool zlang_parse_struct(zlang_parser_t *parser);
static bool zlang_parse_trait(zlang_parser_t *parser);
static bool zlang_parse_enum(zlang_parser_t *parser);
static bool zlang_parse_simple_block_statement(zlang_parser_t *parser, zlang_statement_kind_t kind);
static bool zlang_parse_foreach(zlang_parser_t *parser);
static bool zlang_parse_function(zlang_parser_t *parser, const char *owner, bool is_static,
                                 bool is_constructor);
static bool zlang_parse_import(zlang_parser_t *parser);
static bool zlang_parse_return(zlang_parser_t *parser);
static bool zlang_parse_jump_statement(zlang_parser_t *parser, zlang_statement_kind_t kind);
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
        parser->expressions_[_expression_k].integer64_value_ = _token_k->integer64_value_;
        parser->expressions_[_expression_k].is_unsigned_ = _token_k->is_unsigned_;
        parser->expressions_[_expression_k].is_int64_ = _token_k->is_int64_;
        parser->expressions_[_expression_k].is_uint64_ = _token_k->is_uint64_;
        zlang_parser_advance(parser);
        break;
    case zlang_token_float_literal_:
        _expression_k = zlang_parser_add_expression(parser);
        if (_expression_k == ZLANG_NO_EXPRESSION) {
            return false;
        }
        parser->expressions_[_expression_k].kind_ = zlang_expression_float_literal_;
        parser->expressions_[_expression_k].float_value_ = _token_k->float_value_;
        parser->expressions_[_expression_k].is_float_ = _token_k->is_float_;
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
    case zlang_token_identifier_: {
        if (parser->position_ + 1U < parser->token_count_ &&
            parser->tokens_[parser->position_ + 1U].kind_ == zlang_token_left_paren_ &&
            zlang_conversion_target(_token_k->text_) != NULL) {
            const char *const _target_k = zlang_conversion_target(_token_k->text_);
            zlang_parser_advance(parser);
            size_t _arguments_k[ZLANG_CALL_ARGUMENT_CAPACITY];
            size_t _argument_count_k = 0U;
            if (!zlang_parse_call_arguments(parser, _arguments_k, &_argument_count_k,
                                            ZLANG_CALL_ARGUMENT_CAPACITY)) {
                return false;
            }
            if (_argument_count_k != 1U) {
                zlang_parser_unsupported(parser, "conversion requires one argument");
                return false;
            }
            _expression_k = zlang_parser_add_expression(parser);
            if (_expression_k == ZLANG_NO_EXPRESSION) {
                return false;
            }
            parser->expressions_[_expression_k].kind_ = zlang_expression_conversion_;
            parser->expressions_[_expression_k].name_ = _target_k;
            parser->expressions_[_expression_k].left_ = _arguments_k[0];
            parser->expressions_[_expression_k].line_ = _token_k->line_;
            parser->expressions_[_expression_k].column_ = _token_k->column_;
            break;
        }
        if (parser->position_ + 1U < parser->token_count_ &&
            parser->tokens_[parser->position_ + 1U].kind_ == zlang_token_left_paren_ &&
            zlang_parser_find_struct(parser, _token_k->text_) != NULL) {
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
            parser->expressions_[_expression_k].kind_ = zlang_expression_construct_;
            parser->expressions_[_expression_k].name_ = _token_k->text_;
            for (size_t _argument = 0U; _argument < _argument_count_k; ++_argument) {
                parser->expressions_[_expression_k].argument_indices_[_argument] =
                    _arguments_k[_argument];
            }
            parser->expressions_[_expression_k].argument_count_ = _argument_count_k;
            parser->expressions_[_expression_k].line_ = _token_k->line_;
            parser->expressions_[_expression_k].column_ = _token_k->column_;
            break;
        }
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
                                     "expected a field or method name after '.'")) {
                return false;
            }
            if (zlang_parser_current(parser)->kind_ == zlang_token_left_paren_) {
                size_t _arguments_k[ZLANG_CALL_ARGUMENT_CAPACITY];
                size_t _argument_count_k = 0U;
                if (!zlang_parse_call_arguments(parser, _arguments_k, &_argument_count_k,
                                                ZLANG_CALL_ARGUMENT_CAPACITY)) {
                    return false;
                }
                const char *const _base_name_k = parser->expressions_[_expression_k].name_ != NULL
                                                     ? parser->expressions_[_expression_k].name_
                                                     : NULL;
                const size_t _method_k = zlang_parser_add_expression(parser);
                if (_method_k == ZLANG_NO_EXPRESSION) {
                    return false;
                }
                parser->expressions_[_method_k].kind_ = zlang_expression_method_call_;
                parser->expressions_[_method_k].name_ = _field_k->text_;
                parser->expressions_[_method_k].base_name_ = _base_name_k;
                for (size_t _argument = 0U; _argument < _argument_count_k; ++_argument) {
                    parser->expressions_[_method_k].argument_indices_[_argument] =
                        _arguments_k[_argument];
                }
                parser->expressions_[_method_k].argument_count_ = _argument_count_k;
                parser->expressions_[_method_k].line_ = _dot_k->line_;
                parser->expressions_[_method_k].column_ = _dot_k->column_;
                _expression_k = _method_k;
                break;
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
                while (zlang_parser_current(parser)->kind_ == zlang_token_left_bracket_) {
                    zlang_parser_advance(parser);
                    size_t _index_k = ZLANG_NO_EXPRESSION;
                    if (!zlang_parse_expression(parser, &_index_k)) {
                        return false;
                    }
                    if (!zlang_parser_expect(parser, zlang_token_right_bracket_,
                                             "expected ']' after field index")) {
                        return false;
                    }
                    if (_base_ptr_k->argument_count_ >= ZLANG_ARRAY_MAX_RANK) {
                        zlang_parser_unsupported(parser, "array field index exceeds rank limit");
                        return false;
                    }
                    if (_base_ptr_k->argument_count_ == 0U) {
                        _base_ptr_k->right_ = _index_k;
                    }
                    _base_ptr_k->argument_indices_[_base_ptr_k->argument_count_++] = _index_k;
                }
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

static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_parse_array_initializer(zlang_parser_t *parser, zlang_ast_statement_t *statement,
                              uint32_t depth,
                              uint32_t extents[ZLANG_ARRAY_MAX_RANK]) { // NOLINT(misc-no-recursion)
    if (zlang_parser_current(parser)->kind_ != zlang_token_left_brace_) {
        size_t _value_k = ZLANG_NO_EXPRESSION;
        if (!zlang_parse_expression(parser, &_value_k)) {
            return false;
        }
        ++statement->initializer_count_;
        return true;
    }
    if (depth >= ZLANG_ARRAY_MAX_RANK) {
        zlang_parser_unsupported(parser, "array initializer exceeds the rank limit");
        return false;
    }
    zlang_parser_advance(parser);
    size_t _count_k = 0U;
    bool _first_child_k = true;
    while (zlang_parser_current(parser)->kind_ != zlang_token_right_brace_) {
        uint32_t _child_extents_k[ZLANG_ARRAY_MAX_RANK] = {0};
        if (!zlang_parse_array_initializer(parser, statement, depth + 1U, _child_extents_k)) {
            return false;
        }
        if (_first_child_k) {
            _first_child_k = false;
            for (uint32_t _axis = depth + 1U; _axis < ZLANG_ARRAY_MAX_RANK; ++_axis) {
                extents[_axis] = _child_extents_k[_axis];
            }
        } else {
            for (uint32_t _axis = depth + 1U; _axis < ZLANG_ARRAY_MAX_RANK; ++_axis) {
                if (extents[_axis] != _child_extents_k[_axis]) {
                    zlang_parser_unsupported(parser, "array initializer rows have different sizes");
                    return false;
                }
            }
        }
        ++_count_k;
        if (zlang_parser_current(parser)->kind_ != zlang_token_comma_) {
            break;
        }
        zlang_parser_advance(parser);
    }
    if (!zlang_parser_expect(parser, zlang_token_right_brace_,
                             "expected '}' after array initializer")) {
        return false;
    }
    extents[depth] = (uint32_t)_count_k;
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
            _statement_ptr_k->initializer_start_ = parser->expression_count_;
            uint32_t _initializer_extents_k[ZLANG_ARRAY_MAX_RANK] = {0};
            if (!zlang_parse_array_initializer(parser, _statement_ptr_k, 0U,
                                               _initializer_extents_k)) {
                return false;
            }
            for (uint32_t _axis = 0U; _axis < ZLANG_ARRAY_MAX_RANK; ++_axis) {
                _statement_ptr_k->initializer_extents_[_axis] = _initializer_extents_k[_axis];
            }
            _statement_ptr_k->initializer_count_ =
                parser->expression_count_ - _statement_ptr_k->initializer_start_;
            if (!_statement_ptr_k->has_explicit_capacity_) {
                uint32_t _inferred_rank_k = 0U;
                uint64_t _inferred_capacity_k = 1U;
                for (uint32_t _axis = 0U; _axis < ZLANG_ARRAY_MAX_RANK; ++_axis) {
                    if (_initializer_extents_k[_axis] == 0U) {
                        break;
                    }
                    _inferred_rank_k = _axis + 1U;
                    _inferred_capacity_k *= _initializer_extents_k[_axis];
                }
                if (_inferred_rank_k == 0U) {
                    (void)zlang_diagnostics_add(
                        parser->diagnostics_, ZLANG_DIAGNOSTIC_UNBOUNDED_SEQUENCE, _start_k->line_,
                        _start_k->column_, "array initializer cannot be empty");
                    return false;
                }
                _statement_ptr_k->rank_ = _inferred_rank_k;
                for (uint32_t _axis = 0U; _axis < _inferred_rank_k; ++_axis) {
                    _statement_ptr_k->extents_[_axis] = _initializer_extents_k[_axis];
                }
                _statement_ptr_k->capacity_ = (uint32_t)_inferred_capacity_k;
            }
        } else if (!zlang_parse_expression(parser, &_statement_ptr_k->expression_)) {
            return false;
        }
    }
    return zlang_parser_expect(parser, zlang_token_semicolon_, "expected ';' after declaration");
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_struct(zlang_parser_t *parser) {
    const zlang_token_t *const _keyword_k = zlang_parser_current(parser);
    zlang_parser_advance(parser);
    const zlang_token_t *const _name_k = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected a struct name")) {
        return false;
    }
    const char *_base_name_k = NULL;
    if (zlang_parser_current(parser)->kind_ == zlang_token_colon_) {
        zlang_parser_advance(parser);
        const zlang_token_t *const _base_k = zlang_parser_current(parser);
        if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected a base struct name")) {
            return false;
        }
        _base_name_k = _base_k->text_;
    }
    size_t _uses_count_k = 0U;
    const char *_uses_names_k[4] = {NULL};
    if (zlang_parser_current(parser)->kind_ == zlang_token_uses_) {
        zlang_parser_advance(parser);
        for (;;) {
            const zlang_token_t *const _trait_k = zlang_parser_current(parser);
            if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected a trait name")) {
                return false;
            }
            if (_uses_count_k >= 4U) {
                zlang_parser_unsupported(parser, "trait capacity exceeded");
                return false;
            }
            _uses_names_k[_uses_count_k++] = _trait_k->text_;
            if (zlang_parser_current(parser)->kind_ != zlang_token_comma_) {
                break;
            }
            zlang_parser_advance(parser);
        }
    }
    if (!zlang_parser_expect(parser, zlang_token_left_brace_, "expected '{' after struct header")) {
        return false;
    }
    const size_t _statement_k = zlang_parser_add_statement(parser);
    if (_statement_k == (size_t)-1) {
        return false;
    }
    parser->statements_[_statement_k].kind_ = zlang_statement_struct_;
    parser->statements_[_statement_k].name_ = _name_k->text_;
    parser->statements_[_statement_k].base_name_ = _base_name_k;
    for (size_t _trait = 0U; _trait < _uses_count_k; ++_trait) {
        parser->statements_[_statement_k].uses_names_[_trait] = _uses_names_k[_trait];
    }
    parser->statements_[_statement_k].uses_count_ = _uses_count_k;
    parser->statements_[_statement_k].line_ = _keyword_k->line_;
    parser->statements_[_statement_k].column_ = _keyword_k->column_;
    const size_t _member_start_k = parser->statement_count_;
    while (zlang_parser_current(parser)->kind_ != zlang_token_right_brace_ &&
           zlang_parser_current(parser)->kind_ != zlang_token_end_of_file_) {
        const zlang_token_kind_t _kind_k = zlang_parser_current(parser)->kind_;
        bool _member_ok_k = false;
        if (_kind_k == zlang_token_constructor_) {
            _member_ok_k = zlang_parse_function(parser, _name_k->text_, false, true);
        } else if (_kind_k == zlang_token_static_) {
            zlang_parser_advance(parser);
            if (zlang_parser_current(parser)->kind_ != zlang_token_function_) {
                zlang_parser_unsupported(parser, "expected 'function' after 'static'");
                return false;
            }
            _member_ok_k = zlang_parse_function(parser, _name_k->text_, true, false);
        } else if (_kind_k == zlang_token_function_) {
            _member_ok_k = zlang_parse_function(parser, _name_k->text_, false, false);
        } else if (zlang_parser_starts_declaration(parser)) {
            _member_ok_k = zlang_parse_declaration(parser);
        } else {
            zlang_parser_unsupported(parser, "expected a struct field or method");
            return false;
        }
        if (!_member_ok_k) {
            return false;
        }
    }
    parser->statements_[_statement_k].body_start_ = _member_start_k;
    parser->statements_[_statement_k].body_count_ = parser->statement_count_ - _member_start_k;
    for (size_t _member = _member_start_k; _member < parser->statement_count_; ++_member) {
        if (parser->statements_[_member].kind_ == zlang_statement_array_declaration_) {
            parser->statements_[_member].use_canonical_type_ = true;
        }
    }
    return zlang_parser_expect(parser, zlang_token_right_brace_,
                               "expected '}' after struct fields");
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_trait(zlang_parser_t *parser) {
    const zlang_token_t *const _keyword_k = zlang_parser_current(parser);
    zlang_parser_advance(parser);
    const zlang_token_t *const _name_k = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected a trait name")) {
        return false;
    }
    size_t _requires_count_k = 0U;
    const char *_require_names_k[4] = {NULL};
    const char *_require_type_names_k[4] = {NULL};
    if (zlang_parser_current(parser)->kind_ == zlang_token_requires_) {
        zlang_parser_advance(parser);
        for (;;) {
            const zlang_token_t *const _type_k = zlang_parser_current(parser);
            if (!zlang_parser_is_declaration_type(parser, _type_k)) {
                zlang_parser_unsupported(parser, "expected a required field type");
                return false;
            }
            zlang_parser_advance(parser);
            const zlang_token_t *const _field_k = zlang_parser_current(parser);
            if (!zlang_parser_expect(parser, zlang_token_identifier_,
                                     "expected a required field name")) {
                return false;
            }
            if (_requires_count_k >= 4U) {
                zlang_parser_unsupported(parser, "trait requirement capacity exceeded");
                return false;
            }
            _require_type_names_k[_requires_count_k] = _type_k->text_;
            _require_names_k[_requires_count_k] = _field_k->text_;
            ++_requires_count_k;
            if (zlang_parser_current(parser)->kind_ != zlang_token_comma_) {
                break;
            }
            zlang_parser_advance(parser);
        }
    }
    if (!zlang_parser_expect(parser, zlang_token_left_brace_, "expected '{' after trait header")) {
        return false;
    }
    const size_t _statement_k = zlang_parser_add_statement(parser);
    if (_statement_k == (size_t)-1) {
        return false;
    }
    parser->statements_[_statement_k].kind_ = zlang_statement_trait_;
    parser->statements_[_statement_k].name_ = _name_k->text_;
    for (size_t _field = 0U; _field < _requires_count_k; ++_field) {
        parser->statements_[_statement_k].requires_names_[_field] = _require_names_k[_field];
        parser->statements_[_statement_k].requires_type_names_[_field] =
            _require_type_names_k[_field];
    }
    parser->statements_[_statement_k].requires_count_ = _requires_count_k;
    parser->statements_[_statement_k].line_ = _keyword_k->line_;
    parser->statements_[_statement_k].column_ = _keyword_k->column_;
    const size_t _member_start_k = parser->statement_count_;
    while (zlang_parser_current(parser)->kind_ != zlang_token_right_brace_ &&
           zlang_parser_current(parser)->kind_ != zlang_token_end_of_file_) {
        const zlang_token_kind_t _kind_k = zlang_parser_current(parser)->kind_;
        bool _member_ok_k = false;
        if (_kind_k == zlang_token_static_) {
            zlang_parser_advance(parser);
            if (zlang_parser_current(parser)->kind_ != zlang_token_function_) {
                zlang_parser_unsupported(parser, "expected 'function' after 'static'");
                return false;
            }
            _member_ok_k = zlang_parse_function(parser, _name_k->text_, true, false);
        } else if (_kind_k == zlang_token_function_) {
            _member_ok_k = zlang_parse_function(parser, _name_k->text_, false, false);
        } else {
            zlang_parser_unsupported(parser, "expected a trait method");
            return false;
        }
        if (!_member_ok_k) {
            return false;
        }
    }
    parser->statements_[_statement_k].body_start_ = _member_start_k;
    parser->statements_[_statement_k].body_count_ = parser->statement_count_ - _member_start_k;
    return zlang_parser_expect(parser, zlang_token_right_brace_, "expected '}' after trait");
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

static const zlang_ast_statement_t *zlang_parser_find_array(const zlang_parser_t *parser,
                                                            const char *name) {
    for (size_t _index = 0U; _index < parser->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &parser->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_array_declaration_ &&
            _statement_k->name_ != NULL && strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

static const char *zlang_parser_sequence_type(const zlang_parser_t *parser, const char *name) {
    const zlang_ast_statement_t *const _statement_k = zlang_parser_find_array(parser, name);
    return _statement_k != NULL && _statement_k->type_name_ != NULL ? _statement_k->type_name_
                                                                    : "integer";
}

static uint32_t zlang_parser_array_rank(const zlang_parser_t *parser, const char *name) {
    const zlang_ast_statement_t *const _statement_k = zlang_parser_find_array(parser, name);
    return _statement_k != NULL ? _statement_k->rank_ : 0U;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_foreach(zlang_parser_t *parser) {
    zlang_parser_advance(parser);
    if (!zlang_parser_expect(parser, zlang_token_left_paren_, "expected '(' after foreach")) {
        return false;
    }
    const zlang_token_t *_names_k[ZLANG_ARRAY_MAX_RANK] = {0};
    size_t _name_count_k = 0U;
    for (;;) {
        if (_name_count_k >= ZLANG_ARRAY_MAX_RANK) {
            zlang_parser_unsupported(parser, "too many foreach names");
            return false;
        }
        _names_k[_name_count_k++] = zlang_parser_current(parser);
        if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected an iteration name")) {
            return false;
        }
        if (zlang_parser_current(parser)->kind_ != zlang_token_comma_) {
            break;
        }
        zlang_parser_advance(parser);
    }
    if (!zlang_parser_expect(parser, zlang_token_in_, "expected 'in' in foreach")) {
        return false;
    }
    const zlang_token_t *const _iterable_k = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected an array name")) {
        return false;
    }
    if (!zlang_parser_expect(parser, zlang_token_right_paren_, "expected ')' in foreach")) {
        return false;
    }

    const uint32_t _array_rank_k = zlang_parser_array_rank(parser, _iterable_k->text_);
    const bool _coordinates_k = _array_rank_k > 1U && _name_count_k == _array_rank_k;
    if (_array_rank_k > 1U && !_coordinates_k) {
        zlang_parser_unsupported(parser,
                                 "foreach over a multidimensional array needs one name per axis");
        return false;
    }
    if (!_coordinates_k && _name_count_k > 2U) {
        zlang_parser_unsupported(parser, "foreach expects index and item, or one item name");
        return false;
    }

    const size_t _statement_k = zlang_parser_add_statement(parser);
    if (_statement_k == (size_t)-1) {
        return false;
    }
    zlang_ast_statement_t *const _statement_k_k = &parser->statements_[_statement_k];
    _statement_k_k->kind_ = zlang_statement_foreach_;
    _statement_k_k->type_name_ = zlang_parser_sequence_type(parser, _iterable_k->text_);
    _statement_k_k->iterable_name_ = _iterable_k->text_;
    _statement_k_k->line_ = _names_k[0]->line_;
    _statement_k_k->column_ = _names_k[0]->column_;
    if (_coordinates_k) {
        _statement_k_k->foreach_rank_ = (uint32_t)_name_count_k;
        for (size_t _name = 0U; _name < _name_count_k; ++_name) {
            _statement_k_k->foreach_names_[_name] = _names_k[_name]->text_;
        }
    } else {
        _statement_k_k->name_ = _names_k[_name_count_k - 1U]->text_;
        if (_name_count_k == 2U) {
            _statement_k_k->index_name_ = _names_k[0]->text_;
        }
    }

    const size_t _body_start_k = parser->statement_count_;
    if (!zlang_parse_block(parser, _statement_k)) { // NOLINT(misc-no-recursion)
        return false;
    }
    _statement_k_k->body_start_ = _body_start_k;
    _statement_k_k->body_count_ = parser->statement_count_ - _body_start_k;
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_parse_function(zlang_parser_t *parser, const char *owner, bool is_static,
                                 bool is_constructor) {
    zlang_parser_advance(parser);
    const zlang_token_t *_name_k = zlang_parser_current(parser);
    const char *_function_name_k = "constructor";
    if (is_constructor) {
        if (zlang_parser_current(parser)->kind_ != zlang_token_left_paren_) {
            zlang_parser_unsupported(parser, "constructor has no name");
            return false;
        }
    } else {
        if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected a function name")) {
            return false;
        }
        _function_name_k = _name_k->text_;
    }
    const bool _is_main_k = _name_k != NULL && !is_constructor && zlang_token_is(_name_k, "main");
    if (!zlang_parser_expect(parser, zlang_token_left_paren_, "expected '(' after function name")) {
        return false;
    }

    const size_t _parameter_start_k = parser->parameter_count_;
    size_t _main_parameter_k = 0U;
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
        bool _is_reference_k = false;
        if (_is_main_k) {
            if (_qualifier_kind_k == zlang_token_const_ ||
                _qualifier_kind_k == zlang_token_clone_ ||
                _qualifier_kind_k == zlang_token_reference_) {
                zlang_parser_unsupported(parser, "main parameters use implicit binding modes");
                return false;
            }
            _is_reference_k = _main_parameter_k == 1U;
            ++_main_parameter_k;
        } else {
            if (_qualifier_kind_k == zlang_token_const_) {
                _is_const_k = true;
                zlang_parser_advance(parser);
            }
            const zlang_token_kind_t _mode_kind_k = zlang_parser_current(parser)->kind_;
            if (_mode_kind_k != zlang_token_clone_ && _mode_kind_k != zlang_token_reference_) {
                zlang_parser_unsupported(parser,
                                         "expected 'clone' or 'reference' parameter qualifier");
                return false;
            }
            _is_reference_k = _mode_kind_k == zlang_token_reference_;
            zlang_parser_advance(parser);
        }
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
    _statement_ptr_k->name_ = _function_name_k;
    _statement_ptr_k->is_main_ = _is_main_k;
    _statement_ptr_k->is_static_ = is_static;
    _statement_ptr_k->is_constructor_ = is_constructor;
    _statement_ptr_k->owner_name_ = owner;
    _statement_ptr_k->type_name_ = is_constructor ? owner : "void";
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

static bool zlang_parse_import(zlang_parser_t *parser) {
    const zlang_token_t *const _start_k = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_import_, "expected 'import'")) {
        return false;
    }
    if (!zlang_parser_expect(parser, zlang_token_left_brace_, "expected '{' after import")) {
        return false;
    }
    const char *_names_k[ZLANG_CALL_ARGUMENT_CAPACITY];
    size_t _name_count_k = 0U;
    for (;;) {
        const zlang_token_t *const _name_k = zlang_parser_current(parser);
        if (!zlang_parser_expect(parser, zlang_token_identifier_,
                                 "expected imported function name")) {
            return false;
        }
        if (_name_count_k >= ZLANG_CALL_ARGUMENT_CAPACITY) {
            zlang_parser_unsupported(parser, "too many imported names");
            return false;
        }
        _names_k[_name_count_k++] = _name_k->text_;
        if (zlang_parser_current(parser)->kind_ != zlang_token_comma_) {
            break;
        }
        zlang_parser_advance(parser);
    }
    if (!zlang_parser_expect(parser, zlang_token_right_brace_, "expected '}' after imports")) {
        return false;
    }
    if (!zlang_parser_expect(parser, zlang_token_from_, "expected 'from' after imports")) {
        return false;
    }
    const zlang_token_t *const _domain_k = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_string_literal_, "expected a domain string")) {
        return false;
    }
    if (!zlang_parser_expect(parser, zlang_token_semicolon_, "expected ';' after import")) {
        return false;
    }
    for (size_t _index = 0U; _index < _name_count_k; ++_index) {
        const size_t _statement_k = zlang_parser_add_statement(parser);
        if (_statement_k == (size_t)-1) {
            return false;
        }
        zlang_ast_statement_t *const _import_statement_k = &parser->statements_[_statement_k];
        _import_statement_k->kind_ = zlang_statement_import_;
        _import_statement_k->name_ = _names_k[_index];
        _import_statement_k->type_name_ = _domain_k->text_;
        _import_statement_k->line_ = _start_k->line_;
        _import_statement_k->column_ = _start_k->column_;
    }
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
        } else if (_kind_k == zlang_token_struct_) {
            if (!zlang_parse_struct(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_trait_) {
            if (!zlang_parse_trait(parser)) {
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
            if (!zlang_parse_function(parser, NULL, false, false)) {
                return false;
            }
        } else if (_kind_k == zlang_token_import_) {
            if (!zlang_parse_import(parser)) {
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
        if (_array_k->initializer_extents_[0] != 0U) {
            uint32_t _initializer_rank_k = 0U;
            uint32_t _initializer_capacity_k = 1U;
            for (uint32_t _axis = 0U; _axis < ZLANG_ARRAY_MAX_RANK; ++_axis) {
                if (_array_k->initializer_extents_[_axis] == 0U) {
                    break;
                }
                ++_initializer_rank_k;
                _initializer_capacity_k *= _array_k->initializer_extents_[_axis];
            }
            _array_k->rank_ = _initializer_rank_k;
            for (uint32_t _axis = 0U; _axis < _initializer_rank_k; ++_axis) {
                _array_k->extents_[_axis] = _array_k->initializer_extents_[_axis];
            }
            _array_k->capacity_ = _initializer_capacity_k;
            _array_k->has_explicit_capacity_ = true;
            continue;
        }
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

static bool zlang_infer_foreach_shapes(zlang_program_t *program, zlang_diagnostics_t *diagnostics) {
    for (size_t _statement = 0U; _statement < program->statement_count_; ++_statement) {
        zlang_ast_statement_t *const _foreach_k = &program->statements_[_statement];
        if (_foreach_k->kind_ != zlang_statement_foreach_ || _foreach_k->foreach_rank_ != 0U) {
            continue;
        }
        const zlang_ast_statement_t *const _array_k =
            zlang_find_array_statement(program, _foreach_k->iterable_name_);
        if (_array_k == NULL || _array_k->rank_ <= 1U) {
            continue;
        }
        if (_foreach_k->index_name_ != NULL && _foreach_k->name_ != NULL && _array_k->rank_ == 2U) {
            _foreach_k->foreach_rank_ = 2U;
            _foreach_k->foreach_names_[0] = _foreach_k->index_name_;
            _foreach_k->foreach_names_[1] = _foreach_k->name_;
            _foreach_k->index_name_ = NULL;
            _foreach_k->name_ = NULL;
        } else {
            (void)zlang_diagnostics_add(
                diagnostics, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, _foreach_k->line_, _foreach_k->column_,
                "foreach over a multidimensional array needs one name per axis");
            return false;
        }
    }
    return true;
}

static bool zlang_add_inferred_shape(zlang_program_t *program, size_t function_statement,
                                     const zlang_ast_statement_t *array) {
    if (array == NULL || array->type_name_ == NULL || array->rank_ == 0U ||
        program->inferred_shape_count_ >= ZLANG_MAX_INFERRED_SHAPES) {
        return false;
    }
    for (size_t _index = 0U; _index < program->inferred_shape_count_; ++_index) {
        zlang_inferred_shape_t *const _entry_k = &program->inferred_shapes_[_index];
        if (_entry_k->function_statement_ != function_statement ||
            _entry_k->rank_ != array->rank_ ||
            strcmp(_entry_k->type_name_, array->type_name_) != 0) {
            continue;
        }
        bool _same_k = true;
        for (uint32_t _axis = 0U; _axis < array->rank_; ++_axis) {
            if (_entry_k->extents_[_axis] != array->extents_[_axis]) {
                _same_k = false;
                break;
            }
        }
        if (_same_k) {
            return true;
        }
    }
    zlang_inferred_shape_t *const _entry_k =
        &program->inferred_shapes_[program->inferred_shape_count_++];
    _entry_k->function_statement_ = function_statement;
    _entry_k->type_name_ = array->type_name_;
    _entry_k->rank_ = array->rank_;
    _entry_k->capacity_ = array->capacity_;
    for (uint32_t _axis = 0U; _axis < array->rank_; ++_axis) {
        _entry_k->extents_[_axis] = array->extents_[_axis];
    }
    return true;
}

static bool zlang_collect_inferred_shapes(zlang_program_t *program, size_t function_statement,
                                          const zlang_ast_statement_t *function) {
    for (size_t _expression = 0U; _expression < program->expression_count_; ++_expression) {
        const zlang_ast_expression_t *const _call_k = &program->expressions_[_expression];
        if (_call_k->kind_ != zlang_expression_call_ || _call_k->name_ == NULL ||
            strcmp(_call_k->name_, function->name_) != 0) {
            continue;
        }
        const zlang_ast_statement_t *_shape_k = NULL;
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
                const zlang_ast_statement_t *const _candidate_k = &program->statements_[_candidate];
                if (_candidate_k->kind_ == zlang_statement_array_declaration_ &&
                    _candidate_k->expression_ == _expression) {
                    _shape_k = _candidate_k;
                    break;
                }
            }
        }
        if (_shape_k != NULL && !zlang_add_inferred_shape(program, function_statement, _shape_k)) {
            return false;
        }
    }
    return true;
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
        if (_shape_k == NULL || !zlang_collect_inferred_shapes(program, _statement, _function_k)) {
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
            _local_k->is_inferred_ = true;
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
        } else if (_kind_k == zlang_token_struct_) {
            if (!zlang_parse_struct(parser)) {
                return false;
            }
        } else if (_kind_k == zlang_token_trait_) {
            if (!zlang_parse_trait(parser)) {
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
            if (!zlang_parse_function(parser, NULL, false, false)) {
                return false;
            }
        } else if (_kind_k == zlang_token_import_) {
            if (!zlang_parse_import(parser)) {
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

    memset(&result->program_, 0, sizeof(result->program_));
    result->program_.statements_ = parser->statements_;
    result->program_.statement_count_ = parser->statement_count_;
    result->program_.expressions_ = parser->expressions_;
    result->program_.expression_count_ = parser->expression_count_;
    result->program_.parameters_ = parser->parameters_;
    result->program_.parameter_count_ = parser->parameter_count_;
    if (!zlang_infer_array_shapes(&result->program_, parser->diagnostics_) ||
        !zlang_infer_foreach_shapes(&result->program_, parser->diagnostics_) ||
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
