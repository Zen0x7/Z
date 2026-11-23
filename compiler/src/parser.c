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

static zlang_ast_expression_t *zlang_parser_add_expression(zlang_parser_t *parser) {
    if (parser->expression_count_ >= parser->expression_capacity_) {
        zlang_parser_unsupported(parser, "expression capacity exceeded");
        return NULL;
    }
    zlang_ast_expression_t *_expression = &parser->expressions_[parser->expression_count_++];
    memset(_expression, 0, sizeof(*_expression));
    return _expression;
}

static zlang_ast_statement_t *zlang_parser_add_statement(zlang_parser_t *parser) {
    if (parser->statement_count_ >= parser->statement_capacity_) {
        zlang_parser_unsupported(parser, "statement capacity exceeded");
        return NULL;
    }
    zlang_ast_statement_t *_statement = &parser->statements_[parser->statement_count_++];
    memset(_statement, 0, sizeof(*_statement));
    return _statement;
}

static const zlang_ast_expression_t *zlang_parse_expression(zlang_parser_t *parser) {
    const zlang_token_t *_token = zlang_parser_current(parser);
    zlang_ast_expression_t *_expression = NULL;

    switch (_token->kind_) {
    case zlang_token_integer_literal_:
        _expression = zlang_parser_add_expression(parser);
        if (_expression == NULL) {
            return NULL;
        }
        _expression->kind_ = zlang_ast_expression_integer_literal_;
        _expression->integer_value_ = _token->integer_value_;
        _expression->is_unsigned_ = _token->is_unsigned_;
        zlang_parser_advance(parser);
        return _expression;
    case zlang_token_string_literal_:
        _expression = zlang_parser_add_expression(parser);
        if (_expression == NULL) {
            return NULL;
        }
        _expression->kind_ = zlang_ast_expression_string_literal_;
        _expression->string_.data_ = _token->text_;
        _expression->string_.length_ = _token->value_length_;
        zlang_parser_advance(parser);
        return _expression;
    case zlang_token_identifier_:
        _expression = zlang_parser_add_expression(parser);
        if (_expression == NULL) {
            return NULL;
        }
        _expression->kind_ = zlang_ast_expression_identifier_;
        _expression->name_ = _token->text_;
        zlang_parser_advance(parser);
        return _expression;
    default:
        zlang_parser_unsupported(parser, "unsupported expression");
        return NULL;
    }
}

static const zlang_ast_statement_t *zlang_parse_declaration(zlang_parser_t *parser) {
    const zlang_token_t *_type = zlang_parser_current(parser);
    const zlang_token_t *_name = NULL;
    zlang_ast_statement_t *_statement = NULL;
    zlang_parser_advance(parser);

    _name = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected a declaration name")) {
        return NULL;
    }
    _statement = zlang_parser_add_statement(parser);
    if (_statement == NULL) {
        return NULL;
    }
    _statement->type_name_ = _type->text_;
    _statement->name_ = _name->text_;
    _statement->line_ = _type->line_;
    _statement->column_ = _type->column_;

    if (zlang_parser_current(parser)->kind_ == zlang_token_left_bracket_) {
        zlang_parser_advance(parser);
        _statement->kind_ = zlang_ast_statement_sequence_declaration_;
        if (zlang_parser_current(parser)->kind_ == zlang_token_integer_literal_) {
            _statement->capacity_ = (uint32_t)zlang_parser_current(parser)->integer_value_;
            zlang_parser_advance(parser);
        }
        if (!zlang_parser_expect(parser, zlang_token_right_bracket_,
                                 "expected ']' after sequence capacity")) {
            return NULL;
        }
    } else {
        _statement->kind_ = zlang_ast_statement_declaration_;
        if (zlang_parser_current(parser)->kind_ == zlang_token_equal_) {
            zlang_parser_advance(parser);
            _statement->initializer_ = zlang_parse_expression(parser);
            if (_statement->initializer_ == NULL) {
                return NULL;
            }
        }
    }

    if (!zlang_parser_expect(parser, zlang_token_semicolon_, "expected ';' after declaration")) {
        return NULL;
    }
    return _statement;
}

static const zlang_ast_statement_t *zlang_parse_print(zlang_parser_t *parser) {
    const zlang_token_t *_keyword = zlang_parser_current(parser);
    zlang_ast_statement_t *_statement = NULL;
    const zlang_ast_expression_t *_message = NULL;
    zlang_parser_advance(parser);

    if (!zlang_parser_expect(parser, zlang_token_left_paren_, "expected '(' after print")) {
        return NULL;
    }
    _message = zlang_parse_expression(parser);
    if (_message == NULL) {
        return NULL;
    }
    if (_message->kind_ != zlang_ast_expression_string_literal_) {
        zlang_parser_unsupported(parser, "print currently accepts string literals only");
        return NULL;
    }
    if (!zlang_parser_expect(parser, zlang_token_right_paren_,
                             "expected ')' after print argument") ||
        !zlang_parser_expect(parser, zlang_token_semicolon_, "expected ';' after print")) {
        return NULL;
    }

    _statement = zlang_parser_add_statement(parser);
    if (_statement == NULL) {
        return NULL;
    }
    _statement->kind_ = zlang_ast_statement_print_;
    _statement->initializer_ = _message;
    _statement->line_ = _keyword->line_;
    _statement->column_ = _keyword->column_;
    return _statement;
}

