/** @file
 * @brief Tokenization for Zlang source text.
 */

#include "zlang/compiler/lexer.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct zlang_keyword_t {
    const char *text_;
    zlang_token_kind_t kind_;
} zlang_keyword_t;

static const zlang_keyword_t keywords_k[] = {{"boolean", zlang_token_boolean_},
                                             {"clone", zlang_token_clone_},
                                             {"const", zlang_token_const_},
                                             {"else", zlang_token_else_},
                                             {"enum", zlang_token_enum_},
                                             {"false", zlang_token_false_},
                                             {"for", zlang_token_for_},
                                             {"foreach", zlang_token_foreach_},
                                             {"constructor", zlang_token_constructor_},
                                             {"function", zlang_token_function_},
                                             {"static", zlang_token_static_},
                                             {"trait", zlang_token_trait_},
                                             {"uses", zlang_token_uses_},
                                             {"requires", zlang_token_requires_},
                                             {"import", zlang_token_import_},
                                             {"from", zlang_token_from_},
                                             {"if", zlang_token_if_},
                                             {"in", zlang_token_in_},
                                             {"int32", zlang_token_integer_},
                                             {"int64", zlang_token_int64_},
                                             {"uint64", zlang_token_uint64_},
                                             {"float", zlang_token_float_},
                                             {"double", zlang_token_double_},
                                             {"length", zlang_token_length_},
                                             {"struct", zlang_token_struct_},
                                             {"reference", zlang_token_reference_},
                                             {"arguments", zlang_token_arguments_},
                                             {"array", zlang_token_array_},
                                             {"repeat", zlang_token_repeat_},
                                             {"return", zlang_token_return_},
                                             {"string", zlang_token_string_},
                                             {"true", zlang_token_true_},
                                             {"uint32", zlang_token_unsigned_},
                                             {"while", zlang_token_while_},
                                             {"break", zlang_token_break_},
                                             {"continue", zlang_token_continue_},
                                             {"jump", zlang_token_jump_},
                                             {"switch", zlang_token_switch_},
                                             {"case", zlang_token_case_},
                                             {"default", zlang_token_default_},
                                             {"do", zlang_token_do_},
                                             {"match", zlang_token_match_}};

static void zlang_lexer_advance(zlang_lexer_t *lexer) {
    const char _character_k = lexer->source_[lexer->position_];
    ++lexer->position_;
    if (_character_k == '\n') {
        ++lexer->line_;
        lexer->column_ = 1U;
    } else {
        ++lexer->column_;
    }
}

