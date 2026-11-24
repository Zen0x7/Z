/** @file
 * @brief C99 emitter for the Zlang 0.2 abstract syntax tree.
 */

#include "zlang/compiler/emitter.h"

#include <stdio.h>
#include <string.h>

void zlang_emitter_init(zlang_emitter_t *emitter, char *buffer, size_t capacity,
                        zlang_diagnostics_t *diagnostics, const zlang_memory_plan_t *plan) {
    emitter->buffer_ = buffer;
    emitter->capacity_ = capacity;
    emitter->length_ = 0U;
    emitter->diagnostics_ = diagnostics;
    emitter->plan_ = plan;
    emitter->next_index_ = 0U;
    emitter->emitted_type_count_ = 0U;
    emitter->emitted_symbol_count_ = 0U;
    emitter->active_symbol_count_ = 0U;
    if (capacity > 0U) {
        buffer[0] = '\0';
    }
}

static bool zlang_append_text(zlang_emitter_t *emitter, const char *text) {
    if (emitter->length_ >= emitter->capacity_) {
        return false;
    }
    const size_t _length_k = strlen(text);
    const size_t _remaining_k = emitter->capacity_ - emitter->length_ - 1U;
    if (_length_k > _remaining_k) {
        return false;
    }
    memcpy(emitter->buffer_ + emitter->length_, text, _length_k);
    emitter->length_ += _length_k;
    emitter->buffer_[emitter->length_] = '\0';
    return true;
}

static bool zlang_append_integer(zlang_emitter_t *emitter, int32_t value) {
    char _digits[16];
    const int _written_k = snprintf(_digits, sizeof(_digits), "%ld", (long)value);
    return _written_k >= 0 && (size_t)_written_k < sizeof(_digits) &&
           zlang_append_text(emitter, _digits);
}

static bool zlang_append_unsigned(zlang_emitter_t *emitter, uint32_t value) {
    char _digits[16];
    const int _written_k = snprintf(_digits, sizeof(_digits), "%lu", (unsigned long)value);
    return _written_k >= 0 && (size_t)_written_k < sizeof(_digits) &&
           zlang_append_text(emitter, _digits);
}

static bool zlang_append_indent(zlang_emitter_t *emitter, uint32_t depth) {
    for (uint32_t _level = 0U; _level < depth; ++_level) {
        if (!zlang_append_text(emitter, "    ")) {
            return false;
        }
    }
    return true;
}

static const char *zlang_c_type(const char *type_name) {
    if (strcmp(type_name, "integer") == 0) {
        return "int32_t";
    }
    if (strcmp(type_name, "unsigned") == 0) {
        return "uint32_t";
    }
    if (strcmp(type_name, "boolean") == 0) {
        return "bool";
    }
    if (strcmp(type_name, "string") == 0) {
        return "zlang_string_t";
    }
    return "void";
}

static const char *zlang_type_suffix(const char *type_name) {
    if (strcmp(type_name, "integer") == 0) {
        return "i32";
    }
    if (strcmp(type_name, "unsigned") == 0) {
        return "u32";
    }
    if (strcmp(type_name, "boolean") == 0) {
        return "bool";
    }
    if (strcmp(type_name, "string") == 0) {
        return "string";
    }
    return "unknown";
}

static const char *zlang_default_initializer(const char *type_name) {
    if (strcmp(type_name, "integer") == 0) {
        return "0";
    }
    if (strcmp(type_name, "unsigned") == 0) {
        return "0U";
    }
    if (strcmp(type_name, "boolean") == 0) {
        return "false";
    }
    if (strcmp(type_name, "string") == 0) {
        return "(zlang_string_t){ 0, 0U }";
    }
    return "0";
}

static bool zlang_is_static(const zlang_emitter_t *emitter,
                            const zlang_ast_statement_t *statement) {
    return emitter->plan_ != NULL &&
           zlang_plan_storage_for(emitter->plan_, statement) == zlang_storage_static_;
}

static bool zlang_type_name(const zlang_ast_statement_t *statement, char *buffer, size_t capacity) {
    const int _written_k =
        snprintf(buffer, capacity, "zlang_sequence_%s_%lu_t",
                 zlang_type_suffix(statement->type_name_), (unsigned long)statement->capacity_);
    return _written_k >= 0 && (size_t)_written_k < capacity;
}

static bool zlang_type_was_emitted(const zlang_emitter_t *emitter, const char *name) {
    for (size_t _index = 0U; _index < emitter->emitted_type_count_; ++_index) {
        if (strcmp(emitter->emitted_types_[_index], name) == 0) {
            return true;
        }
    }
    return false;
}

static bool zlang_mark_type_emitted(zlang_emitter_t *emitter, const char *name) {
    if (zlang_type_was_emitted(emitter, name) ||
        emitter->emitted_type_count_ >= ZLANG_EMITTED_TYPE_CAPACITY) {
        return true;
    }
    (void)snprintf(emitter->emitted_types_[emitter->emitted_type_count_],
                   sizeof(emitter->emitted_types_[0]), "%s", name);
    ++emitter->emitted_type_count_;
    return true;
}