static const zlang_ast_statement_t *zlang_parse_foreach(zlang_parser_t *parser) {
    const zlang_token_t *_keyword = zlang_parser_current(parser);
    const zlang_token_t *_item = NULL;
    const zlang_token_t *_iterable = NULL;
    zlang_ast_statement_t *_statement = NULL;
    size_t _body_start = 0U;
    size_t _body_count = 0U;

    zlang_parser_advance(parser);
    if (!zlang_parser_expect(parser, zlang_token_left_paren_, "expected '(' after foreach")) {
        return NULL;
    }
    _item = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected an item name")) {
        return NULL;
    }
    if (!zlang_parser_expect(parser, zlang_token_in_, "expected 'in'")) {
        return NULL;
    }
    _iterable = zlang_parser_current(parser);
    if (!zlang_parser_expect(parser, zlang_token_identifier_, "expected an iterable name")) {
        return NULL;
    }
    if (!zlang_parser_expect(parser, zlang_token_right_paren_,
                             "expected ')' after foreach header") ||
        !zlang_parser_expect(parser, zlang_token_left_brace_,
                             "expected '{' after foreach header")) {
        return NULL;
    }

    _body_start = parser->statement_count_;
    while (zlang_parser_current(parser)->kind_ != zlang_token_right_brace_ &&
           zlang_parser_current(parser)->kind_ != zlang_token_end_of_file_) {
        if (zlang_parser_current(parser)->kind_ != zlang_token_print_) {
            zlang_parser_unsupported(parser, "foreach body accepts print only");
            return NULL;
        }
        if (zlang_parse_print(parser) == NULL) {
            return NULL;
        }
        ++_body_count;
    }
    if (!zlang_parser_expect(parser, zlang_token_right_brace_, "expected '}' after foreach body")) {
        return NULL;
    }

    _statement = zlang_parser_add_statement(parser);
    if (_statement == NULL) {
        return NULL;
    }
    _statement->kind_ = zlang_ast_statement_foreach_;
    _statement->type_name_ = "integer";
    _statement->name_ = _item->text_;
    _statement->iterable_name_ = _iterable->text_;
    _statement->body_ = &parser->statements_[_body_start];
    _statement->body_count_ = _body_count;
    _statement->line_ = _keyword->line_;
    _statement->column_ = _keyword->column_;
    return _statement;
}

static const zlang_ast_statement_t *zlang_parse_statement(zlang_parser_t *parser) {
    switch (zlang_parser_current(parser)->kind_) {
    case zlang_token_integer_:
    case zlang_token_unsigned_:
    case zlang_token_boolean_:
    case zlang_token_string_:
        return zlang_parse_declaration(parser);
    case zlang_token_print_:
        return zlang_parse_print(parser);
    case zlang_token_foreach_:
        return zlang_parse_foreach(parser);
    default:
        zlang_parser_unsupported(parser, "unsupported statement");
        return NULL;
    }
}

void zlang_parser_init(zlang_parser_t *parser, zlang_token_t *tokens, size_t token_capacity,
                       zlang_ast_expression_t *expressions, size_t expression_capacity,
                       zlang_ast_statement_t *statements, size_t statement_capacity,
                       zlang_diagnostics_t *diagnostics) {
    parser->tokens_ = tokens;
    parser->token_capacity_ = token_capacity;
    parser->token_count_ = 0U;
    parser->expressions_ = expressions;
    parser->expression_capacity_ = expression_capacity;
    parser->expression_count_ = 0U;
    parser->statements_ = statements;
    parser->statement_capacity_ = statement_capacity;
    parser->statement_count_ = 0U;
    parser->diagnostics_ = diagnostics;
    parser->position_ = 0U;
    parser->failed_ = false;
}

bool zlang_parse(zlang_parser_t *parser, const char *source, char *scratch, size_t scratch_capacity,
                 zlang_parse_result_t *result) {
    zlang_lexer_init(&parser->lexer_, source, scratch, scratch_capacity);
    if (!zlang_lex(&parser->lexer_, parser->tokens_, parser->token_capacity_, &parser->token_count_,
                   parser->diagnostics_)) {
        return false;
    }

    while (zlang_parser_current(parser)->kind_ != zlang_token_end_of_file_) {
        if (zlang_parse_statement(parser) == NULL) {
            return false;
        }
    }

    result->program_.statements_ = parser->statements_;
    result->program_.statement_count_ = parser->statement_count_;
    result->token_count_ = parser->token_count_;
    result->expression_count_ = parser->expression_count_;
    return !parser->failed_;
}
