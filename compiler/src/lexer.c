/** @file
 * @brief Tokenization for Zlang 0.1 source text.
 */

#include "zlang/compiler/lexer.h"

#include <string.h>

typedef struct zlang_keyword_t {
    const char *text_;
    zlang_token_kind_t kind_;
} zlang_keyword_t;

static const zlang_keyword_t keywords_k[] = {{"boolean", zlang_token_boolean_},
                                             {"else", zlang_token_else_},
                                             {"false", zlang_token_false_},
                                             {"foreach", zlang_token_foreach_},
                                             {"function", zlang_token_function_},
                                             {"if", zlang_token_if_},
                                             {"in", zlang_token_in_},
                                             {"integer", zlang_token_integer_},
                                             {"print", zlang_token_print_},
                                             {"repeat", zlang_token_repeat_},
                                             {"return", zlang_token_return_},
                                             {"string", zlang_token_string_},
                                             {"true", zlang_token_true_},
                                             {"unsigned", zlang_token_unsigned_},
                                             {"while", zlang_token_while_},
                                             {"break", zlang_token_break_}};

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
    uint64_t _value = 0U;
    token->kind_ = zlang_token_integer_literal_;
    token->text_ = lexer->source_ + lexer->position_;
    token->line_ = line;
    token->column_ = column;

    while (lexer->source_[lexer->position_] >= '0' && lexer->source_[lexer->position_] <= '9') {
        const uint32_t _digit_k = (uint32_t)(lexer->source_[lexer->position_] - '0');
        if (_value > (UINT64_MAX - _digit_k) / 10U) {
            _value = UINT64_MAX;
        } else {
            _value = _value * 10U + _digit_k;
        }
        zlang_lexer_advance(lexer);
    }

    if (lexer->source_[lexer->position_] == 'u' || lexer->source_[lexer->position_] == 'U') {
        token->is_unsigned_ = true;
        zlang_lexer_advance(lexer);
    }

    token->length_ = (uint32_t)((lexer->source_ + lexer->position_) - token->text_);
    if (token->is_unsigned_) {
        if (_value > UINT32_MAX) {
            (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_LITERAL_OUT_OF_RANGE, line,
                                        column, "unsigned literal out of range");
            return false;
        }
    } else if (_value > INT32_MAX) {
        (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_LITERAL_OUT_OF_RANGE, line,
                                    column, "integer literal out of range");
        return false;
    }
    token->integer_value_ = (int32_t)_value;
    return true;
}

static bool zlang_lex_operator(zlang_lexer_t *lexer, zlang_token_t *token,
                               zlang_diagnostics_t *diagnostics, const uint32_t line,
                               const uint32_t column) {
    const char _character_k = lexer->source_[lexer->position_];
    token->text_ = lexer->source_ + lexer->position_;
    token->length_ = 1U;
    token->line_ = line;
    token->column_ = column;

    if (_character_k == '=' && lexer->source_[lexer->position_ + 1U] == '=') {
        token->kind_ = zlang_token_equal_equal_;
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
    case '=':
        token->kind_ = zlang_token_equal_;
        break;
    case '+':
        token->kind_ = zlang_token_plus_;
        break;
    default:
        (void)zlang_diagnostics_add(diagnostics, ZLANG_DIAGNOSTIC_INVALID_CHARACTER, line, column,
                                    "invalid character");
        return false;
    }
    zlang_lexer_advance(lexer);
    return true;
}

void zlang_lexer_init(zlang_lexer_t *lexer, const char *source, char *scratch,
                      size_t scratch_capacity) {
    lexer->source_ = source;
    lexer->scratch_ = scratch;
    lexer->scratch_capacity_ = scratch_capacity;
    lexer->scratch_length_ = 0U;
    lexer->position_ = 0U;
    lexer->line_ = 1U;
    lexer->column_ = 1U;
}

bool zlang_lex(zlang_lexer_t *lexer, zlang_token_t *tokens, size_t token_capacity,
               size_t *token_count, zlang_diagnostics_t *diagnostics) {
    *token_count = 0U;

    for (;;) {
        const char _character_k = lexer->source_[lexer->position_];
        zlang_token_t _token;
        memset(&_token, 0, sizeof(_token));

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
            tokens[(*token_count)++] = _token;
            return true;
        }

        if (_character_k == ' ' || _character_k == '\t' || _character_k == '\r' ||
            _character_k == '\n') {
            zlang_lexer_advance(lexer);
            continue;
        }

        if (_character_k == '/' && lexer->source_[lexer->position_ + 1U] == '/') {
            while (lexer->source_[lexer->position_] != '\0' &&
                   lexer->source_[lexer->position_] != '\n') {
                zlang_lexer_advance(lexer);
            }
            continue;
        }

        if (_character_k == '/' && lexer->source_[lexer->position_ + 1U] == '*') {
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