static bool zlang_is_identifier_start(const char character) {
    return (character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z') ||
           character == '_';
}

static bool zlang_is_identifier_continue(const char character) {
    return zlang_is_identifier_start(character) || (character >= '0' && character <= '9');
}

static bool zlang_identifier_has_prefix(const char *text, const char *prefix) {
    return strncmp(text, prefix, strlen(prefix)) == 0;
}

static bool zlang_identifier_is_reserved(const char *text) {
    return zlang_identifier_has_prefix(text, "zlang") ||
           zlang_identifier_has_prefix(text, "_zlang") ||
           zlang_identifier_has_prefix(text, "ZLANG");
}

static zlang_token_kind_t zlang_keyword_kind(const char *text, size_t length) {
    for (size_t _index = 0U; _index < sizeof(keywords_k) / sizeof(keywords_k[0]); ++_index) {
        if (strlen(keywords_k[_index].text_) == length &&
            memcmp(keywords_k[_index].text_, text, length) == 0) {
            return keywords_k[_index].kind_;
        }
    }
    return zlang_token_identifier_;
}

static bool zlang_token_fits(const size_t token_count, const size_t capacity,
                             zlang_diagnostics_t *diagnostics, const uint32_t line,
                             const uint32_t column) {
    if (token_count < capacity) {
        return true;
    }
    (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, line, column,
                                "token capacity exceeded");
    return false;
}

static bool zlang_scratch_append(zlang_lexer_t *lexer, const char character,
                                 zlang_diagnostics_t *diagnostics, const uint32_t line,
                                 const uint32_t column) {
    if (lexer->scratch_length_ >= lexer->scratch_capacity_) {
        (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_INVALID_CHARACTER, line, column,
                                    "scratch buffer too small");
        return false;
    }
    lexer->scratch_[lexer->scratch_length_] = character;
    ++lexer->scratch_length_;
    return true;
}

static bool zlang_lex_string(zlang_lexer_t *lexer, zlang_token_t *token,
                             zlang_diagnostics_t *diagnostics, const uint32_t line,
                             const uint32_t column) {
    zlang_lexer_advance(lexer);
    token->kind_ = zlang_token_string_literal_;
    token->text_ = lexer->scratch_ + lexer->scratch_length_;
    token->line_ = line;
    token->column_ = column;

    for (;;) {
        const char _character_k = lexer->source_[lexer->position_];
        if (_character_k == '\0' || _character_k == '\n') {
            (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_UNTERMINATED_STRING, line,
                                        column, "unterminated string literal");
            return false;
        }
        zlang_lexer_advance(lexer);
        if (_character_k == '"') {
            token->length_ =
                (uint32_t)(lexer->scratch_length_ - (size_t)(token->text_ - lexer->scratch_));
            return zlang_scratch_append(lexer, '\0', diagnostics, line, column);
        }
        if (_character_k == '\\') {
            const char _escape_k = lexer->source_[lexer->position_];
            if (_escape_k != 'n' && _escape_k != 't' && _escape_k != '"' && _escape_k != '\\') {
                (void)zlang_diagnostics_add(diagnostics,
                                            _escape_k == '\0' ? ZLANG_DIAGNOSTIC_UNTERMINATED_STRING
                                                              : ZLANG_DIAGNOSTIC_INVALID_CHARACTER,
                                            line, column, "unknown or unterminated string escape");
                return false;
            }
            zlang_lexer_advance(lexer);
            if (!zlang_scratch_append(lexer, '\\', diagnostics, line, column) ||
                !zlang_scratch_append(lexer, _escape_k, diagnostics, line, column)) {
                return false;
            }
            ++token->value_length_;
        } else {
            if (!zlang_scratch_append(lexer, _character_k, diagnostics, line, column)) {
                return false;
            }
            ++token->value_length_;
        }
    }
}

static bool zlang_lex_number(zlang_lexer_t *lexer, zlang_token_t *token,
                             zlang_diagnostics_t *diagnostics, const uint32_t line,
                             const uint32_t column) {
    uint64_t _value_k = 0U;
    bool _overflow_k = false;
    bool _floating_k = false;
    token->kind_ = zlang_token_integer_literal_;
    token->text_ = lexer->source_ + lexer->position_;
    token->line_ = line;
    token->column_ = column;

    while (lexer->source_[lexer->position_] >= '0' && lexer->source_[lexer->position_] <= '9') {
        const uint32_t _digit_k = (uint32_t)(lexer->source_[lexer->position_] - '0');
        if (_value_k > (UINT64_MAX - _digit_k) / 10U) {
            _overflow_k = true;
        } else {
            _value_k = _value_k * 10U + _digit_k;
        }
        zlang_lexer_advance(lexer);
    }

    if (lexer->source_[lexer->position_] == '.') {
        const char _next_k = lexer->source_[lexer->position_ + 1U];
        if (_next_k >= '0' && _next_k <= '9') {
            _floating_k = true;
            zlang_lexer_advance(lexer);
            while (lexer->source_[lexer->position_] >= '0' &&
                   lexer->source_[lexer->position_] <= '9') {
                zlang_lexer_advance(lexer);
            }
        }
    }
    if (lexer->source_[lexer->position_] == 'e' || lexer->source_[lexer->position_] == 'E') {
        _floating_k = true;
        zlang_lexer_advance(lexer);
        if (lexer->source_[lexer->position_] == '+' || lexer->source_[lexer->position_] == '-') {
            zlang_lexer_advance(lexer);
        }
        if (lexer->source_[lexer->position_] < '0' || lexer->source_[lexer->position_] > '9') {
            (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_UNEXPECTED_TOKEN, line,
                                        column, "exponent requires decimal digits");
            return false;
        }
        while (lexer->source_[lexer->position_] >= '0' && lexer->source_[lexer->position_] <= '9') {
            zlang_lexer_advance(lexer);
        }
    }

    if (_floating_k) {
        const size_t _start_k = lexer->scratch_length_;
        const size_t _length_k = (size_t)((lexer->source_ + lexer->position_) - token->text_);
        if (_length_k + 1U > lexer->scratch_capacity_ - _start_k) {
            (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, line,
                                        column, "numeric literal is too long");
            return false;
        }
        for (size_t _index = 0U; _index < _length_k; ++_index) {
            lexer->scratch_[_start_k + _index] = token->text_[_index];
        }
        lexer->scratch_[_start_k + _length_k] = '\0';
        lexer->scratch_length_ = _start_k + _length_k + 1U;
        char *_end_k = NULL;
        token->float_value_ = strtod(lexer->scratch_ + _start_k, &_end_k);
        if (_end_k == NULL || *_end_k != '\0' || !isfinite(token->float_value_)) {
            (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_LITERAL_OUT_OF_RANGE, line,
                                        column, "floating literal out of range");
            return false;
        }
        token->kind_ = zlang_token_float_literal_;
        token->is_float_ =
            lexer->source_[lexer->position_] == 'f' || lexer->source_[lexer->position_] == 'F';
        if (token->is_float_) {
            zlang_lexer_advance(lexer);
        }
        token->length_ = (uint32_t)((lexer->source_ + lexer->position_) - token->text_);
        return true;
    }

    if (lexer->source_[lexer->position_] == 'f' || lexer->source_[lexer->position_] == 'F') {
        token->float_value_ = (double)_value_k;
        token->is_float_ = true;
        token->kind_ = zlang_token_float_literal_;
        zlang_lexer_advance(lexer);
        token->length_ = (uint32_t)((lexer->source_ + lexer->position_) - token->text_);
        return true;
    }

    const bool _u64_k =
        lexer->source_[lexer->position_] == 'u' || lexer->source_[lexer->position_] == 'U';
    const bool _i64_k =
        lexer->source_[lexer->position_] == 'i' || lexer->source_[lexer->position_] == 'I';
    if (_u64_k || _i64_k) {
        const char _second_k = lexer->source_[lexer->position_ + 1U];
        const char _third_k = lexer->source_[lexer->position_ + 2U];
        if (_second_k == '6' && _third_k == '4') {
            if (_i64_k) {
                token->is_int64_ = true;
            } else {
                token->is_uint64_ = true;
            }
            zlang_lexer_advance(lexer);
            zlang_lexer_advance(lexer);
            zlang_lexer_advance(lexer);
        } else if (_u64_k) {
            token->is_unsigned_ = true;
            zlang_lexer_advance(lexer);
        }
    }

    token->length_ = (uint32_t)((lexer->source_ + lexer->position_) - token->text_);
    token->integer64_value_ = _value_k;
    if (token->is_int64_) {
        if (_overflow_k || _value_k > (uint64_t)INT64_MAX) {
            (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_LITERAL_OUT_OF_RANGE, line,
                                        column, "int64 literal out of range");
            return false;
        }
    } else if (token->is_uint64_) {
        if (_overflow_k) {
            (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_LITERAL_OUT_OF_RANGE, line,
                                        column, "uint64 literal out of range");
            return false;
        }
    } else if (token->is_unsigned_) {
        if (_overflow_k || _value_k > UINT32_MAX) {
            (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_LITERAL_OUT_OF_RANGE, line,
                                        column, "uint32 literal out of range");
            return false;
        }
    } else if (_overflow_k || _value_k > (uint64_t)INT32_MAX) {
        (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_LITERAL_OUT_OF_RANGE, line,
                                    column, "int32 literal out of range");
        return false;
    }
    token->integer_value_ = (int32_t)_value_k;
    return true;
}

static zlang_token_kind_t zlang_compound_token_kind(const char character) {
    switch (character) {
    case '+':
        return zlang_token_plus_equal_;
    case '-':
        return zlang_token_minus_equal_;
    case '*':
        return zlang_token_star_equal_;
    case '/':
        return zlang_token_slash_equal_;
    default:
        return zlang_token_percent_equal_;
    }
}

static bool zlang_lex_operator(zlang_lexer_t *lexer, zlang_token_t *token,
                               zlang_diagnostics_t *diagnostics, const uint32_t line,
                               const uint32_t column) {
    const char _character_k = lexer->source_[lexer->position_];
    token->text_ = lexer->source_ + lexer->position_;
    token->length_ = 1U;
    token->line_ = line;
    token->column_ = column;

    const char _next_k = lexer->source_[lexer->position_ + 1U];
    if (_character_k == '.' && _next_k == '.') {
        token->kind_ = zlang_token_dot_dot_;
        token->length_ = 2U;
        zlang_lexer_advance(lexer);
        zlang_lexer_advance(lexer);
        return true;
    }
    if (_character_k == '=' && _next_k == '>') {
        token->kind_ = zlang_token_arrow_;
        token->length_ = 2U;
        zlang_lexer_advance(lexer);
        zlang_lexer_advance(lexer);
        return true;
    }
    if ((_character_k == '&' && _next_k == '&') || (_character_k == '|' && _next_k == '|')) {
        token->kind_ = _character_k == '&' ? zlang_token_and_ : zlang_token_or_;
        token->length_ = 2U;
        zlang_lexer_advance(lexer);
        zlang_lexer_advance(lexer);
        return true;
    }
    if ((_character_k == '<' && _next_k == '<') || (_character_k == '>' && _next_k == '>')) {
        token->kind_ = _character_k == '<' ? zlang_token_shift_left_ : zlang_token_shift_right_;
        token->length_ = 2U;
        zlang_lexer_advance(lexer);
        zlang_lexer_advance(lexer);
        return true;
    }
    if (_next_k == '=' && (_character_k == '+' || _character_k == '-' || _character_k == '*' ||
                           _character_k == '/' || _character_k == '%')) {
        token->kind_ = zlang_compound_token_kind(_character_k);
        token->length_ = 2U;
        zlang_lexer_advance(lexer);
        zlang_lexer_advance(lexer);
        return true;
    }
    if ((_character_k == '=' || _character_k == '!' || _character_k == '<' ||
         _character_k == '>') &&
        _next_k == '=') {
        if (_character_k == '=') {
            token->kind_ = zlang_token_equal_equal_;
        } else if (_character_k == '!') {
            token->kind_ = zlang_token_not_equal_;
        } else if (_character_k == '<') {
            token->kind_ = zlang_token_less_equal_;
        } else {
            token->kind_ = zlang_token_greater_equal_;
        }
        token->length_ = 2U;
        zlang_lexer_advance(lexer);
        zlang_lexer_advance(lexer);
        return true;
    }

    switch (_character_k) {
    case '(':
        token->kind_ = zlang_token_left_paren_;
        break;
    case ')':
        token->kind_ = zlang_token_right_paren_;
        break;
    case '{':
        token->kind_ = zlang_token_left_brace_;
        break;
    case '}':
        token->kind_ = zlang_token_right_brace_;
        break;
    case '[':
        token->kind_ = zlang_token_left_bracket_;
        break;
    case ']':
        token->kind_ = zlang_token_right_bracket_;
        break;
    case ';':
        token->kind_ = zlang_token_semicolon_;
        break;
    case ',':
        token->kind_ = zlang_token_comma_;
        break;
    case ':':
        token->kind_ = zlang_token_colon_;
        break;
    case '.':
        token->kind_ = zlang_token_dot_;
        break;
    case '&':
        token->kind_ = zlang_token_ampersand_;
        break;
    case '|':
        token->kind_ = zlang_token_pipe_;
        break;
    case '^':
        token->kind_ = zlang_token_caret_;
        break;
    case '~':
        token->kind_ = zlang_token_tilde_;
        break;
    case '=':
        token->kind_ = zlang_token_equal_;
        break;
    case '?':
        token->kind_ = zlang_token_question_;
        break;
    case '!':
        token->kind_ = zlang_token_not_;
        break;
    case '<':
        token->kind_ = zlang_token_less_;
        break;
    case '>':
        token->kind_ = zlang_token_greater_;
        break;
    case '+':
        token->kind_ = zlang_token_plus_;
        break;
    case '-':
        token->kind_ = zlang_token_minus_;
        break;
    case '*':
        token->kind_ = zlang_token_star_;
        break;
    case '/':
        token->kind_ = zlang_token_slash_;
        break;
    case '%':
        token->kind_ = zlang_token_percent_;
        break;
    default:
        (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_INVALID_CHARACTER, line, column,
                                    "invalid character");
        return false;
    }
    zlang_lexer_advance(lexer);
    return true;
}

static bool zlang_lexer_add_comment(zlang_lexer_t *lexer, const char *text, size_t length,
                                    uint32_t line, uint32_t end_line, uint32_t column,
                                    bool is_block, zlang_diagnostics_t *diagnostics) {
    if (lexer->comments_ == NULL || lexer->comment_count_ >= lexer->comment_capacity_ ||
        length > UINT32_MAX) {
        (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT, line,
                                    column, "comment capacity exceeded");
        return false;
    }
    zlang_comment_t *const _comment_k = &lexer->comments_[lexer->comment_count_++];
    _comment_k->text_ = text;
    _comment_k->length_ = (uint32_t)length;
    _comment_k->line_ = line;
    _comment_k->end_line_ = end_line;
    _comment_k->column_ = column;
    _comment_k->is_block_ = is_block;
    return true;
}

void zlang_lexer_init(zlang_lexer_t *lexer, const char *source, char *scratch,
                      size_t scratch_capacity, zlang_comment_t *comments, size_t comment_capacity) {
    lexer->source_ = source;
    lexer->scratch_ = scratch;
    lexer->scratch_capacity_ = scratch_capacity;
    lexer->scratch_length_ = 0U;
    lexer->position_ = 0U;
    lexer->line_ = 1U;
    lexer->column_ = 1U;
    lexer->comments_ = comments;
    lexer->comment_capacity_ = comment_capacity;
    lexer->comment_count_ = 0U;
}

bool zlang_lex(zlang_lexer_t *lexer, zlang_token_t *tokens, size_t token_capacity,
               size_t *token_count, zlang_diagnostics_t *diagnostics) {
    *token_count = 0U;

    for (;;) {
        const char _character_k = lexer->source_[lexer->position_];
        zlang_token_t _token;
        memset(&_token, 0, sizeof(_token));
        const size_t _offset_k = lexer->position_;
        _token.offset_ = _offset_k;

        if (_character_k == '\0') {
            if (!zlang_token_fits(*token_count, token_capacity, diagnostics, lexer->line_,
                                  lexer->column_)) {
                return false;
            }
            _token.kind_ = zlang_token_end_of_file_;
            _token.text_ = lexer->source_ + lexer->position_;
            _token.length_ = 0U;
            _token.line_ = lexer->line_;
            _token.column_ = lexer->column_;
            _token.offset_ = _offset_k;
            tokens[(*token_count)++] = _token;
            return true;
        }

        if (_character_k == ' ' || _character_k == '\t' || _character_k == '\r' ||
            _character_k == '\n') {
            zlang_lexer_advance(lexer);
            continue;
        }

        if (_character_k == '/' && lexer->source_[lexer->position_ + 1U] == '/') {
            const char *const _text_k = lexer->source_ + lexer->position_;
            const uint32_t _line_k = lexer->line_;
            const uint32_t _column_k = lexer->column_;
            while (lexer->source_[lexer->position_] != '\0' &&
                   lexer->source_[lexer->position_] != '\n') {
                zlang_lexer_advance(lexer);
            }
            const size_t _length_k = (size_t)((lexer->source_ + lexer->position_) - _text_k);
            if (!zlang_lexer_add_comment(lexer, _text_k, _length_k, _line_k, lexer->line_,
                                         _column_k, false, diagnostics)) {
                return false;
            }
            continue;
        }

        if (_character_k == '/' && lexer->source_[lexer->position_ + 1U] == '*') {
            const char *const _text_k = lexer->source_ + lexer->position_;
            const uint32_t _line_k = lexer->line_;
            const uint32_t _column_k = lexer->column_;
            zlang_lexer_advance(lexer);
            zlang_lexer_advance(lexer);
            for (;;) {
                if (lexer->source_[lexer->position_] == '\0') {
                    (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_INVALID_CHARACTER,
                                                lexer->line_, lexer->column_,
                                                "unterminated block comment");
                    return false;
                }
                if (lexer->source_[lexer->position_] == '*' &&
                    lexer->source_[lexer->position_ + 1U] == '/') {
                    zlang_lexer_advance(lexer);
                    zlang_lexer_advance(lexer);
                    break;
                }
                zlang_lexer_advance(lexer);
            }
            const size_t _length_k = (size_t)((lexer->source_ + lexer->position_) - _text_k);
            if (!zlang_lexer_add_comment(lexer, _text_k, _length_k, _line_k, lexer->line_,
                                         _column_k, true, diagnostics)) {
                return false;
            }
            continue;
        }

        if (!zlang_token_fits(*token_count, token_capacity, diagnostics, lexer->line_,
                              lexer->column_)) {
            return false;
        }

        if (zlang_is_identifier_start(_character_k)) {
            const size_t _start_length_k = lexer->scratch_length_;
            _token.text_ = lexer->scratch_ + _start_length_k;
            _token.line_ = lexer->line_;
            _token.column_ = lexer->column_;
            while (zlang_is_identifier_continue(lexer->source_[lexer->position_])) {
                if (lexer->scratch_length_ + 1U >= lexer->scratch_capacity_) {
                    (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_INVALID_CHARACTER,
                                                lexer->line_, lexer->column_,
                                                "scratch buffer too small");
                    return false;
                }
                lexer->scratch_[lexer->scratch_length_] = lexer->source_[lexer->position_];
                ++lexer->scratch_length_;
                zlang_lexer_advance(lexer);
            }
            if (lexer->scratch_length_ >= lexer->scratch_capacity_) {
                (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_INVALID_CHARACTER,
                                            lexer->line_, lexer->column_,
                                            "scratch buffer too small");
                return false;
            }
            lexer->scratch_[lexer->scratch_length_] = '\0';
            ++lexer->scratch_length_;
            _token.length_ = (uint32_t)(lexer->scratch_length_ - _start_length_k - 1U);
            _token.kind_ = zlang_keyword_kind(_token.text_, _token.length_);
            if (_token.kind_ == zlang_token_identifier_ &&
                zlang_identifier_is_reserved(_token.text_)) {
                (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT,
                                            _token.line_, _token.column_,
                                            "identifier uses the reserved zlang namespace");
                return false;
            }
        } else if (_character_k >= '0' && _character_k <= '9') {
            if (!zlang_lex_number(lexer, &_token, diagnostics, lexer->line_, lexer->column_)) {
                return false;
            }
        } else if (_character_k == '"') {
            if (!zlang_lex_string(lexer, &_token, diagnostics, lexer->line_, lexer->column_)) {
                return false;
            }
        } else if (!zlang_lex_operator(lexer, &_token, diagnostics, lexer->line_, lexer->column_)) {
            return false;
        }

        tokens[(*token_count)++] = _token;
    }
}

bool zlang_token_is(const zlang_token_t *token, const char *text) {
    const size_t _length_k = strlen(text);
    return token->length_ == _length_k && memcmp(token->text_, text, _length_k) == 0;
}