static const zlang_ast_statement_t *zlang_find_sequence(const zlang_emitter_t *emitter,
                                                        const char *name) {
    if (emitter->plan_ == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < emitter->plan_->count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k =
            emitter->plan_->entries_[_index].statement_;
        if (_statement_k->kind_ == zlang_statement_sequence_declaration_ &&
            _statement_k->name_ != NULL && strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

static bool zlang_name_is_used(const zlang_emitter_t *emitter, const char *name) {
    for (size_t _index = 0U; _index < emitter->emitted_symbol_count_; ++_index) {
        if (strcmp(emitter->emitted_symbols_[_index].generated_name_, name) == 0 ||
            strcmp(emitter->emitted_symbols_[_index].storage_name_, name) == 0) {
            return true;
        }
    }
    return false;
}

static bool zlang_copy_name(const char *source, char *output, size_t capacity) {
    if (source == NULL || source[0] == '\0') {
        return false;
    }
    const size_t _length_k = strlen(source);
    if (_length_k >= capacity) {
        return false;
    }
    memcpy(output, source, _length_k + 1U);
    return true;
}

static bool zlang_append_name_suffix(const char *source, uint32_t suffix, char *output,
                                     size_t capacity) {
    char _digits[16];
    const int _written_k = snprintf(_digits, sizeof(_digits), "%u", (unsigned int)suffix);
    if (_written_k < 0 || (size_t)_written_k >= sizeof(_digits)) {
        return false;
    }
    const size_t _source_length_k = strlen(source);
    const size_t _digit_length_k = (size_t)_written_k;
    if (_source_length_k + 1U + _digit_length_k >= capacity) {
        return false;
    }
    memcpy(output, source, _source_length_k + 1U);
    output[_source_length_k] = '_';
    memcpy(output + _source_length_k + 1U, _digits, _digit_length_k + 1U);
    return true;
}

static bool zlang_make_unique_name(zlang_emitter_t *emitter, const char *source, char *output,
                                   size_t capacity) {
    if (!zlang_copy_name(source, output, capacity)) {
        return false;
    }
    for (uint32_t _suffix = 0U; zlang_name_is_used(emitter, output); ++_suffix) {
        if (!zlang_append_name_suffix(source, emitter->next_index_++, output, capacity) ||
            _suffix == UINT32_MAX) {
            return false;
        }
    }
    return true;
}

static bool zlang_register_symbol(zlang_emitter_t *emitter, const char *source_name,
                                  const char *type_name, uint32_t capacity,
                                  bool has_explicit_capacity, bool needs_storage, char *generated,
                                  size_t generated_capacity) {
    if (emitter->emitted_symbol_count_ >= ZLANG_EMITTED_SYMBOL_CAPACITY ||
        emitter->active_symbol_count_ >= ZLANG_EMITTED_SYMBOL_CAPACITY ||
        !zlang_make_unique_name(emitter, source_name, generated, generated_capacity)) {
        return false;
    }
    zlang_emitted_symbol_t *const _symbol_k =
        &emitter->emitted_symbols_[emitter->active_symbol_count_];
    memset(_symbol_k, 0, sizeof(*_symbol_k));
    _symbol_k->source_name_ = source_name;
    _symbol_k->type_name_ = type_name;
    _symbol_k->capacity_ = capacity;
    _symbol_k->has_explicit_capacity_ = has_explicit_capacity;
    (void)snprintf(_symbol_k->generated_name_, sizeof(_symbol_k->generated_name_), "%s", generated);
    if (needs_storage) {
        char _base[ZLANG_EMITTED_NAME_CAPACITY];
        const size_t _generated_length_k = strlen(generated);
        const size_t _suffix_length_k = strlen("_storage");
        if (_generated_length_k + _suffix_length_k >= sizeof(_base)) {
            return false;
        }
        memcpy(_base, generated, _generated_length_k + 1U);
        memcpy(_base + _generated_length_k, "_storage", _suffix_length_k + 1U);
        if (!zlang_make_unique_name(emitter, _base, _symbol_k->storage_name_,
                                    sizeof(_symbol_k->storage_name_))) {
            return false;
        }
    }
    ++emitter->active_symbol_count_;
    ++emitter->emitted_symbol_count_;
    return true;
}

static const zlang_emitted_symbol_t *zlang_find_active_symbol(const zlang_emitter_t *emitter,
                                                              const char *source_name) {
    if (source_name == NULL) {
        return NULL;
    }
    for (size_t _index = emitter->active_symbol_count_; _index > 0U; --_index) {
        const zlang_emitted_symbol_t *const _symbol_k = &emitter->emitted_symbols_[_index - 1U];
        if (strcmp(_symbol_k->source_name_, source_name) == 0) {
            return _symbol_k;
        }
    }
    return NULL;
}

static const char *zlang_resolve_name(const zlang_emitter_t *emitter, const char *source_name) {
    const zlang_emitted_symbol_t *const _symbol_k = zlang_find_active_symbol(emitter, source_name);
    return _symbol_k != NULL ? _symbol_k->generated_name_ : source_name;
}

static size_t zlang_save_scope(const zlang_emitter_t *emitter) {
    return emitter->active_symbol_count_;
}

static void zlang_restore_scope(zlang_emitter_t *emitter, size_t saved) {
    emitter->active_symbol_count_ = saved;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_expression(const zlang_program_t *program, size_t index,
                                  zlang_emitter_t *emitter);

static const char *zlang_comparison_operator(zlang_expression_kind_t kind) {
    switch (kind) {
    case zlang_expression_equal_:
        return " == ";
    case zlang_expression_less_:
        return " < ";
    case zlang_expression_greater_:
        return " > ";
    case zlang_expression_less_equal_:
        return " <= ";
    case zlang_expression_greater_equal_:
        return " >= ";
    case zlang_expression_not_equal_:
        return " != ";
    default:
        return " == ";
    }
}

static bool zlang_emit_string_concat_call(const zlang_program_t *program, size_t index,
                                          const char *storage, uint32_t capacity,
                                          zlang_emitter_t *emitter);

/* Expression trees are bounded by the caller-owned expression array. */
// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_expression(const zlang_program_t *program, size_t index,
                                  zlang_emitter_t *emitter) { // NOLINT(misc-no-recursion)
    if (index == ZLANG_NO_EXPRESSION || index >= program->expression_count_) {
        return false;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    switch (_expression_k->kind_) {
    case zlang_expression_integer_literal_:
        if (_expression_k->is_unsigned_) {
            return zlang_append_unsigned(emitter, (uint32_t)_expression_k->integer_value_) &&
                   zlang_append_text(emitter, "U");
        }
        return zlang_append_integer(emitter, _expression_k->integer_value_);
    case zlang_expression_boolean_literal_:
        return zlang_append_text(emitter, _expression_k->bool_value_ ? "true" : "false");
    case zlang_expression_string_literal_:
        return zlang_append_text(emitter, "(zlang_string_t){ \"") &&
               zlang_append_text(emitter, _expression_k->string_.data_) &&
               zlang_append_text(emitter, "\", ") &&
               zlang_append_unsigned(emitter, _expression_k->string_.length_) &&
               zlang_append_text(emitter, "U }");
    case zlang_expression_identifier_:
        return zlang_append_text(emitter, zlang_resolve_name(emitter, _expression_k->name_));
    case zlang_expression_add_: {
        const zlang_ast_expression_t *const _left_k = &program->expressions_[_expression_k->left_];
        const bool _is_unsigned_k =
            _left_k->kind_ == zlang_expression_integer_literal_ && _left_k->is_unsigned_;
        if (!zlang_append_text(emitter, _is_unsigned_k ? "zlang_add_u32(" : "zlang_add_i32(") ||
            !zlang_emit_expression(program, _expression_k->left_, emitter) ||
            !zlang_append_text(emitter, ", ") ||
            !zlang_emit_expression(program, _expression_k->right_, emitter) ||
            !zlang_append_text(emitter, ")")) {
            return false;
        }
        return true;
    }
    case zlang_expression_equal_:
    case zlang_expression_less_:
    case zlang_expression_greater_:
    case zlang_expression_less_equal_:
    case zlang_expression_greater_equal_:
    case zlang_expression_not_equal_: {
        const char *const _operator_k = zlang_comparison_operator(_expression_k->kind_);
        return zlang_append_text(emitter, "(") &&
               zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, _operator_k) &&
               zlang_emit_expression(program, _expression_k->right_, emitter) &&
               zlang_append_text(emitter, ")");
    }
    case zlang_expression_assignment_: {
        const zlang_ast_expression_t *const _target_k =
            &program->expressions_[_expression_k->left_];
        const zlang_emitted_symbol_t *const _symbol_k =
            _target_k->kind_ == zlang_expression_identifier_
                ? zlang_find_active_symbol(emitter, _target_k->name_)
                : NULL;
        const zlang_ast_expression_t *const _value_k =
            &program->expressions_[_expression_k->right_];
        if (_symbol_k != NULL && _symbol_k->storage_name_[0] != '\0' &&
            _value_k->kind_ == zlang_expression_string_concat_) {
            return zlang_emit_string_concat_call(program, _expression_k->right_,
                                                 _symbol_k->storage_name_, _symbol_k->capacity_,
                                                 emitter);
        }
        return zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, " = ") &&
               zlang_emit_expression(program, _expression_k->right_, emitter);
    }
    case zlang_expression_print_call_:
        return zlang_append_text(emitter, "zlang_print(") &&
               zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, ")");
    case zlang_expression_append_call_: {
        const zlang_ast_statement_t *const _sequence_k =
            zlang_find_sequence(emitter, _expression_k->name_);
        const char *const _name_k = zlang_resolve_name(emitter, _expression_k->name_);
        if (_sequence_k == NULL) {
            return false;
        }
        if (!zlang_append_text(emitter, "zlang_array_append(") ||
            !zlang_append_text(emitter, _name_k) || !zlang_append_text(emitter, ".data_, &") ||
            !zlang_append_text(emitter, _name_k) || !zlang_append_text(emitter, ".length_, ") ||
            !zlang_append_unsigned(emitter, _sequence_k->capacity_) ||
            !zlang_append_text(emitter, "U, sizeof(") || !zlang_append_text(emitter, _name_k) ||
            !zlang_append_text(emitter, ".data_[0]), (") ||
            !zlang_append_text(emitter, zlang_c_type(_sequence_k->type_name_)) ||
            !zlang_append_text(emitter, "[]){ (") ||
            !zlang_emit_expression(program, _expression_k->right_, emitter) ||
            !zlang_append_text(emitter, ") })")) {
            return false;
        }
        return true;
    }
    case zlang_expression_index_: {
        const zlang_ast_expression_t *const _left_k = &program->expressions_[_expression_k->left_];
        const char *const _name_k = zlang_resolve_name(emitter, _left_k->name_);
        if (_name_k == NULL) {
            return false;
        }
        if (!zlang_append_text(emitter, "((") ||
            !zlang_emit_expression(program, _expression_k->right_, emitter) ||
            !zlang_append_text(emitter, ") < ") || !zlang_append_text(emitter, _name_k) ||
            !zlang_append_text(emitter, ".length_) ? ") || !zlang_append_text(emitter, _name_k) ||
            !zlang_append_text(emitter, ".data_[") ||
            !zlang_emit_expression(program, _expression_k->right_, emitter) ||
            !zlang_append_text(emitter, "] : (zlang_trap(2U), ") ||
            !zlang_append_text(emitter, _name_k) || !zlang_append_text(emitter, ".data_[0])")) {
            return false;
        }
        return true;
    }
    case zlang_expression_call_: {
        if (!zlang_append_text(emitter, zlang_resolve_name(emitter, _expression_k->name_)) ||
            !zlang_append_text(emitter, "(")) {
            return false;
        }
        for (size_t _argument = 0U; _argument < _expression_k->argument_count_; ++_argument) {
            const size_t _index_k = _expression_k->argument_start_ + _argument;
            if ((_argument > 0U && !zlang_append_text(emitter, ", ")) ||
                !zlang_emit_expression(program, _index_k, emitter)) {
                return false;
            }
        }
        return zlang_append_text(emitter, ")");
    }
    default:
        return false;
    }
}

static bool zlang_append_zero_list(zlang_emitter_t *emitter, uint32_t capacity,
                                   uint32_t initializer_count) {
    for (uint32_t _index = initializer_count; _index < capacity; ++_index) {
        if (_index > initializer_count && !zlang_append_text(emitter, ", ")) {
            return false;
        }
        if (!zlang_append_text(emitter, "0")) {
            return false;
        }
    }
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_sequence_values(const zlang_program_t *program,
                                       const zlang_ast_statement_t *statement,
                                       zlang_emitter_t *emitter) { // NOLINT(misc-no-recursion)
    for (size_t _index = 0U; _index < statement->initializer_count_; ++_index) {
        const size_t _expression_index_k = statement->initializer_start_ + _index;
        if ((_index > 0U && !zlang_append_text(emitter, ", ")) ||
            !zlang_emit_expression(program, _expression_index_k, emitter)) {
            return false;
        }
    }
    if (statement->initializer_count_ > 0U &&
        statement->initializer_count_ < statement->capacity_ && !zlang_append_text(emitter, ", ")) {
        return false;
    }
    return zlang_append_zero_list(emitter, statement->capacity_,
                                  (uint32_t)statement->initializer_count_);
}

// NOLINTNEXTLINE(misc-no-recursion)
static size_t zlang_concat_part_count(const zlang_program_t *program,
                                      size_t index) { // NOLINT(misc-no-recursion)
    if (index == ZLANG_NO_EXPRESSION || index >= program->expression_count_) {
        return 0U;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ != zlang_expression_string_concat_) {
        return 1U;
    }
    return zlang_concat_part_count(program, _expression_k->left_) +
           zlang_concat_part_count(program, _expression_k->right_);
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_concat_parts(const zlang_program_t *program, size_t index,
                                    zlang_emitter_t *emitter) { // NOLINT(misc-no-recursion)
    if (index == ZLANG_NO_EXPRESSION || index >= program->expression_count_) {
        return false;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ != zlang_expression_string_concat_) {
        return zlang_emit_expression(program, index, emitter);
    }
    return zlang_emit_concat_parts(program, _expression_k->left_, emitter) &&
           zlang_append_text(emitter, ", ") &&
           zlang_emit_concat_parts(program, _expression_k->right_, emitter);
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

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_folded_string_length(const zlang_program_t *program, size_t index,
                                       uint32_t *length) { // NOLINT(misc-no-recursion)
    if (!zlang_is_literal_string(program, index)) {
        return false;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ == zlang_expression_string_literal_) {
        *length = _expression_k->string_.length_;
        return true;
    }
    uint32_t _left_k = 0U;
    uint32_t _right_k = 0U;
    if (!zlang_folded_string_length(program, _expression_k->left_, &_left_k) ||
        !zlang_folded_string_length(program, _expression_k->right_, &_right_k) ||
        _left_k > UINT32_MAX - _right_k) {
        return false;
    }
    *length = _left_k + _right_k;
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_folded_string_data(const zlang_program_t *program, size_t index,
                                          zlang_emitter_t *emitter) { // NOLINT(misc-no-recursion)
    if (index == ZLANG_NO_EXPRESSION || index >= program->expression_count_) {
        return false;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ == zlang_expression_string_literal_) {
        return zlang_append_text(emitter, _expression_k->string_.data_);
    }
    if (_expression_k->kind_ != zlang_expression_string_concat_) {
        return false;
    }
    return zlang_emit_folded_string_data(program, _expression_k->left_, emitter) &&
           zlang_emit_folded_string_data(program, _expression_k->right_, emitter);
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_folded_string(const zlang_program_t *program, size_t index,
                                     zlang_emitter_t *emitter) { // NOLINT(misc-no-recursion)
    uint32_t _length_k = 0U;
    if (!zlang_folded_string_length(program, index, &_length_k) ||
        !zlang_append_text(emitter, "(zlang_string_t){ \"")) {
        return false;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ == zlang_expression_string_literal_) {
        if (!zlang_append_text(emitter, _expression_k->string_.data_)) {
            return false;
        }
    } else if (!zlang_emit_folded_string_data(program, _expression_k->left_, emitter) ||
               !zlang_emit_folded_string_data(program, _expression_k->right_, emitter)) {
        return false;
    }
    return zlang_append_text(emitter, "\", ") && zlang_append_unsigned(emitter, _length_k) &&
           zlang_append_text(emitter, "U }");
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_string_concat_call(const zlang_program_t *program, size_t index,
                                          const char *storage, uint32_t capacity,
                                          zlang_emitter_t *emitter) { // NOLINT(misc-no-recursion)
    const size_t _count_k = zlang_concat_part_count(program, index);
    if (_count_k == 0U || _count_k > UINT32_MAX) {
        return false;
    }
    return zlang_append_text(emitter, "zlang_string_concat(") &&
           zlang_append_text(emitter, storage) && zlang_append_text(emitter, ", ") &&
           zlang_append_unsigned(emitter, capacity) &&
           zlang_append_text(emitter, "U, (zlang_string_t[]){ ") &&
           zlang_emit_concat_parts(program, index, emitter) && zlang_append_text(emitter, " }, ") &&
           zlang_append_unsigned(emitter, (uint32_t)_count_k) && zlang_append_text(emitter, "U)");
}

static bool zlang_emit_index_assignment(const zlang_program_t *program,
                                        const zlang_ast_expression_t *expression,
                                        zlang_emitter_t *emitter, uint32_t depth) {
    const zlang_ast_expression_t *const _target_k = &program->expressions_[expression->left_];
    const zlang_ast_expression_t *const _name_k = &program->expressions_[_target_k->left_];
    const char *const _generated_k = zlang_resolve_name(emitter, _name_k->name_);
    char _index_name[32];
    const int _written_k = snprintf(_index_name, sizeof(_index_name), "_zlang_index_%lu",
                                    (unsigned long)emitter->next_index_++);
    if (_target_k->kind_ != zlang_expression_index_ || _generated_k == NULL || _written_k < 0 ||
        (size_t)_written_k >= sizeof(_index_name)) {
        return false;
    }
    return zlang_append_text(emitter, "{\n") && zlang_append_indent(emitter, depth + 1U) &&
           zlang_append_text(emitter, "uint32_t ") && zlang_append_text(emitter, _index_name) &&
           zlang_append_text(emitter, " = ") &&
           zlang_emit_expression(program, _target_k->right_, emitter) &&
           zlang_append_text(emitter, ";\n") && zlang_append_indent(emitter, depth + 1U) &&
           zlang_append_text(emitter, "if (") && zlang_append_text(emitter, _index_name) &&
           zlang_append_text(emitter, " < ") && zlang_append_text(emitter, _generated_k) &&
           zlang_append_text(emitter, ".length_) {\n") &&
           zlang_append_indent(emitter, depth + 2U) && zlang_append_text(emitter, _generated_k) &&
           zlang_append_text(emitter, ".data_[") && zlang_append_text(emitter, _index_name) &&
           zlang_append_text(emitter, "] = ") &&
           zlang_emit_expression(program, expression->right_, emitter) &&
           zlang_append_text(emitter, ";\n") && zlang_append_indent(emitter, depth + 1U) &&
           zlang_append_text(emitter, "} else {\n") && zlang_append_indent(emitter, depth + 2U) &&
           zlang_append_text(emitter, "zlang_trap(2U);\n") &&
           zlang_append_indent(emitter, depth + 1U) && zlang_append_text(emitter, "}\n") &&
           zlang_append_indent(emitter, depth) && zlang_append_text(emitter, "}\n");
}

static bool zlang_emit_body(const zlang_program_t *program, size_t start, size_t count,
                            zlang_emitter_t *emitter, uint32_t depth);

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_statement(const zlang_program_t *program,
                                 const zlang_ast_statement_t *statement, zlang_emitter_t *emitter,
                                 uint32_t depth);

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_body(const zlang_program_t *program, size_t start, size_t count,
                            zlang_emitter_t *emitter, uint32_t depth) { // NOLINT(misc-no-recursion)
    for (size_t _index = 0U; _index < count; ++_index) {
        const size_t _statement_index_k = start + _index;
        if (_statement_index_k >= program->statement_count_ ||
            !zlang_emit_statement(program, &program->statements_[_statement_index_k], emitter,
                                  depth)) {
            return false;
        }
    }
    return true;
}

static bool zlang_emit_string_storage(const zlang_program_t *program,
                                      const zlang_ast_statement_t *statement,
                                      const zlang_emitted_symbol_t *symbol,
                                      zlang_emitter_t *emitter, uint32_t depth, bool is_static) {
    if (statement->capacity_ == UINT32_MAX) {
        return false;
    }
    if ((is_static && !zlang_append_text(emitter, "static ")) ||
        !zlang_append_text(emitter, "char ") ||
        !zlang_append_text(emitter, symbol->storage_name_) || !zlang_append_text(emitter, "[") ||
        !zlang_append_unsigned(emitter, statement->capacity_ + 1U) ||
        !zlang_append_text(emitter, "] = { 0 };\n")) {
        return false;
    }

    const bool _has_initializer_k = statement->expression_ != ZLANG_NO_EXPRESSION;
    const zlang_ast_expression_t *const _initializer_k =
        _has_initializer_k ? &program->expressions_[statement->expression_] : NULL;
    const bool _is_concat_k =
        _initializer_k != NULL && _initializer_k->kind_ == zlang_expression_string_concat_;
    const bool _all_literals_k =
        _is_concat_k && zlang_is_literal_string(program, statement->expression_);

    if (!zlang_append_indent(emitter, depth) ||
        (is_static && !zlang_append_text(emitter, "static ")) ||
        !zlang_append_text(emitter, "zlang_string_t ") ||
        !zlang_append_text(emitter, symbol->generated_name_) ||
        !zlang_append_text(emitter, " = ")) {
        return false;
    }
    if (is_static && _all_literals_k) {
        uint32_t _length_k = 0U;
        if (!zlang_folded_string_length(program, statement->expression_, &_length_k) ||
            !zlang_emit_folded_string(program, statement->expression_, emitter)) {
            return false;
        }
    } else if (is_static || !_has_initializer_k) {
        if (!zlang_append_text(emitter, "(zlang_string_t){ ") ||
            !zlang_append_text(emitter, symbol->storage_name_) ||
            !zlang_append_text(emitter, ", 0U }")) {
            return false;
        }
    } else if (_is_concat_k && !zlang_is_literal_string(program, statement->expression_)) {
        if (!zlang_emit_string_concat_call(program, statement->expression_, symbol->storage_name_,
                                           statement->capacity_, emitter)) {
            return false;
        }
    } else if (!zlang_emit_string_concat_call(program, statement->expression_,
                                              symbol->storage_name_, statement->capacity_,
                                              emitter)) {
        return false;
    }
    return zlang_append_text(emitter, ";\n");
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_statement(const zlang_program_t *program,
                                 const zlang_ast_statement_t *statement, zlang_emitter_t *emitter,
                                 uint32_t depth) { // NOLINT(misc-no-recursion)
    if (!zlang_append_indent(emitter, depth)) {
        return false;
    }
    switch (statement->kind_) {
    case zlang_statement_declaration_: {
        const bool _is_string_k = strcmp(statement->type_name_, "string") == 0;
        const bool _is_concat_k =
            statement->expression_ != ZLANG_NO_EXPRESSION &&
            statement->expression_ < program->expression_count_ &&
            program->expressions_[statement->expression_].kind_ == zlang_expression_string_concat_;
        char _name[ZLANG_EMITTED_NAME_CAPACITY];
        const bool _needs_storage_k = _is_string_k && statement->has_explicit_capacity_;
        if (!zlang_register_symbol(emitter, statement->name_, statement->type_name_,
                                   statement->capacity_, statement->has_explicit_capacity_,
                                   _needs_storage_k, _name, sizeof(_name))) {
            return false;
        }
        const zlang_emitted_symbol_t *const _symbol_k =
            zlang_find_active_symbol(emitter, statement->name_);
        if (_symbol_k == NULL) {
            return false;
        }
        if (_needs_storage_k) {
            return zlang_emit_string_storage(program, statement, _symbol_k, emitter, depth,
                                             zlang_is_static(emitter, statement));
        }
        if (zlang_is_static(emitter, statement) && !zlang_append_text(emitter, "static ")) {
            return false;
        }
        if (!zlang_append_text(emitter, zlang_c_type(statement->type_name_)) ||
            !zlang_append_text(emitter, " ") || !zlang_append_text(emitter, _name) ||
            !zlang_append_text(emitter, " = ")) {
            return false;
        }
        if (_is_concat_k && zlang_is_literal_string(program, statement->expression_)) {
            if (!zlang_emit_folded_string(program, statement->expression_, emitter)) {
                return false;
            }
        } else if (statement->expression_ != ZLANG_NO_EXPRESSION) {
            if (!zlang_emit_expression(program, statement->expression_, emitter)) {
                return false;
            }
        } else if (!zlang_append_text(emitter, zlang_default_initializer(statement->type_name_))) {
            return false;
        }
        if (!zlang_append_text(emitter, ";\n") || !zlang_append_indent(emitter, depth) ||
            !zlang_append_text(emitter, "(void)") || !zlang_append_text(emitter, _name) ||
            !zlang_append_text(emitter, ";\n")) {
            return false;
        }
        return true;
    }
    case zlang_statement_sequence_declaration_: {
        char _name[ZLANG_EMITTED_NAME_CAPACITY];
        if (statement->capacity_ == 0U ||
            !zlang_register_symbol(emitter, statement->name_, statement->type_name_,
                                   statement->capacity_, statement->has_explicit_capacity_, false,
                                   _name, sizeof(_name))) {
            return false;
        }
        char _type[64];
        if (!zlang_type_name(statement, _type, sizeof(_type))) {
            return false;
        }
        const bool _needs_type_k = !zlang_type_was_emitted(emitter, _type);
        if (_needs_type_k &&
            (!zlang_mark_type_emitted(emitter, _type) || !zlang_append_text(emitter, "struct ") ||
             !zlang_append_text(emitter, _type) || !zlang_append_text(emitter, " {\n    ") ||
             !zlang_append_text(emitter, zlang_c_type(statement->type_name_)) ||
             !zlang_append_text(emitter, " data_[") ||
             !zlang_append_unsigned(emitter, statement->capacity_) ||
             !zlang_append_text(emitter, "];\n    uint32_t length_;\n};\n"))) {
            return false;
        }
        if (!zlang_append_indent(emitter, depth)) {
            return false;
        }
        if (zlang_is_static(emitter, statement) && !zlang_append_text(emitter, "static ")) {
            return false;
        }
        if (!zlang_append_text(emitter, "struct ") || !zlang_append_text(emitter, _type) ||
            !zlang_append_text(emitter, " ") || !zlang_append_text(emitter, _name) ||
            !zlang_append_text(emitter, " = { { ")) {
            return false;
        }
        if (!zlang_emit_sequence_values(program, statement, emitter) ||
            !zlang_append_text(emitter, " }, ") ||
            !zlang_append_unsigned(emitter, (uint32_t)statement->initializer_count_) ||
            !zlang_append_text(emitter, "U };\n") || !zlang_append_indent(emitter, depth) ||
            !zlang_append_text(emitter, "(void)") || !zlang_append_text(emitter, _name) ||
            !zlang_append_text(emitter, ";\n")) {
            return false;
        }
        return true;
    }
    case zlang_statement_expression_: {
        const zlang_ast_expression_t *const _expression_k =
            statement->expression_ < program->expression_count_
                ? &program->expressions_[statement->expression_]
                : NULL;
        if (_expression_k != NULL && _expression_k->kind_ == zlang_expression_index_assignment_) {
            return zlang_emit_index_assignment(program, _expression_k, emitter, depth);
        }
        if (!zlang_emit_expression(program, statement->expression_, emitter)) {
            return false;
        }
        return zlang_append_text(emitter, ";\n");
    }
    case zlang_statement_return_:
        if (statement->expression_ == ZLANG_NO_EXPRESSION) {
            return zlang_append_text(emitter, "return;\n");
        }
        if (!zlang_append_text(emitter, "return ") ||
            !zlang_emit_expression(program, statement->expression_, emitter)) {
            return false;
        }
        return zlang_append_text(emitter, ";\n");
    case zlang_statement_if_: {
        if (!zlang_append_text(emitter, "if (") ||
            !zlang_emit_expression(program, statement->expression_, emitter) ||
            !zlang_append_text(emitter, ") {\n")) {
            return false;
        }
        const size_t _if_scope_k = zlang_save_scope(emitter);
        if (!zlang_emit_body(program, statement->body_start_, statement->body_count_, emitter,
                             depth + 1U) ||
            !zlang_append_indent(emitter, depth) || !zlang_append_text(emitter, "}")) {
            return false;
        }
        zlang_restore_scope(emitter, _if_scope_k);
        if (statement->else_count_ == 0U) {
            return zlang_append_text(emitter, "\n");
        }
        const size_t _else_scope_k = zlang_save_scope(emitter);
        if (!zlang_append_text(emitter, " else {\n") ||
            !zlang_emit_body(program, statement->else_start_, statement->else_count_, emitter,
                             depth + 1U) ||
            !zlang_append_indent(emitter, depth) || !zlang_append_text(emitter, "}\n")) {
            return false;
        }
        zlang_restore_scope(emitter, _else_scope_k);
        return true;
    }
    case zlang_statement_while_: {
        if (!zlang_append_text(emitter, "while (") ||
            !zlang_emit_expression(program, statement->expression_, emitter) ||
            !zlang_append_text(emitter, ") {\n")) {
            return false;
        }
        const size_t _while_scope_k = zlang_save_scope(emitter);
        const bool _result_k = zlang_emit_body(program, statement->body_start_,
                                               statement->body_count_, emitter, depth + 1U) &&
                               zlang_append_indent(emitter, depth) &&
                               zlang_append_text(emitter, "}\n");
        zlang_restore_scope(emitter, _while_scope_k);
        return _result_k;
    }
    case zlang_statement_repeat_: {
        if (!zlang_append_text(emitter, "for (uint32_t _repeat = 0U; _repeat < ") ||
            !zlang_emit_expression(program, statement->expression_, emitter) ||
            !zlang_append_text(emitter, "; ++_repeat) {\n")) {
            return false;
        }
        const size_t _repeat_scope_k = zlang_save_scope(emitter);
        const bool _result_k = zlang_emit_body(program, statement->body_start_,
                                               statement->body_count_, emitter, depth + 1U) &&
                               zlang_append_indent(emitter, depth) &&
                               zlang_append_text(emitter, "}\n");
        zlang_restore_scope(emitter, _repeat_scope_k);
        return _result_k;
    }
    case zlang_statement_foreach_: {
        char _item_name[ZLANG_EMITTED_NAME_CAPACITY];
        const char *const _iterable_k = zlang_resolve_name(emitter, statement->iterable_name_);
        if (!zlang_register_symbol(emitter, statement->name_, statement->type_name_, 0U, false,
                                   false, _item_name, sizeof(_item_name))) {
            return false;
        }
        if (!zlang_append_text(emitter, "for (uint32_t index = 0U; index < ") ||
            !zlang_append_text(emitter, _iterable_k) ||
            !zlang_append_text(emitter, ".length_; ++index) {\n") ||
            !zlang_append_indent(emitter, depth + 1U) ||
            !zlang_append_text(emitter, zlang_c_type(statement->type_name_)) ||
            !zlang_append_text(emitter, " ") || !zlang_append_text(emitter, _item_name) ||
            !zlang_append_text(emitter, " = ") || !zlang_append_text(emitter, _iterable_k) ||
            !zlang_append_text(emitter, ".data_[index];\n") ||
            !zlang_append_indent(emitter, depth + 1U) || !zlang_append_text(emitter, "(void)") ||
            !zlang_append_text(emitter, _item_name) || !zlang_append_text(emitter, ";\n")) {
            return false;
        }
        const size_t _foreach_scope_k = zlang_save_scope(emitter);
        const bool _result_k = zlang_emit_body(program, statement->body_start_,
                                               statement->body_count_, emitter, depth + 1U) &&
                               zlang_append_indent(emitter, depth) &&
                               zlang_append_text(emitter, "}\n");
        zlang_restore_scope(emitter, _foreach_scope_k);
        return _result_k;
    }
    case zlang_statement_function_: {
        char _name[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_register_symbol(emitter, statement->name_, statement->type_name_, 0U, false,
                                   false, _name, sizeof(_name))) {
            return false;
        }
        const size_t _function_scope_k = zlang_save_scope(emitter);
        if (!zlang_append_text(emitter, zlang_c_type(statement->type_name_)) ||
            !zlang_append_text(emitter, " ") || !zlang_append_text(emitter, _name) ||
            !zlang_append_text(emitter, "(")) {
            return false;
        }
        for (size_t _index = 0U; _index < statement->parameter_count_; ++_index) {
            const zlang_parameter_t *const _parameter_k =
                &program->parameters_[statement->parameter_start_ + _index];
            char _parameter_name[ZLANG_EMITTED_NAME_CAPACITY];
            if (!zlang_register_symbol(emitter, _parameter_k->name_, _parameter_k->type_name_, 0U,
                                       false, false, _parameter_name, sizeof(_parameter_name))) {
                return false;
            }
            if ((_index > 0U && !zlang_append_text(emitter, ", ")) ||
                !zlang_append_text(emitter, zlang_c_type(_parameter_k->type_name_)) ||
                !zlang_append_text(emitter, " ") || !zlang_append_text(emitter, _parameter_name)) {
                return false;
            }
        }
        const bool _result_k = zlang_append_text(emitter, ") {\n") &&
                               zlang_emit_body(program, statement->body_start_,
                                               statement->body_count_, emitter, depth + 1U) &&
                               zlang_append_indent(emitter, depth) &&
                               zlang_append_text(emitter, "}\n\n");
        zlang_restore_scope(emitter, _function_scope_k);
        return _result_k;
    }
    case zlang_statement_block_: {
        const size_t _block_scope_k = zlang_save_scope(emitter);
        const bool _result_k = zlang_append_text(emitter, "{\n") &&
                               zlang_emit_body(program, statement->body_start_,
                                               statement->body_count_, emitter, depth + 1U) &&
                               zlang_append_indent(emitter, depth) &&
                               zlang_append_text(emitter, "}\n");
        zlang_restore_scope(emitter, _block_scope_k);
        return _result_k;
    }
    default:
        return false;
    }
}

static bool zlang_emit_prologue(zlang_emitter_t *emitter) {
    return zlang_append_text(emitter, "#include <stdbool.h>\n#include <stdint.h>\n\n"
                                      "#include <zlang/runtime/runtime.h>\n\n");
}

static bool zlang_is_nested(const zlang_program_t *program, size_t index) {
    for (size_t _owner = 0U; _owner < program->statement_count_; ++_owner) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_owner];
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

static bool zlang_is_file_scope(const zlang_emitter_t *emitter,
                                const zlang_ast_statement_t *statement) {
    return statement->kind_ == zlang_statement_function_ ||
           ((statement->kind_ == zlang_statement_declaration_ ||
             statement->kind_ == zlang_statement_sequence_declaration_) &&
            zlang_is_static(emitter, statement));
}

bool zlang_emit_program(const zlang_program_t *program, zlang_emitter_t *emitter) {
    if (emitter->plan_ == NULL || !zlang_emit_prologue(emitter)) {
        return false;
    }

    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (zlang_is_nested(program, _index)) {
            continue;
        }
        if (zlang_is_file_scope(emitter, _statement_k) &&
            !zlang_emit_statement(program, _statement_k, emitter, 0U)) {
            return false;
        }
    }

    if (!zlang_append_text(emitter, "int main(void) {\n")) {
        return false;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (zlang_is_nested(program, _index) || zlang_is_file_scope(emitter, _statement_k)) {
            continue;
        }
        if (!zlang_emit_statement(program, _statement_k, emitter, 1U)) {
            return false;
        }
    }
    return zlang_append_text(emitter, "    return 0;\n}\n");
}
