/** @file
 * @brief C99 emitter for the Zlang abstract syntax tree.
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
    emitter->emitted_symbol_count_ = 0U;
    emitter->active_symbol_count_ = 0U;
    emitter->emitted_sequence_count_ = 0U;
    emitter->emitted_literal_count_ = 0U;
    emitter->emitted_capacity_count_ = 0U;
    emitter->emitted_user_type_count_ = 0U;
    emitter->emitted_enum_item_count_ = 0U;
    emitter->cse_count_ = 0U;
    emitter->cse_pointer_count_ = 0U;
    emitter->emit_comments_ = true;
    memset(emitter->emitted_comments_, 0, sizeof(emitter->emitted_comments_));
    memset(emitter->emitted_symbols_, 0, sizeof(emitter->emitted_symbols_));
    memset(emitter->emitted_sequences_, 0, sizeof(emitter->emitted_sequences_));
    memset(emitter->emitted_literals_, 0, sizeof(emitter->emitted_literals_));
    memset(emitter->emitted_capacities_, 0, sizeof(emitter->emitted_capacities_));
    memset(emitter->emitted_user_types_, 0, sizeof(emitter->emitted_user_types_));
    memset(emitter->emitted_enum_items_, 0, sizeof(emitter->emitted_enum_items_));
    memset(emitter->cse_expression_indices_, 0, sizeof(emitter->cse_expression_indices_));
    memset(emitter->cse_names_, 0, sizeof(emitter->cse_names_));
    memset(emitter->cse_pointer_indices_, 0, sizeof(emitter->cse_pointer_indices_));
    memset(emitter->cse_pointer_names_, 0, sizeof(emitter->cse_pointer_names_));
    for (size_t _index = 0U; _index < ZLANG_EMITTED_CSE_CAPACITY; ++_index) {
        emitter->cse_pointer_base_names_[_index] = NULL;
        emitter->cse_pointer_dependency_names_[_index] = NULL;
    }
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

static bool zlang_append_part_count(zlang_emitter_t *emitter, size_t count);

static const char *zlang_find_cse(const zlang_emitter_t *emitter, size_t expression_index);

static const char *zlang_find_pointer_cse(const zlang_emitter_t *emitter, size_t expression_index);

static bool zlang_materialize_field_references(const zlang_program_t *program,
                                               const zlang_ast_statement_t *statement,
                                               zlang_emitter_t *emitter, uint32_t depth);

static void zlang_invalidate_pointer_cse(zlang_emitter_t *emitter, const char *name);

static bool zlang_expression_equal(const zlang_program_t *program, size_t left, size_t right);

static bool zlang_same_text(const char *left, const char *right);

// NOLINTNEXTLINE(misc-no-recursion)
static const char *zlang_expression_type(const zlang_program_t *program,
                                         const zlang_emitter_t *emitter, size_t index);

static bool zlang_append_unsigned(zlang_emitter_t *emitter, uint32_t value) {
    char _digits[16];
    const int _written_k = snprintf(_digits, sizeof(_digits), "%lu", (unsigned long)value);
    return _written_k >= 0 && (size_t)_written_k < sizeof(_digits) &&
           zlang_append_text(emitter, _digits);
}

static bool zlang_append_line_break(zlang_emitter_t *emitter) {
    while (emitter->length_ > 0U && (emitter->buffer_[emitter->length_ - 1U] == ' ' ||
                                     emitter->buffer_[emitter->length_ - 1U] == '\t')) {
        --emitter->length_;
        emitter->buffer_[emitter->length_] = '\0';
    }
    return zlang_append_text(emitter, "\n");
}

static bool zlang_append_indent(zlang_emitter_t *emitter, uint32_t depth);

static bool zlang_append_exact(zlang_emitter_t *emitter, const char *text, size_t length) {
    if (emitter->length_ >= emitter->capacity_ ||
        length > emitter->capacity_ - emitter->length_ - 1U) {
        return false;
    }
    memcpy(emitter->buffer_ + emitter->length_, text, length);
    emitter->length_ += length;
    emitter->buffer_[emitter->length_] = '\0';
    return true;
}

static uint32_t zlang_comment_target_line(const zlang_program_t *program,
                                          const zlang_comment_t *comment) {
    uint32_t _target_k = 0U;
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const uint32_t _line_k = program->statements_[_index].line_;
        if (_line_k > comment->end_line_ && (_target_k == 0U || _line_k < _target_k)) {
            _target_k = _line_k;
        }
    }
    for (size_t _index = 0U; _index < program->expression_count_; ++_index) {
        const zlang_ast_expression_t *const _expression_k = &program->expressions_[_index];
        if (_expression_k->kind_ != zlang_expression_enum_item_) {
            continue;
        }
        const uint32_t _line_k = _expression_k->line_;
        if (_line_k > comment->end_line_ && (_target_k == 0U || _line_k < _target_k)) {
            _target_k = _line_k;
        }
    }
    return _target_k;
}

static bool zlang_comment_is_attached(const zlang_program_t *program,
                                      const zlang_comment_t *comment) {
    return zlang_comment_target_line(program, comment) != 0U;
}

static bool zlang_emit_comment(zlang_emitter_t *emitter, const zlang_comment_t *comment,
                               uint32_t depth) {
    return zlang_append_indent(emitter, depth) &&
           zlang_append_exact(emitter, comment->text_, comment->length_) &&
           zlang_append_text(emitter, "\n");
}

static bool zlang_emit_attached_comments(const zlang_program_t *program, zlang_emitter_t *emitter,
                                         uint32_t depth, uint32_t target_line) {
    if (!emitter->emit_comments_ || program == NULL || program->comments_ == NULL) {
        return true;
    }
    for (size_t _index = 0U; _index < program->comment_count_; ++_index) {
        if (_index >= ZLANG_EMITTED_COMMENT_CAPACITY || emitter->emitted_comments_[_index]) {
            continue;
        }
        const zlang_comment_t *const _comment_k = &program->comments_[_index];
        if (zlang_comment_target_line(program, _comment_k) != target_line) {
            continue;
        }
        if (!zlang_emit_comment(emitter, _comment_k, depth)) {
            return false;
        }
        emitter->emitted_comments_[_index] = true;
    }
    return true;
}

static bool zlang_emit_remaining_comments(const zlang_program_t *program, zlang_emitter_t *emitter,
                                          uint32_t depth) {
    if (!emitter->emit_comments_ || program == NULL || program->comments_ == NULL) {
        return true;
    }
    for (size_t _index = 0U; _index < program->comment_count_; ++_index) {
        if (_index >= ZLANG_EMITTED_COMMENT_CAPACITY || emitter->emitted_comments_[_index]) {
            continue;
        }
        const zlang_comment_t *const _comment_k = &program->comments_[_index];
        if (zlang_comment_is_attached(program, _comment_k)) {
            continue;
        }
        if (!zlang_emit_comment(emitter, _comment_k, depth)) {
            return false;
        }
        emitter->emitted_comments_[_index] = true;
    }
    return true;
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

static const char *zlang_default_initializer(const char *type_name) {
    if (strcmp(type_name, "integer") == 0) {
        return "0";
    }
    if (strcmp(type_name, "unsigned") == 0) {
        return "ZLANG_ZERO";
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
                                  const char *type_name, const char *prefix, uint32_t capacity,
                                  bool has_explicit_capacity, bool needs_storage, bool is_sequence,
                                  bool is_reference, char *generated, size_t generated_capacity) {
    char _user_name[ZLANG_EMITTED_NAME_CAPACITY];
    const int _user_written_k =
        source_name != NULL && prefix != NULL
            ? snprintf(_user_name, sizeof(_user_name), "%s%s", prefix, source_name)
            : -1;
    if (emitter->emitted_symbol_count_ >= ZLANG_EMITTED_SYMBOL_CAPACITY ||
        emitter->active_symbol_count_ >= ZLANG_EMITTED_SYMBOL_CAPACITY || _user_written_k < 0 ||
        (size_t)_user_written_k >= sizeof(_user_name) ||
        !zlang_make_unique_name(emitter, _user_name, generated, generated_capacity)) {
        return false;
    }
    zlang_emitted_symbol_t *const _symbol_k =
        &emitter->emitted_symbols_[emitter->active_symbol_count_];
    memset(_symbol_k, 0, sizeof(*_symbol_k));
    _symbol_k->source_name_ = source_name;
    _symbol_k->type_name_ = type_name;
    _symbol_k->capacity_ = capacity;
    _symbol_k->has_explicit_capacity_ = has_explicit_capacity;
    _symbol_k->is_sequence_ = is_sequence;
    _symbol_k->is_reference_ = is_reference;
    (void)snprintf(_symbol_k->generated_name_, sizeof(_symbol_k->generated_name_), "%s", generated);
    if (needs_storage) {
        const int _storage_written_k =
            snprintf(_symbol_k->storage_name_, sizeof(_symbol_k->storage_name_),
                     "_zlang_%s_storage_%lu", source_name, (unsigned long)emitter->next_index_++);
        if (_storage_written_k < 0 ||
            (size_t)_storage_written_k >= sizeof(_symbol_k->storage_name_)) {
            return false;
        }
    }
    ++emitter->active_symbol_count_;
    if (emitter->active_symbol_count_ > emitter->emitted_symbol_count_) {
        emitter->emitted_symbol_count_ = emitter->active_symbol_count_;
    }
    return true;
}

static bool zlang_append_field_name(zlang_emitter_t *emitter, const char *name) {
    if (name == NULL || name[0] == '\0') {
        return false;
    }
    return zlang_append_text(emitter, name) &&
           (name[strlen(name) - 1U] == '_' || zlang_append_text(emitter, "_"));
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

static bool zlang_emit_symbol_value(zlang_emitter_t *emitter, const char *source_name) {
    const zlang_emitted_symbol_t *const _symbol_k = zlang_find_active_symbol(emitter, source_name);
    if (_symbol_k == NULL) {
        return false;
    }
    if (_symbol_k->is_reference_) {
        return zlang_append_text(emitter, "(*") &&
               zlang_append_text(emitter, _symbol_k->generated_name_) &&
               zlang_append_text(emitter, ")");
    }
    return zlang_append_text(emitter, _symbol_k->generated_name_);
}

static bool zlang_emit_reference_argument(const zlang_program_t *program, size_t index,
                                          zlang_emitter_t *emitter);

static const zlang_emitted_sequence_t *
zlang_find_sequence_entry(const zlang_program_t *program, const zlang_emitter_t *emitter,
                          const zlang_ast_statement_t *statement) {
    if (statement == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < emitter->emitted_sequence_count_; ++_index) {
        const zlang_emitted_sequence_t *const _entry_k = &emitter->emitted_sequences_[_index];
        if (_entry_k->statement_ == statement ||
            (_entry_k->statement_->name_ != NULL && statement->name_ != NULL &&
             strcmp(_entry_k->statement_->name_, statement->name_) == 0 &&
             _entry_k->statement_->capacity_ == statement->capacity_ &&
             _entry_k->statement_->element_capacity_ == statement->element_capacity_)) {
            return _entry_k;
        }
    }
    (void)program;
    return NULL;
}

static bool zlang_upper_name(const char *source, char *output, size_t capacity) {
    if (source == NULL || source[0] == '\0' || strlen(source) + 1U >= capacity) {
        return false;
    }
    for (size_t _index = 0U; source[_index] != '\0'; ++_index) {
        const int _code_k = (unsigned char)source[_index];
        output[_index] = (char)(_code_k >= 'a' && _code_k <= 'z' ? _code_k - 'a' + 'A' : _code_k);
    }
    output[strlen(source)] = '\0';
    return true;
}

static bool zlang_lower_name(const char *source, char *output, size_t capacity) {
    if (source == NULL || source[0] == '\0' || strlen(source) + 1U >= capacity) {
        return false;
    }
    for (size_t _index = 0U; source[_index] != '\0'; ++_index) {
        const int _code_k = (unsigned char)source[_index];
        output[_index] = (char)(_code_k >= 'A' && _code_k <= 'Z' ? _code_k - 'A' + 'a' : _code_k);
    }
    output[strlen(source)] = '\0';
    return true;
}

static bool zlang_assemble_name(const char *prefix, const char *middle, const char *suffix,
                                char *output, size_t capacity) {
    if (prefix == NULL || middle == NULL || suffix == NULL) {
        return false;
    }
    const size_t _prefix_length_k = strlen(prefix);
    const size_t _middle_length_k = strlen(middle);
    const size_t _suffix_length_k = strlen(suffix);
    if (_prefix_length_k + _middle_length_k + _suffix_length_k + 1U >= capacity) {
        return false;
    }
    memcpy(output, prefix, _prefix_length_k + 1U);
    memcpy(output + _prefix_length_k, middle, _middle_length_k + 1U);
    memcpy(output + _prefix_length_k + _middle_length_k, suffix, _suffix_length_k + 1U);
    return true;
}

static bool zlang_generated_name_used(const zlang_emitter_t *emitter, const char *name) {
    if (zlang_name_is_used(emitter, name)) {
        return true;
    }
    for (size_t _index = 0U; _index < emitter->emitted_sequence_count_; ++_index) {
        const zlang_emitted_sequence_t *const _entry_k = &emitter->emitted_sequences_[_index];
        if (strcmp(_entry_k->type_name_, name) == 0 ||
            strcmp(_entry_k->capacity_macro_, name) == 0 ||
            strcmp(_entry_k->element_macro_, name) == 0) {
            return true;
        }
    }
    for (size_t _index = 0U; _index < emitter->emitted_capacity_count_; ++_index) {
        if (strcmp(emitter->emitted_capacities_[_index].macro_, name) == 0) {
            return true;
        }
    }
    for (size_t _index = 0U; _index < emitter->emitted_literal_count_; ++_index) {
        const zlang_emitted_literal_t *const _entry_k = &emitter->emitted_literals_[_index];
        if (strcmp(_entry_k->data_name_, name) == 0 || strcmp(_entry_k->view_name_, name) == 0) {
            return true;
        }
    }
    return false;
}

static bool zlang_make_sequence_names(zlang_emitter_t *emitter, const char *source,
                                      bool has_element_capacity, char *type_name,
                                      char *capacity_macro, char *element_macro, char *length_macro,
                                      size_t capacity) {
    char _lower_base[ZLANG_EMITTED_NAME_CAPACITY];
    char _upper_base[ZLANG_EMITTED_NAME_CAPACITY];
    if (!zlang_copy_name(source, _lower_base, sizeof(_lower_base)) ||
        !zlang_upper_name(source, _upper_base, sizeof(_upper_base))) {
        return false;
    }
    char _suffix[16];
    for (uint32_t _attempt = 0U;; ++_attempt) {
        if (!zlang_assemble_name("zlang_", _lower_base, "_t", type_name, capacity) ||
            !zlang_assemble_name("ZLANG_", _upper_base, "_CAPACITY", capacity_macro, capacity) ||
            !zlang_assemble_name("ZLANG_", _upper_base, "_ELEMENT_CAPACITY", element_macro,
                                 capacity) ||
            !zlang_assemble_name("ZLANG_", _upper_base, "_LENGTH", length_macro, capacity)) {
            return false;
        }
        if (!zlang_generated_name_used(emitter, type_name) &&
            !zlang_generated_name_used(emitter, capacity_macro) &&
            !zlang_generated_name_used(emitter, length_macro) &&
            (!has_element_capacity || !zlang_generated_name_used(emitter, element_macro))) {
            return true;
        }
        if (_attempt == UINT32_MAX) {
            return false;
        }
        const uint32_t _suffix_value_k = emitter->next_index_++;
        const int _written_k = snprintf(_suffix, sizeof(_suffix), "%u", _suffix_value_k);
        if (_written_k < 0 || (size_t)_written_k >= sizeof(_suffix) ||
            !zlang_append_name_suffix(source, _suffix_value_k, _lower_base, sizeof(_lower_base)) ||
            !zlang_append_name_suffix(_upper_base, _suffix_value_k, _upper_base,
                                      sizeof(_upper_base))) {
            return false;
        }
    }
}

static bool zlang_collect_sequences(zlang_emitter_t *emitter, const zlang_program_t *program) {
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ != zlang_statement_sequence_declaration_ ||
            zlang_find_sequence_entry(program, emitter, _statement_k) != NULL) {
            continue;
        }
        if (emitter->emitted_sequence_count_ >= ZLANG_EMITTED_SEQUENCE_CAPACITY) {
            return false;
        }
        zlang_emitted_sequence_t *const _entry_k =
            &emitter->emitted_sequences_[emitter->emitted_sequence_count_];
        memset(_entry_k, 0, sizeof(*_entry_k));
        _entry_k->statement_ = _statement_k;
        if (!zlang_make_sequence_names(emitter, _statement_k->name_,
                                       _statement_k->has_element_capacity_, _entry_k->type_name_,
                                       _entry_k->capacity_macro_, _entry_k->element_macro_,
                                       _entry_k->length_macro_, sizeof(_entry_k->type_name_))) {
            return false;
        }
        ++emitter->emitted_sequence_count_;
    }
    return true;
}

static const zlang_emitted_capacity_t *zlang_find_capacity(const zlang_emitter_t *emitter,
                                                           const zlang_ast_statement_t *statement) {
    if (statement == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < emitter->emitted_capacity_count_; ++_index) {
        if (emitter->emitted_capacities_[_index].statement_ == statement) {
            return &emitter->emitted_capacities_[_index];
        }
    }
    return NULL;
}

static bool zlang_collect_capacities(zlang_emitter_t *emitter, const zlang_program_t *program) {
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ != zlang_statement_declaration_ ||
            strcmp(_statement_k->type_name_, "string") != 0 ||
            !_statement_k->has_explicit_capacity_ ||
            zlang_find_capacity(emitter, _statement_k) != NULL) {
            continue;
        }
        if (emitter->emitted_capacity_count_ >= ZLANG_EMITTED_SEQUENCE_CAPACITY) {
            return false;
        }
        char _upper[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_upper_name(_statement_k->name_, _upper, sizeof(_upper))) {
            return false;
        }
        zlang_emitted_capacity_t *const _entry_k =
            &emitter->emitted_capacities_[emitter->emitted_capacity_count_];
        for (uint32_t _attempt = 0U;; ++_attempt) {
            if (!zlang_assemble_name("ZLANG_", _upper, "_CAPACITY", _entry_k->macro_,
                                     sizeof(_entry_k->macro_))) {
                return false;
            }
            if (!zlang_generated_name_used(emitter, _entry_k->macro_)) {
                break;
            }
            if (_attempt == UINT32_MAX) {
                return false;
            }
            const unsigned long _suffix_k = emitter->next_index_++;
            if (!zlang_append_name_suffix(_upper, (uint32_t)_suffix_k, _upper, sizeof(_upper))) {
                return false;
            }
        }
        _entry_k->statement_ = _statement_k;
        ++emitter->emitted_capacity_count_;
    }
    return true;
}

static const zlang_emitted_capacity_t *
zlang_find_capacity_for_symbol(const zlang_emitter_t *emitter,
                               const zlang_emitted_symbol_t *symbol) {
    if (symbol == NULL || symbol->source_name_ == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < emitter->emitted_capacity_count_; ++_index) {
        const zlang_emitted_capacity_t *const _entry_k = &emitter->emitted_capacities_[_index];
        if (strcmp(_entry_k->statement_->name_, symbol->source_name_) == 0 &&
            _entry_k->statement_->capacity_ == symbol->capacity_) {
            return _entry_k;
        }
    }
    return NULL;
}

static bool zlang_capacity_text(const zlang_emitter_t *emitter,
                                const zlang_ast_statement_t *statement, uint32_t value,
                                char *output, size_t capacity) {
    const zlang_emitted_capacity_t *const _entry_k = zlang_find_capacity(emitter, statement);
    if (_entry_k != NULL) {
        return zlang_copy_name(_entry_k->macro_, output, capacity);
    }
    const int _written_k = snprintf(output, capacity, "%luU", (unsigned long)value);
    return _written_k >= 0 && (size_t)_written_k < capacity;
}

static bool zlang_symbol_capacity_text(const zlang_emitter_t *emitter,
                                       const zlang_emitted_symbol_t *symbol, char *output,
                                       size_t capacity) {
    const zlang_emitted_capacity_t *const _entry_k =
        zlang_find_capacity_for_symbol(emitter, symbol);
    if (_entry_k != NULL) {
        return zlang_copy_name(_entry_k->macro_, output, capacity);
    }
    const int _written_k = snprintf(output, capacity, "%luU", (unsigned long)symbol->capacity_);
    return _written_k >= 0 && (size_t)_written_k < capacity;
}

static bool zlang_sequence_capacity_text(const zlang_program_t *program,
                                         const zlang_emitter_t *emitter,
                                         const zlang_ast_statement_t *statement, char *output,
                                         size_t capacity) {
    const zlang_emitted_sequence_t *const _entry_k =
        zlang_find_sequence_entry(program, emitter, statement);
    if (_entry_k == NULL) {
        return false;
    }
    return zlang_copy_name(_entry_k->capacity_macro_, output, capacity);
}

static bool zlang_sequence_element_capacity_text(const zlang_program_t *program,
                                                 const zlang_emitter_t *emitter,
                                                 const zlang_ast_statement_t *statement,
                                                 char *output, size_t capacity) {
    const zlang_emitted_sequence_t *const _entry_k =
        zlang_find_sequence_entry(program, emitter, statement);
    if (_entry_k == NULL || _entry_k->element_macro_[0] == '\0') {
        return false;
    }
    return zlang_copy_name(_entry_k->element_macro_, output, capacity);
}

static const zlang_emitted_user_type_t *zlang_find_user_type(const zlang_emitter_t *emitter,
                                                             const char *name) {
    if (name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < emitter->emitted_user_type_count_; ++_index) {
        const zlang_emitted_user_type_t *const _entry_k = &emitter->emitted_user_types_[_index];
        if (_entry_k->statement_->name_ != NULL && strcmp(_entry_k->statement_->name_, name) == 0) {
            return _entry_k;
        }
    }
    return NULL;
}

static const char *zlang_generated_type(const zlang_program_t *program,
                                        const zlang_emitter_t *emitter, const char *type_name) {
    const zlang_emitted_user_type_t *const _entry_k = zlang_find_user_type(emitter, type_name);
    if (_entry_k != NULL) {
        return _entry_k->type_name_;
    }
    if (type_name != NULL &&
        (strcmp(type_name, "integer") == 0 || strcmp(type_name, "unsigned") == 0 ||
         strcmp(type_name, "boolean") == 0 || strcmp(type_name, "string") == 0 ||
         strcmp(type_name, "void") == 0)) {
        return zlang_c_type(type_name);
    }
    (void)program;
    return NULL;
}

static const char *zlang_generated_enum_item(const zlang_emitter_t *emitter,
                                             const char *source_name) {
    if (source_name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < emitter->emitted_enum_item_count_; ++_index) {
        const zlang_emitted_enum_item_t *const _entry_k = &emitter->emitted_enum_items_[_index];
        if (_entry_k->source_name_ != NULL && strcmp(_entry_k->source_name_, source_name) == 0) {
            return _entry_k->generated_name_;
        }
    }
    return NULL;
}

static bool zlang_collect_user_types(zlang_emitter_t *emitter, const zlang_program_t *program) {
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ != zlang_statement_record_ &&
            _statement_k->kind_ != zlang_statement_enum_) {
            continue;
        }
        if (zlang_find_user_type(emitter, _statement_k->name_) != NULL) {
            continue;
        }
        if (emitter->emitted_user_type_count_ >= ZLANG_EMITTED_USER_TYPE_CAPACITY) {
            return false;
        }
        zlang_emitted_user_type_t *const _entry_k =
            &emitter->emitted_user_types_[emitter->emitted_user_type_count_++];
        memset(_entry_k, 0, sizeof(*_entry_k));
        _entry_k->statement_ = _statement_k;
        char _lower_name[ZLANG_EMITTED_NAME_CAPACITY];
        char _base[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_lower_name(_statement_k->name_, _lower_name, sizeof(_lower_name)) ||
            !zlang_assemble_name("zlang_", _lower_name, "_t", _base, sizeof(_base)) ||
            !zlang_make_unique_name(emitter, _base, _entry_k->type_name_,
                                    sizeof(_entry_k->type_name_))) {
            return false;
        }
        if (_statement_k->kind_ != zlang_statement_enum_) {
            continue;
        }
        char _enum_upper[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_upper_name(_statement_k->name_, _enum_upper, sizeof(_enum_upper))) {
            return false;
        }
        const size_t _end_k = _statement_k->initializer_start_ + _statement_k->initializer_count_;
        for (size_t _item = _statement_k->initializer_start_;
             _item < _end_k && _item < program->expression_count_; ++_item) {
            const zlang_ast_expression_t *const _item_k = &program->expressions_[_item];
            if (_item_k->kind_ != zlang_expression_enum_item_ || _item_k->name_ == NULL) {
                continue;
            }
            if (emitter->emitted_enum_item_count_ >= ZLANG_EMITTED_ENUM_ITEM_CAPACITY) {
                return false;
            }
            char _item_upper[ZLANG_EMITTED_NAME_CAPACITY];
            char _base_name[ZLANG_EMITTED_NAME_CAPACITY];
            if (!zlang_upper_name(_item_k->name_, _item_upper, sizeof(_item_upper)) ||
                !zlang_assemble_name("ZLANG_", _enum_upper, "_", _base_name, sizeof(_base_name))) {
                return false;
            }
            const size_t _base_length_k = strlen(_base_name);
            const size_t _item_length_k = strlen(_item_upper);
            if (_base_length_k + _item_length_k + 1U >= sizeof(_base_name)) {
                return false;
            }
            memcpy(_base_name + _base_length_k, _item_upper, _item_length_k + 1U);
            zlang_emitted_enum_item_t *const _item_entry_k =
                &emitter->emitted_enum_items_[emitter->emitted_enum_item_count_++];
            _item_entry_k->source_name_ = _item_k->name_;
            _item_entry_k->type_name_ = _statement_k->name_;
            if (!zlang_make_unique_name(emitter, _base_name, _item_entry_k->generated_name_,
                                        sizeof(_item_entry_k->generated_name_))) {
                return false;
            }
        }
    }
    return true;
}

static const zlang_ast_statement_t *zlang_find_record_statement(const zlang_program_t *program,
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

static const zlang_ast_statement_t *zlang_find_function_statement(const zlang_program_t *program,
                                                                  const char *name) {
    if (program == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_function_ && _statement_k->name_ != NULL &&
            strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

static const zlang_ast_statement_t *zlang_find_enum_statement(const zlang_program_t *program,
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

static const zlang_ast_statement_t *zlang_find_field_statement(const zlang_program_t *program,
                                                               const zlang_ast_statement_t *record,
                                                               const char *name) {
    if (record == NULL || name == NULL) {
        return NULL;
    }
    const size_t _end_k = record->body_start_ + record->body_count_;
    for (size_t _index = record->body_start_; _index < _end_k && _index < program->statement_count_;
         ++_index) {
        const zlang_ast_statement_t *const _field_k = &program->statements_[_index];
        if (_field_k->name_ != NULL && strcmp(_field_k->name_, name) == 0) {
            return _field_k;
        }
    }
    return NULL;
}

static const char *
// NOLINTNEXTLINE(misc-no-recursion)
zlang_field_record_type(const zlang_program_t *program, const zlang_emitter_t *emitter,
                        const zlang_ast_expression_t *expression) { // NOLINT(misc-no-recursion)
    if (expression->base_name_ != NULL) {
        const zlang_emitted_symbol_t *const _symbol_k =
            zlang_find_active_symbol(emitter, expression->base_name_);
        return _symbol_k != NULL ? _symbol_k->type_name_ : NULL;
    }
    return zlang_expression_type(program, emitter, expression->left_);
}

// NOLINTNEXTLINE(misc-no-recursion)
static const char *zlang_expression_type(const zlang_program_t *program,
                                         const zlang_emitter_t *emitter,
                                         size_t index) { // NOLINT(misc-no-recursion)
    if (program == NULL || index == ZLANG_NO_EXPRESSION || index >= program->expression_count_) {
        return NULL;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    switch (_expression_k->kind_) {
    case zlang_expression_integer_literal_:
        return _expression_k->is_unsigned_ ? "unsigned" : "integer";
    case zlang_expression_boolean_literal_:
        return "boolean";
    case zlang_expression_string_literal_:
    case zlang_expression_string_concat_:
        return "string";
    case zlang_expression_identifier_: {
        const zlang_emitted_symbol_t *const _symbol_k =
            zlang_find_active_symbol(emitter, _expression_k->name_);
        if (_symbol_k != NULL) {
            return _symbol_k->type_name_;
        }
        if (emitter != NULL) {
            for (size_t _item = 0U; _item < emitter->emitted_enum_item_count_; ++_item) {
                const zlang_emitted_enum_item_t *const _entry_k =
                    &emitter->emitted_enum_items_[_item];
                if (_entry_k->source_name_ != NULL && _expression_k->name_ != NULL &&
                    strcmp(_entry_k->source_name_, _expression_k->name_) == 0) {
                    return _entry_k->type_name_;
                }
            }
        }
        return NULL;
    }
    case zlang_expression_enum_item_: {
        for (size_t _item = 0U; _item < emitter->emitted_enum_item_count_; ++_item) {
            const zlang_emitted_enum_item_t *const _entry_k = &emitter->emitted_enum_items_[_item];
            if (_entry_k->source_name_ != NULL && _expression_k->name_ != NULL &&
                strcmp(_entry_k->source_name_, _expression_k->name_) == 0) {
                return _entry_k->type_name_;
            }
        }
        return NULL;
    }
    case zlang_expression_field_:
    case zlang_expression_field_assignment_: {
        const char *const _record_name_k = zlang_field_record_type(program, emitter, _expression_k);
        const zlang_ast_statement_t *const _record_k =
            zlang_find_record_statement(program, _record_name_k);
        const zlang_ast_statement_t *const _field_k =
            zlang_find_field_statement(program, _record_k,
                                       _expression_k->kind_ == zlang_expression_field_
                                           ? _expression_k->name_
                                           : program->expressions_[_expression_k->left_].name_);
        return _field_k != NULL ? _field_k->type_name_ : NULL;
    }
    case zlang_expression_add_:
    case zlang_expression_sub_:
    case zlang_expression_mul_:
    case zlang_expression_div_:
    case zlang_expression_mod_:
    case zlang_expression_assignment_:
    case zlang_expression_compound_assignment_:
        return zlang_expression_type(program, emitter, _expression_k->left_);
    case zlang_expression_negative_:
        return "integer";
    case zlang_expression_length_:
        return "unsigned";
    case zlang_expression_index_: {
        const zlang_emitted_symbol_t *const _symbol_k =
            zlang_find_active_symbol(emitter, _expression_k->name_);
        if (_symbol_k == NULL) {
            return NULL;
        }
        if (!_symbol_k->is_sequence_ && strcmp(_symbol_k->type_name_, "string") == 0) {
            return "unsigned";
        }
        return _symbol_k->type_name_;
    }
    case zlang_expression_equal_:
    case zlang_expression_less_:
    case zlang_expression_greater_:
    case zlang_expression_less_equal_:
    case zlang_expression_greater_equal_:
    case zlang_expression_not_equal_:
    case zlang_expression_not_:
    case zlang_expression_and_:
    case zlang_expression_or_:
        return "boolean";
    case zlang_expression_call_: {
        const zlang_emitted_symbol_t *const _symbol_k =
            zlang_find_active_symbol(emitter, _expression_k->name_);
        return _symbol_k != NULL ? _symbol_k->type_name_ : NULL;
    }
    default:
        return NULL;
    }
}

typedef struct zlang_emitter_scope_t {
    size_t symbol_count_;
    size_t cse_count_;
    size_t cse_pointer_count_;
} zlang_emitter_scope_t;

static zlang_emitter_scope_t zlang_save_scope(const zlang_emitter_t *emitter) {
    zlang_emitter_scope_t _scope_k;
    _scope_k.symbol_count_ = emitter->active_symbol_count_;
    _scope_k.cse_count_ = emitter->cse_count_;
    _scope_k.cse_pointer_count_ = emitter->cse_pointer_count_;
    return _scope_k;
}

static void zlang_restore_scope(zlang_emitter_t *emitter, zlang_emitter_scope_t saved) {
    emitter->active_symbol_count_ = saved.symbol_count_;
    emitter->cse_count_ = saved.cse_count_;
    emitter->cse_pointer_count_ = saved.cse_pointer_count_;
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

static const char *zlang_array_get_function(const char *type_name) {
    if (type_name == NULL) {
        return NULL;
    }
    if (strcmp(type_name, "integer") == 0) {
        return "zlang_array_get_i32(";
    }
    if (strcmp(type_name, "unsigned") == 0) {
        return "zlang_array_get_u32(";
    }
    if (strcmp(type_name, "boolean") == 0) {
        return "zlang_array_get_bool(";
    }
    if (strcmp(type_name, "string") == 0) {
        return "zlang_array_get_string(";
    }
    return NULL;
}

static const char *zlang_print_function(const char *type_name) {
    if (type_name == NULL) {
        return NULL;
    }
    if (strcmp(type_name, "string") == 0) {
        return "zlang_print(";
    }
    if (strcmp(type_name, "unsigned") == 0) {
        return "zlang_print_u32(";
    }
    if (strcmp(type_name, "boolean") == 0) {
        return "zlang_print_bool(";
    }
    if (strcmp(type_name, "integer") == 0) {
        return "zlang_print_i32(";
    }
    return NULL;
}

static const char *zlang_arithmetic_function(zlang_expression_kind_t kind, bool is_unsigned) {
    switch (kind) {
    case zlang_expression_add_:
        return is_unsigned ? "zlang_add_u32(" : "zlang_add_i32(";
    case zlang_expression_sub_:
        return is_unsigned ? "zlang_sub_u32(" : "zlang_sub_i32(";
    case zlang_expression_mul_:
        return is_unsigned ? "zlang_mul_u32(" : "zlang_mul_i32(";
    case zlang_expression_div_:
        return is_unsigned ? "zlang_div_u32(" : "zlang_div_i32(";
    case zlang_expression_mod_:
        return is_unsigned ? "zlang_mod_u32(" : "zlang_mod_i32(";
    default:
        return NULL;
    }
}

static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_emit_comparison(const zlang_program_t *program, const zlang_ast_expression_t *expression,
                      zlang_emitter_t *emitter) { // NOLINT(misc-no-recursion)
    const char *const _left_type_k = zlang_expression_type(program, emitter, expression->left_);
    const char *const _right_type_k = zlang_expression_type(program, emitter, expression->right_);
    const bool _cast_left_k = _left_type_k != NULL && _right_type_k != NULL &&
                              strcmp(_left_type_k, "integer") == 0 &&
                              strcmp(_right_type_k, "unsigned") == 0;
    const bool _cast_right_k = _left_type_k != NULL && _right_type_k != NULL &&
                               strcmp(_left_type_k, "unsigned") == 0 &&
                               strcmp(_right_type_k, "integer") == 0;
    if (_cast_left_k && !zlang_append_text(emitter, "(uint32_t)")) {
        return false;
    }
    if (!zlang_emit_expression(program, expression->left_, emitter)) {
        return false;
    }
    if (!zlang_append_text(emitter, zlang_comparison_operator(expression->kind_))) {
        return false;
    }
    if (_cast_right_k && !zlang_append_text(emitter, "(uint32_t)")) {
        return false;
    }
    return zlang_emit_expression(program, expression->right_, emitter);
}

static bool zlang_emit_string_concat_call(const zlang_program_t *program, size_t index,
                                          const char *storage, const char *capacity,
                                          zlang_emitter_t *emitter);

static bool zlang_emit_literal_view(const zlang_program_t *program, size_t index,
                                    zlang_emitter_t *emitter);

static size_t zlang_concat_part_count(const zlang_program_t *program, size_t index);

static bool zlang_emit_concat_parts(const zlang_program_t *program, size_t index,
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
        return zlang_emit_literal_view(program, index, emitter);
    case zlang_expression_identifier_: {
        const zlang_emitted_symbol_t *const _symbol_k =
            zlang_find_active_symbol(emitter, _expression_k->name_);
        if (_symbol_k != NULL) {
            return zlang_emit_symbol_value(emitter, _expression_k->name_);
        }
        const char *const _enum_k = zlang_generated_enum_item(emitter, _expression_k->name_);
        return _enum_k != NULL && zlang_append_text(emitter, _enum_k);
    }
    case zlang_expression_enum_item_:
        return zlang_append_text(emitter, zlang_generated_enum_item(emitter, _expression_k->name_));
    case zlang_expression_add_:
    case zlang_expression_sub_:
    case zlang_expression_mul_:
    case zlang_expression_div_:
    case zlang_expression_mod_: {
        const char *const _left_type_k =
            zlang_expression_type(program, emitter, _expression_k->left_);
        const char *const _function_k = zlang_arithmetic_function(
            _expression_k->kind_, _left_type_k != NULL && strcmp(_left_type_k, "unsigned") == 0);
        if (_function_k == NULL || !zlang_append_text(emitter, _function_k) ||
            !zlang_emit_expression(program, _expression_k->left_, emitter) ||
            !zlang_append_text(emitter, ", ") ||
            !zlang_emit_expression(program, _expression_k->right_, emitter) ||
            !zlang_append_text(emitter, ")")) {
            return false;
        }
        return true;
    }
    case zlang_expression_negative_:
        return zlang_append_text(emitter, "(-") &&
               zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, ")");
    case zlang_expression_length_:
        return zlang_append_text(emitter, zlang_resolve_name(emitter, _expression_k->name_)) &&
               zlang_append_text(emitter, ".length_");
    case zlang_expression_compound_assignment_: {
        const zlang_ast_expression_t *const _target_k =
            &program->expressions_[_expression_k->left_];
        const char *const _name_k = zlang_resolve_name(emitter, _target_k->name_);
        const char *const _left_type_k = zlang_expression_type(program, emitter, index);
        const char *const _function_k = zlang_arithmetic_function(
            _expression_k->operator_,
            _left_type_k != NULL && strcmp(_left_type_k, "unsigned") == 0);
        if (_name_k == NULL || _function_k == NULL || !zlang_append_text(emitter, _name_k) ||
            !zlang_append_text(emitter, " = ") || !zlang_append_text(emitter, _function_k) ||
            !zlang_append_text(emitter, _name_k) || !zlang_append_text(emitter, ", ") ||
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
        return zlang_emit_comparison(program, _expression_k, emitter);
    }
    case zlang_expression_not_:
        return zlang_append_text(emitter, "(!") &&
               zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, ")");
    case zlang_expression_and_:
        return zlang_append_text(emitter, "(") &&
               zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, " && ") &&
               zlang_emit_expression(program, _expression_k->right_, emitter) &&
               zlang_append_text(emitter, ")");
    case zlang_expression_or_:
        return zlang_append_text(emitter, "(") &&
               zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, " || ") &&
               zlang_emit_expression(program, _expression_k->right_, emitter) &&
               zlang_append_text(emitter, ")");
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
            char _capacity[ZLANG_EMITTED_NAME_CAPACITY];
            if (!zlang_symbol_capacity_text(emitter, _symbol_k, _capacity, sizeof(_capacity))) {
                return false;
            }
            return zlang_emit_string_concat_call(program, _expression_k->right_,
                                                 _symbol_k->storage_name_, _capacity, emitter);
        }
        return zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, " = ") &&
               zlang_emit_expression(program, _expression_k->right_, emitter);
    }
    case zlang_expression_print_call_: {
        const char *const _type_k = zlang_expression_type(program, emitter, _expression_k->left_);
        const char *_function_k = zlang_print_function(_type_k);
        if (_function_k == NULL && zlang_find_enum_statement(program, _type_k) != NULL) {
            _function_k = "zlang_print_u32(";
        }
        return _function_k != NULL && zlang_append_text(emitter, _function_k) &&
               zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, ")");
    }
    case zlang_expression_append_call_: {
        const zlang_ast_statement_t *const _sequence_k =
            zlang_find_sequence(emitter, _expression_k->name_);
        const char *const _name_k = zlang_resolve_name(emitter, _expression_k->name_);
        if (_sequence_k == NULL) {
            return false;
        }
        char _capacity[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_sequence_capacity_text(program, emitter, _sequence_k, _capacity,
                                          sizeof(_capacity))) {
            return false;
        }
        if (_sequence_k->has_element_capacity_ && strcmp(_sequence_k->type_name_, "string") == 0) {
            char _element_capacity[ZLANG_EMITTED_NAME_CAPACITY];
            const size_t _count_k = zlang_concat_part_count(program, _expression_k->right_);
            if (!zlang_sequence_element_capacity_text(
                    program, emitter, _sequence_k, _element_capacity, sizeof(_element_capacity)) ||
                _count_k == 0U || _count_k > UINT32_MAX ||
                !zlang_append_text(emitter, "zlang_string_array_append((char *)") ||
                !zlang_append_text(emitter, _name_k) ||
                !zlang_append_text(emitter, ".storage_, ") ||
                !zlang_append_text(emitter, _name_k) || !zlang_append_text(emitter, ".data_, &") ||
                !zlang_append_text(emitter, _name_k) || !zlang_append_text(emitter, ".length_, ") ||
                !zlang_append_text(emitter, _capacity) || !zlang_append_text(emitter, ", ") ||
                !zlang_append_text(emitter, _element_capacity) ||
                !zlang_append_text(emitter, ", (zlang_string_t[]){ ") ||
                !zlang_emit_concat_parts(program, _expression_k->right_, emitter) ||
                !zlang_append_text(emitter, " }, ") ||
                !zlang_append_part_count(emitter, _count_k) || !zlang_append_text(emitter, ")")) {
                return false;
            }
            return true;
        }
        const char *const _emitted_type_k =
            zlang_generated_type(program, emitter, _sequence_k->type_name_);
        if (_emitted_type_k == NULL || !zlang_append_text(emitter, "zlang_array_append(") ||
            !zlang_append_text(emitter, _name_k) || !zlang_append_text(emitter, ".data_, &") ||
            !zlang_append_text(emitter, _name_k) || !zlang_append_text(emitter, ".length_, ") ||
            !zlang_append_text(emitter, _capacity) || !zlang_append_text(emitter, ", sizeof(") ||
            !zlang_append_text(emitter, _name_k) || !zlang_append_text(emitter, ".data_[0]), (") ||
            !zlang_append_text(emitter, _emitted_type_k) || !zlang_append_text(emitter, "[]){ (") ||
            !zlang_emit_expression(program, _expression_k->right_, emitter) ||
            !zlang_append_text(emitter, ") })")) {
            return false;
        }
        return true;
    }
    case zlang_expression_field_:
    case zlang_expression_field_assignment_: {
        const zlang_ast_expression_t *const _field_k =
            _expression_k->kind_ == zlang_expression_field_assignment_
                ? &program->expressions_[_expression_k->left_]
                : _expression_k;
        if (_expression_k->kind_ == zlang_expression_field_) {
            const char *const _pointer_k = zlang_find_pointer_cse(emitter, index);
            if (_pointer_k != NULL) {
                return zlang_append_text(emitter, _pointer_k) && zlang_append_text(emitter, "->") &&
                       zlang_append_field_name(emitter, _field_k->name_);
            }
        }
        const char *_separator_k = ".";
        if (_field_k->base_name_ != NULL) {
            const zlang_emitted_symbol_t *const _symbol_k =
                zlang_find_active_symbol(emitter, _field_k->base_name_);
            const char *const _generated_type_k =
                _symbol_k != NULL ? zlang_generated_type(program, emitter, _symbol_k->type_name_)
                                  : NULL;
            if (_symbol_k == NULL || _generated_type_k == NULL) {
                return false;
            }
            if (_field_k->right_ != ZLANG_NO_EXPRESSION) {
                if (!zlang_append_text(emitter, "(*((") ||
                    !zlang_append_text(emitter, _generated_type_k) ||
                    !zlang_append_text(emitter, " *)zlang_array_get(") ||
                    !zlang_append_text(emitter, _symbol_k->generated_name_) ||
                    !zlang_append_text(emitter, ".data_, ") ||
                    !zlang_emit_expression(program, _field_k->right_, emitter) ||
                    !zlang_append_text(emitter, ", ") ||
                    !zlang_append_text(emitter, _symbol_k->generated_name_) ||
                    !zlang_append_text(emitter, ".length_, sizeof(") ||
                    !zlang_append_text(emitter, _symbol_k->generated_name_) ||
                    !zlang_append_text(emitter, ".data_[0]))))")) {
                    return false;
                }
            } else if (!zlang_append_text(emitter, _symbol_k->generated_name_)) {
                return false;
            }
            if (_field_k->right_ == ZLANG_NO_EXPRESSION && _symbol_k->is_reference_) {
                _separator_k = "->";
            }
        } else if (!zlang_emit_expression(program, _field_k->left_, emitter)) {
            return false;
        }
        if (!zlang_append_text(emitter, _separator_k) ||
            !zlang_append_field_name(emitter, _field_k->name_)) {
            return false;
        }
        if (_expression_k->kind_ == zlang_expression_field_assignment_) {
            return zlang_append_text(emitter, " = ") &&
                   zlang_emit_expression(program, _expression_k->right_, emitter);
        }
        return true;
    }
    case zlang_expression_index_: {
        const char *const _cse_k = zlang_find_cse(emitter, index);
        if (_cse_k != NULL) {
            return zlang_append_text(emitter, _cse_k);
        }
        const char *const _name_k = zlang_resolve_name(emitter, _expression_k->name_);
        const zlang_emitted_symbol_t *const _symbol_k =
            zlang_find_active_symbol(emitter, _expression_k->name_);
        if (_name_k == NULL || _symbol_k == NULL) {
            return false;
        }
        if (!_symbol_k->is_sequence_ && strcmp(_symbol_k->type_name_, "string") == 0) {
            return zlang_append_text(emitter, "zlang_string_byte(") &&
                   zlang_append_text(emitter, _name_k) && zlang_append_text(emitter, ", ") &&
                   zlang_emit_expression(program, _expression_k->right_, emitter) &&
                   zlang_append_text(emitter, ")");
        }
        const char *const _function_k = zlang_array_get_function(_symbol_k->type_name_);
        if (_function_k == NULL) {
            const char *const _generated_type_k =
                zlang_generated_type(program, emitter, _symbol_k->type_name_);
            if (_generated_type_k == NULL || !zlang_append_text(emitter, "(*((") ||
                !zlang_append_text(emitter, _generated_type_k) ||
                !zlang_append_text(emitter, " *)zlang_array_get(") ||
                !zlang_append_text(emitter, _name_k) || !zlang_append_text(emitter, ".data_, ") ||
                !zlang_emit_expression(program, _expression_k->right_, emitter) ||
                !zlang_append_text(emitter, ", ") || !zlang_append_text(emitter, _name_k) ||
                !zlang_append_text(emitter, ".length_, sizeof(") ||
                !zlang_append_text(emitter, _name_k) ||
                !zlang_append_text(emitter, ".data_[0]))))")) {
                return false;
            }
            return true;
        }
        if (!zlang_append_text(emitter, _function_k) || !zlang_append_text(emitter, _name_k) ||
            !zlang_append_text(emitter, ".data_, ") ||
            !zlang_emit_expression(program, _expression_k->right_, emitter) ||
            !zlang_append_text(emitter, ", ") || !zlang_append_text(emitter, _name_k) ||
            !zlang_append_text(emitter, ".length_)")) {
            return false;
        }
        return true;
    }
    case zlang_expression_call_: {
        if (!zlang_append_text(emitter, zlang_resolve_name(emitter, _expression_k->name_)) ||
            !zlang_append_text(emitter, "(")) {
            return false;
        }
        if (_expression_k->argument_count_ > ZLANG_CALL_ARGUMENT_CAPACITY) {
            return false;
        }
        const zlang_ast_statement_t *const _function_statement_k =
            zlang_find_function_statement(program, _expression_k->name_);
        for (size_t _argument = 0U; _argument < _expression_k->argument_count_; ++_argument) {
            const size_t _index_k = _expression_k->argument_indices_[_argument];
            const zlang_parameter_t *_parameter_k = NULL;
            if (_function_statement_k != NULL &&
                _argument < _function_statement_k->parameter_count_ &&
                _function_statement_k->parameter_start_ + _argument < program->parameter_count_) {
                _parameter_k =
                    &program->parameters_[_function_statement_k->parameter_start_ + _argument];
            }
            if (_argument > 0U && !zlang_append_text(emitter, ", ")) {
                return false;
            }
            if (_parameter_k != NULL && _parameter_k->is_reference_) {
                if (!zlang_emit_reference_argument(program, _index_k, emitter)) {
                    return false;
                }
            } else if (!zlang_emit_expression(program, _index_k, emitter)) {
                return false;
            }
        }
        return zlang_append_text(emitter, ")");
    }
    default:
        return false;
    }
}

static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_emit_reference_argument(const zlang_program_t *program, size_t index,
                              zlang_emitter_t *emitter) { // NOLINT(misc-no-recursion)
    if (index == ZLANG_NO_EXPRESSION || index >= program->expression_count_) {
        return false;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ == zlang_expression_identifier_) {
        const zlang_emitted_symbol_t *const _symbol_k =
            zlang_find_active_symbol(emitter, _expression_k->name_);
        if (_symbol_k == NULL || _symbol_k->is_reference_) {
            return zlang_append_text(emitter, zlang_resolve_name(emitter, _expression_k->name_));
        }
        return zlang_append_text(emitter, "&") &&
               zlang_append_text(emitter, _symbol_k->generated_name_);
    }
    if (_expression_k->kind_ == zlang_expression_index_) {
        const zlang_emitted_symbol_t *const _symbol_k =
            zlang_find_active_symbol(emitter, _expression_k->name_);
        if (_symbol_k == NULL || !_symbol_k->is_sequence_ ||
            zlang_array_get_function(_symbol_k->type_name_) != NULL) {
            return false;
        }
        const char *const _type_k = zlang_generated_type(program, emitter, _symbol_k->type_name_);
        return _type_k != NULL && zlang_append_text(emitter, "(") &&
               zlang_append_text(emitter, _type_k) &&
               zlang_append_text(emitter, " *)zlang_array_get(") &&
               zlang_append_text(emitter, _symbol_k->generated_name_) &&
               zlang_append_text(emitter, ".data_, ") &&
               zlang_emit_expression(program, _expression_k->right_, emitter) &&
               zlang_append_text(emitter, ", ") &&
               zlang_append_text(emitter, _symbol_k->generated_name_) &&
               zlang_append_text(emitter, ".length_, sizeof(") &&
               zlang_append_text(emitter, _symbol_k->generated_name_) &&
               zlang_append_text(emitter, ".data_[0]))");
    }
    if (_expression_k->kind_ == zlang_expression_field_) {
        const zlang_ast_expression_t *const _field_k = _expression_k;
        if (_field_k->base_name_ != NULL && _field_k->right_ == ZLANG_NO_EXPRESSION) {
            const zlang_emitted_symbol_t *const _symbol_k =
                zlang_find_active_symbol(emitter, _field_k->base_name_);
            if (_symbol_k == NULL) {
                return false;
            }
            return zlang_append_text(emitter, "&") &&
                   zlang_append_text(emitter, _symbol_k->generated_name_) &&
                   zlang_append_text(emitter, _symbol_k->is_reference_ ? "->" : ".") &&
                   zlang_append_field_name(emitter, _field_k->name_);
        }
        return zlang_append_text(emitter, "&") && zlang_emit_expression(program, index, emitter);
    }
    return false;
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
    if (program == NULL || program->expressions_ == NULL || index == ZLANG_NO_EXPRESSION ||
        index >= program->expression_count_) {
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
    if (!zlang_folded_string_length(program, index, &_length_k)) {
        return false;
    }
    if (zlang_emit_literal_view(program, index, emitter)) {
        return true;
    }
    if (!zlang_append_text(emitter, "(zlang_string_t){ \"")) {
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

static const zlang_emitted_literal_t *zlang_find_literal(const zlang_emitter_t *emitter,
                                                         size_t expression_index) {
    for (size_t _index = 0U; _index < emitter->emitted_literal_count_; ++_index) {
        const zlang_emitted_literal_t *const _entry_k = &emitter->emitted_literals_[_index];
        if (_entry_k->expression_index_ == expression_index) {
            return _entry_k;
        }
    }
    return NULL;
}

static bool zlang_emit_literal_view(const zlang_program_t *program, size_t index,
                                    zlang_emitter_t *emitter) {
    (void)program;
    const zlang_emitted_literal_t *const _entry_k = zlang_find_literal(emitter, index);
    return _entry_k != NULL && zlang_append_text(emitter, _entry_k->view_name_);
}

static bool zlang_is_folded_child(const zlang_program_t *program, size_t index) {
    for (size_t _candidate = 0U; _candidate < program->expression_count_; ++_candidate) {
        const zlang_ast_expression_t *const _expression_k = &program->expressions_[_candidate];
        if (_expression_k->kind_ != zlang_expression_string_concat_ ||
            !zlang_is_literal_string(program, _candidate)) {
            continue;
        }
        if (_expression_k->left_ == index || _expression_k->right_ == index) {
            return true;
        }
    }
    return false;
}

static bool zlang_literal_content_matches(const zlang_program_t *program, size_t left,
                                          size_t right) {
    if (left >= program->expression_count_ || right >= program->expression_count_) {
        return false;
    }
    const zlang_ast_expression_t *const _left_k = &program->expressions_[left];
    const zlang_ast_expression_t *const _right_k = &program->expressions_[right];
    if (_left_k->kind_ != zlang_expression_string_literal_ ||
        _right_k->kind_ != zlang_expression_string_literal_) {
        return false;
    }
    return _left_k->string_.length_ == _right_k->string_.length_ &&
           strcmp(_left_k->string_.data_, _right_k->string_.data_) == 0;
}

static bool zlang_collect_literals(zlang_emitter_t *emitter, const zlang_program_t *program) {
    for (size_t _index = 0U; _index < program->expression_count_; ++_index) {
        const zlang_ast_expression_t *const _expression_k = &program->expressions_[_index];
        const bool _is_literal_k = _expression_k->kind_ == zlang_expression_string_literal_ ||
                                   (_expression_k->kind_ == zlang_expression_string_concat_ &&
                                    zlang_is_literal_string(program, _index));
        if (!_is_literal_k || zlang_find_literal(emitter, _index) != NULL ||
            (_expression_k->kind_ == zlang_expression_string_literal_ &&
             zlang_is_folded_child(program, _index))) {
            continue;
        }
        if (emitter->emitted_literal_count_ >= ZLANG_EMITTED_LITERAL_CAPACITY) {
            return false;
        }
        size_t _duplicate_k = ZLANG_NO_EXPRESSION;
        for (size_t _candidate = 0U; _candidate < emitter->emitted_literal_count_; ++_candidate) {
            const zlang_emitted_literal_t *const _candidate_k =
                &emitter->emitted_literals_[_candidate];
            if (_candidate_k->is_primary_ &&
                zlang_literal_content_matches(program, _index,
                                              _candidate_k->primary_expression_index_)) {
                _duplicate_k = _candidate;
                break;
            }
        }
        zlang_emitted_literal_t *const _entry_k =
            &emitter->emitted_literals_[emitter->emitted_literal_count_++];
        memset(_entry_k, 0, sizeof(*_entry_k));
        _entry_k->expression_index_ = _index;
        if (_duplicate_k != ZLANG_NO_EXPRESSION) {
            const zlang_emitted_literal_t *const _primary_k =
                &emitter->emitted_literals_[_duplicate_k];
            _entry_k->primary_expression_index_ = _primary_k->primary_expression_index_;
            memcpy(_entry_k->data_name_, _primary_k->data_name_, sizeof(_entry_k->data_name_));
            memcpy(_entry_k->view_name_, _primary_k->view_name_, sizeof(_entry_k->view_name_));
            memcpy(_entry_k->length_macro_, _primary_k->length_macro_,
                   sizeof(_entry_k->length_macro_));
            continue;
        }
        _entry_k->primary_expression_index_ = _index;
        _entry_k->is_primary_ = true;
        const unsigned long _literal_index_k = emitter->next_index_++;
        const int _data_written_k = snprintf(_entry_k->data_name_, sizeof(_entry_k->data_name_),
                                             "_zlang_literal_%lu_data", _literal_index_k);
        const int _view_written_k = snprintf(_entry_k->view_name_, sizeof(_entry_k->view_name_),
                                             "_zlang_literal_%lu", _literal_index_k);
        const int _length_written_k =
            snprintf(_entry_k->length_macro_, sizeof(_entry_k->length_macro_),
                     "ZLANG_LITERAL_%lu_LENGTH", _literal_index_k);
        if (_data_written_k < 0 || (size_t)_data_written_k >= sizeof(_entry_k->data_name_) ||
            _view_written_k < 0 || (size_t)_view_written_k >= sizeof(_entry_k->view_name_) ||
            _length_written_k < 0 || (size_t)_length_written_k >= sizeof(_entry_k->length_macro_)) {
            return false;
        }
    }
    return true;
}

static bool zlang_emit_literal_declarations(zlang_emitter_t *emitter,
                                            const zlang_program_t *program) {
    if (emitter->emitted_literal_count_ == 0U) {
        return true;
    }
    if (!zlang_append_text(emitter, "/* generated string literals */\n")) {
        return false;
    }
    for (size_t _index = 0U; _index < emitter->emitted_literal_count_; ++_index) {
        const zlang_emitted_literal_t *const _entry_k = &emitter->emitted_literals_[_index];
        if (!_entry_k->is_primary_) {
            continue;
        }
        uint32_t _length_k = 0U;
        if (!zlang_folded_string_length(program, _entry_k->primary_expression_index_, &_length_k) ||
            !zlang_append_text(emitter, "#define ") ||
            !zlang_append_text(emitter, _entry_k->length_macro_) ||
            !zlang_append_text(emitter, " ") || !zlang_append_unsigned(emitter, _length_k) ||
            !zlang_append_text(emitter, "U\nstatic const char ") ||
            !zlang_append_text(emitter, _entry_k->data_name_) ||
            !zlang_append_text(emitter, "[] = \"") ||
            !zlang_emit_folded_string_data(program, _entry_k->primary_expression_index_, emitter) ||
            !zlang_append_text(emitter, "\";\nstatic const zlang_string_t ") ||
            !zlang_append_text(emitter, _entry_k->view_name_) ||
            !zlang_append_text(emitter, " = { ") ||
            !zlang_append_text(emitter, _entry_k->data_name_) ||
            !zlang_append_text(emitter, ", ") ||
            !zlang_append_text(emitter, _entry_k->length_macro_) ||
            !zlang_append_text(emitter, " };\n")) {
            return false;
        }
    }
    return zlang_append_text(emitter, "\n");
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_string_concat_call(const zlang_program_t *program, size_t index,
                                          const char *storage, const char *capacity,
                                          zlang_emitter_t *emitter) { // NOLINT(misc-no-recursion)
    const size_t _count_k = zlang_concat_part_count(program, index);
    if (_count_k == 0U || _count_k > UINT32_MAX || capacity == NULL) {
        return false;
    }
    return zlang_append_text(emitter, "zlang_string_concat(") &&
           zlang_append_text(emitter, storage) && zlang_append_text(emitter, ", ") &&
           zlang_append_text(emitter, capacity) &&
           zlang_append_text(emitter, ", (zlang_string_t[]){ ") &&
           zlang_emit_concat_parts(program, index, emitter) && zlang_append_text(emitter, " }, ") &&
           zlang_append_part_count(emitter, _count_k) && zlang_append_text(emitter, ")");
}

static bool zlang_emit_index_assignment(const zlang_program_t *program,
                                        const zlang_ast_expression_t *expression,
                                        zlang_emitter_t *emitter, uint32_t depth) {
    const zlang_ast_expression_t *const _target_k = &program->expressions_[expression->left_];
    const char *const _generated_k = zlang_resolve_name(emitter, _target_k->name_);
    char _index_name[32];
    const int _written_k = snprintf(_index_name, sizeof(_index_name), "_zlang_index_%lu",
                                    (unsigned long)emitter->next_index_++);
    if (_target_k->kind_ != zlang_expression_index_ || _generated_k == NULL || _written_k < 0 ||
        (size_t)_written_k >= sizeof(_index_name)) {
        return false;
    }
    const zlang_ast_statement_t *const _sequence_k = zlang_find_sequence(emitter, _target_k->name_);
    const bool _string_element_k = _sequence_k != NULL && _sequence_k->has_element_capacity_ &&
                                   strcmp(_sequence_k->type_name_, "string") == 0;
    char _element_capacity[ZLANG_EMITTED_NAME_CAPACITY];
    if (_string_element_k &&
        !zlang_sequence_element_capacity_text(program, emitter, _sequence_k, _element_capacity,
                                              sizeof(_element_capacity))) {
        return false;
    }

    if (!zlang_append_text(emitter, "uint32_t ") || !zlang_append_text(emitter, _index_name) ||
        !zlang_append_text(emitter, " = ") ||
        !zlang_emit_expression(program, _target_k->right_, emitter) ||
        !zlang_append_text(emitter, ";\n") || !zlang_append_indent(emitter, depth) ||
        !zlang_append_text(emitter, "if (") || !zlang_append_text(emitter, _index_name) ||
        !zlang_append_text(emitter, " < ") || !zlang_append_text(emitter, _generated_k) ||
        !zlang_append_text(emitter, ".length_) {\n") || !zlang_append_indent(emitter, depth + 1U) ||
        !zlang_append_text(emitter, _generated_k) || !zlang_append_text(emitter, ".data_[") ||
        !zlang_append_text(emitter, _index_name) || !zlang_append_text(emitter, "] = ")) {
        return false;
    }
    if (_string_element_k) {
        char _slot[ZLANG_EMITTED_NAME_CAPACITY + 32];
        const int _slot_written_k =
            snprintf(_slot, sizeof(_slot), "%s.storage_[%s]", _generated_k, _index_name);
        if (_slot_written_k < 0 || (size_t)_slot_written_k >= sizeof(_slot) ||
            !zlang_emit_string_concat_call(program, expression->right_, _slot, _element_capacity,
                                           emitter)) {
            return false;
        }
    } else if (!zlang_emit_expression(program, expression->right_, emitter)) {
        return false;
    }
    return zlang_append_text(emitter, ";\n") && zlang_append_indent(emitter, depth) &&
           zlang_append_text(emitter, "} else {\n") && zlang_append_indent(emitter, depth + 1U) &&
           zlang_append_text(emitter, "zlang_trap(ZLANG_TRAP_BOUNDS);\n") &&
           zlang_append_indent(emitter, depth) && zlang_append_text(emitter, "}\n");
}

static bool zlang_emit_field_assignment(const zlang_program_t *program,
                                        const zlang_ast_expression_t *expression,
                                        zlang_emitter_t *emitter) {
    const zlang_ast_expression_t *const _target_k = &program->expressions_[expression->left_];
    const char *const _pointer_k = zlang_find_pointer_cse(emitter, expression->left_);
    if (_target_k->kind_ != zlang_expression_field_ || _pointer_k == NULL) {
        return false;
    }
    return zlang_append_text(emitter, _pointer_k) && zlang_append_text(emitter, "->") &&
           zlang_append_field_name(emitter, _target_k->name_) &&
           zlang_append_text(emitter, " = ") &&
           zlang_emit_expression(program, expression->right_, emitter) &&
           zlang_append_text(emitter, ";\n");
}

static bool zlang_emit_body(const zlang_program_t *program, size_t start, size_t count,
                            zlang_emitter_t *emitter, uint32_t depth);

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_statement(const zlang_program_t *program,
                                 const zlang_ast_statement_t *statement, zlang_emitter_t *emitter,
                                 uint32_t depth);

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_if_statement(const zlang_program_t *program,
                                    const zlang_ast_statement_t *statement,
                                    zlang_emitter_t *emitter, uint32_t depth, bool save_values);

static bool zlang_emit_condition(const zlang_program_t *program, size_t index,
                                 zlang_emitter_t *emitter) {
    if (index == ZLANG_NO_EXPRESSION || index >= program->expression_count_) {
        return false;
    }
    const zlang_expression_kind_t _kind_k = program->expressions_[index].kind_;
    const bool _self_grouped_k =
        _kind_k == zlang_expression_not_ || _kind_k == zlang_expression_and_ ||
        _kind_k == zlang_expression_or_ || _kind_k == zlang_expression_index_;
    if (_self_grouped_k) {
        return zlang_emit_expression(program, index, emitter);
    }
    return zlang_append_text(emitter, "(") && zlang_emit_expression(program, index, emitter) &&
           zlang_append_text(emitter, ")");
}

static bool zlang_is_nested_in_body(const zlang_program_t *program, size_t index, size_t start,
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

static bool zlang_is_grouped_statement(const zlang_program_t *program,
                                       const zlang_ast_statement_t *statement) {
    if (statement->kind_ == zlang_statement_if_ || statement->kind_ == zlang_statement_while_ ||
        statement->kind_ == zlang_statement_for_ || statement->kind_ == zlang_statement_foreach_ ||
        statement->kind_ == zlang_statement_repeat_ || statement->kind_ == zlang_statement_block_) {
        return true;
    }
    return statement->kind_ == zlang_statement_expression_ &&
           statement->expression_ < program->expression_count_ &&
           program->expressions_[statement->expression_].kind_ ==
               zlang_expression_index_assignment_;
}

static bool zlang_should_insert_blank_line(bool has_previous, bool previous_grouped,
                                           bool current_grouped,
                                           zlang_statement_kind_t current_kind) {
    if (!has_previous) {
        return false;
    }
    return previous_grouped || current_grouped || current_kind == zlang_statement_return_;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_body(const zlang_program_t *program, size_t start, size_t count,
                            zlang_emitter_t *emitter, uint32_t depth) { // NOLINT(misc-no-recursion)
    const zlang_emitter_scope_t _scope_k = zlang_save_scope(emitter);
    bool _has_previous_k = false;
    bool _previous_grouped_k = false;
    bool _result_k = true;
    for (size_t _index = 0U; _index < count && _result_k; ++_index) {
        const size_t _statement_index_k = start + _index;
        if (_statement_index_k >= program->statement_count_) {
            _result_k = false;
            break;
        }
        if (zlang_is_nested_in_body(program, _statement_index_k, start, count)) {
            continue;
        }
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_statement_index_k];
        const bool _grouped_k = zlang_is_grouped_statement(program, _statement_k);
        if (zlang_should_insert_blank_line(_has_previous_k, _previous_grouped_k, _grouped_k,
                                           _statement_k->kind_) &&
            !zlang_append_text(emitter, "\n")) {
            _result_k = false;
            break;
        }
        if (!zlang_emit_statement(program, _statement_k, emitter, depth)) {
            _result_k = false;
            break;
        }
        _previous_grouped_k = _grouped_k;
        _has_previous_k = true;
    }
    zlang_restore_scope(emitter, _scope_k);
    return _result_k;
}

static bool zlang_same_text(const char *left, const char *right) {
    if (left == NULL || right == NULL) {
        return left == right;
    }
    return strcmp(left, right) == 0;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_expression_equal(const zlang_program_t *program, size_t left,
                                   size_t right) { // NOLINT(misc-no-recursion)
    if (left == ZLANG_NO_EXPRESSION || right == ZLANG_NO_EXPRESSION ||
        left >= program->expression_count_ || right >= program->expression_count_) {
        return left == right;
    }
    const zlang_ast_expression_t *const _left_k = &program->expressions_[left];
    const zlang_ast_expression_t *const _right_k = &program->expressions_[right];
    if (_left_k->kind_ != _right_k->kind_ || !zlang_same_text(_left_k->name_, _right_k->name_) ||
        _left_k->integer_value_ != _right_k->integer_value_ ||
        _left_k->bool_value_ != _right_k->bool_value_ ||
        _left_k->is_unsigned_ != _right_k->is_unsigned_ ||
        _left_k->operator_ != _right_k->operator_ ||
        _left_k->string_.length_ != _right_k->string_.length_ ||
        !zlang_same_text(_left_k->string_.data_, _right_k->string_.data_)) {
        return false;
    }
    if (_left_k->argument_count_ != _right_k->argument_count_ ||
        _left_k->argument_count_ > ZLANG_CALL_ARGUMENT_CAPACITY) {
        return false;
    }
    for (size_t _argument = 0U; _argument < _left_k->argument_count_; ++_argument) {
        if (!zlang_expression_equal(program, _left_k->argument_indices_[_argument],
                                    _right_k->argument_indices_[_argument])) {
            return false;
        }
    }
    return zlang_expression_equal(program, _left_k->left_, _right_k->left_) &&
           zlang_expression_equal(program, _left_k->right_, _right_k->right_);
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_collect_field_references(const zlang_program_t *program, size_t index,
                                           size_t *items, size_t *count,
                                           size_t capacity) { // NOLINT(misc-no-recursion)
    if (index == ZLANG_NO_EXPRESSION || index >= program->expression_count_ || *count >= capacity) {
        return true;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ == zlang_expression_field_ && _expression_k->base_name_ != NULL &&
        _expression_k->right_ != ZLANG_NO_EXPRESSION) {
        bool _already_k = false;
        for (size_t _item = 0U; _item < *count; ++_item) {
            if (items[_item] == index) {
                _already_k = true;
                break;
            }
        }
        if (!_already_k) {
            items[(*count)++] = index;
        }
    }
    const bool _short_circuit_k = _expression_k->kind_ == zlang_expression_and_ ||
                                  _expression_k->kind_ == zlang_expression_or_;
    if (!zlang_collect_field_references(program, _expression_k->left_, items, count, capacity) ||
        (!_short_circuit_k &&
         !zlang_collect_field_references(program, _expression_k->right_, items, count, capacity))) {
        return false;
    }
    if (_expression_k->argument_count_ > ZLANG_CALL_ARGUMENT_CAPACITY) {
        return false;
    }
    for (size_t _argument = 0U; _argument < _expression_k->argument_count_; ++_argument) {
        if (!zlang_collect_field_references(program, _expression_k->argument_indices_[_argument],
                                            items, count, capacity)) {
            return false;
        }
    }
    return true;
}

static bool zlang_collect_statement_field_references(const zlang_program_t *program,
                                                     const zlang_ast_statement_t *statement,
                                                     size_t *items, size_t *count,
                                                     size_t capacity) {
    if (!zlang_collect_field_references(program, statement->expression_, items, count, capacity) ||
        !zlang_collect_field_references(program, statement->update_expression_, items, count,
                                        capacity)) {
        return false;
    }
    if (statement->initializer_statement_ != ZLANG_NO_STATEMENT &&
        statement->initializer_statement_ < program->statement_count_ &&
        !zlang_collect_field_references(
            program, program->statements_[statement->initializer_statement_].expression_, items,
            count, capacity)) {
        return false;
    }
    const size_t _end_k = statement->initializer_start_ + statement->initializer_count_;
    for (size_t _item = statement->initializer_start_;
         _item < _end_k && _item < program->expression_count_; ++_item) {
        if (!zlang_collect_field_references(program, _item, items, count, capacity)) {
            return false;
        }
    }
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_collect_index_expressions(const zlang_program_t *program, size_t index,
                                            size_t *items, size_t *count,
                                            size_t capacity) { // NOLINT(misc-no-recursion)
    if (index == ZLANG_NO_EXPRESSION || index >= program->expression_count_ || *count >= capacity) {
        return true;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ == zlang_expression_index_) {
        items[(*count)++] = index;
    }
    if (!zlang_collect_index_expressions(program, _expression_k->left_, items, count, capacity) ||
        !zlang_collect_index_expressions(program, _expression_k->right_, items, count, capacity)) {
        return false;
    }
    if (_expression_k->argument_count_ > ZLANG_CALL_ARGUMENT_CAPACITY) {
        return false;
    }
    for (size_t _argument = 0U; _argument < _expression_k->argument_count_; ++_argument) {
        if (!zlang_collect_index_expressions(program, _expression_k->argument_indices_[_argument],
                                             items, count, capacity)) {
            return false;
        }
    }
    return true;
}

static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_collect_unconditional_index_expressions(const zlang_program_t *program, size_t index,
                                              size_t *items, size_t *count,
                                              size_t capacity) { // NOLINT(misc-no-recursion)
    if (index == ZLANG_NO_EXPRESSION || index >= program->expression_count_ || *count >= capacity) {
        return true;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ == zlang_expression_index_) {
        items[(*count)++] = index;
    }
    const bool _short_circuit_k = _expression_k->kind_ == zlang_expression_and_ ||
                                  _expression_k->kind_ == zlang_expression_or_;
    if (!zlang_collect_unconditional_index_expressions(program, _expression_k->left_, items, count,
                                                       capacity) ||
        (!_short_circuit_k && !zlang_collect_unconditional_index_expressions(
                                  program, _expression_k->right_, items, count, capacity))) {
        return false;
    }
    if (_expression_k->argument_count_ > ZLANG_CALL_ARGUMENT_CAPACITY) {
        return false;
    }
    for (size_t _argument = 0U; _argument < _expression_k->argument_count_; ++_argument) {
        if (!zlang_collect_unconditional_index_expressions(
                program, _expression_k->argument_indices_[_argument], items, count, capacity)) {
            return false;
        }
    }
    return true;
}

static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_collect_statement_index_expressions(const zlang_program_t *program,
                                          const zlang_ast_statement_t *statement, size_t *items,
                                          size_t *count,
                                          size_t capacity) { // NOLINT(misc-no-recursion)
    if (statement == NULL) {
        return true;
    }
    if (!zlang_collect_index_expressions(program, statement->expression_, items, count, capacity) ||
        !zlang_collect_index_expressions(program, statement->update_expression_, items, count,
                                         capacity)) {
        return false;
    }
    if (statement->initializer_statement_ != ZLANG_NO_STATEMENT &&
        statement->initializer_statement_ < program->statement_count_ &&
        !zlang_collect_statement_index_expressions(
            program, &program->statements_[statement->initializer_statement_], items, count,
            capacity)) {
        return false;
    }
    const size_t _initializer_end_k = statement->initializer_start_ + statement->initializer_count_;
    for (size_t _item = statement->initializer_start_;
         _item < _initializer_end_k && _item < program->expression_count_; ++_item) {
        if (!zlang_collect_index_expressions(program, _item, items, count, capacity)) {
            return false;
        }
    }
    const size_t _body_end_k = statement->body_start_ + statement->body_count_;
    for (size_t _item = statement->body_start_;
         _item < _body_end_k && _item < program->statement_count_; ++_item) {
        if (!zlang_collect_statement_index_expressions(program, &program->statements_[_item], items,
                                                       count, capacity)) {
            return false;
        }
    }
    const size_t _else_end_k = statement->else_start_ + statement->else_count_;
    for (size_t _item = statement->else_start_;
         _item < _else_end_k && _item < program->statement_count_; ++_item) {
        if (!zlang_collect_statement_index_expressions(program, &program->statements_[_item], items,
                                                       count, capacity)) {
            return false;
        }
    }
    return true;
}

static bool zlang_save_condition_values(const zlang_program_t *program,
                                        const zlang_ast_statement_t *statement,
                                        zlang_emitter_t *emitter, uint32_t depth) {
    size_t _condition_candidates[64];
    size_t _condition_count_k = 0U;
    const zlang_ast_statement_t *_current_k = statement;
    for (size_t _chain = 0U; _chain < 32U; ++_chain) {
        if (!zlang_collect_index_expressions(
                program, _current_k->expression_, _condition_candidates, &_condition_count_k,
                sizeof(_condition_candidates) / sizeof(_condition_candidates[0]))) {
            return false;
        }
        if (_current_k->else_count_ != 1U ||
            program->statements_[_current_k->else_start_].kind_ != zlang_statement_if_) {
            break;
        }
        _current_k = &program->statements_[_current_k->else_start_];
    }

    size_t _safe_candidates[64];
    size_t _safe_count_k = 0U;
    if (!zlang_collect_unconditional_index_expressions(
            program, statement->expression_, _safe_candidates, &_safe_count_k,
            sizeof(_safe_candidates) / sizeof(_safe_candidates[0]))) {
        return false;
    }

    size_t _all_candidates[128];
    size_t _all_count_k = 0U;
    if (!zlang_collect_statement_index_expressions(
            program, statement, _all_candidates, &_all_count_k,
            sizeof(_all_candidates) / sizeof(_all_candidates[0]))) {
        return false;
    }

    for (size_t _index = 0U; _index < _condition_count_k; ++_index) {
        const size_t _candidate_k = _condition_candidates[_index];
        bool _safe_k = false;
        for (size_t _safe = 0U; _safe < _safe_count_k; ++_safe) {
            if (zlang_expression_equal(program, _candidate_k, _safe_candidates[_safe])) {
                _safe_k = true;
                break;
            }
        }
        if (!_safe_k || zlang_find_cse(emitter, _candidate_k) != NULL) {
            continue;
        }
        size_t _occurrences = 0U;
        for (size_t _other = 0U; _other < _all_count_k; ++_other) {
            if (zlang_expression_equal(program, _candidate_k, _all_candidates[_other])) {
                ++_occurrences;
            }
        }
        if (_occurrences < 2U || emitter->cse_count_ >= ZLANG_EMITTED_CSE_CAPACITY) {
            continue;
        }
        const char *const _type_k = zlang_expression_type(program, emitter, _candidate_k);
        const char *const _c_type_k = zlang_generated_type(program, emitter, _type_k);
        if (_c_type_k == NULL) {
            return false;
        }
        char _name[ZLANG_EMITTED_NAME_CAPACITY];
        const int _written_k = snprintf(_name, sizeof(_name), "_zlang_value_%lu",
                                        (unsigned long)emitter->next_index_++);
        if (_written_k < 0 || (size_t)_written_k >= sizeof(_name) ||
            !zlang_append_line_break(emitter) || !zlang_append_indent(emitter, depth) ||
            !zlang_append_text(emitter, _c_type_k) || !zlang_append_text(emitter, " ") ||
            !zlang_append_text(emitter, _name) || !zlang_append_text(emitter, " = ") ||
            !zlang_emit_expression(program, _candidate_k, emitter) ||
            !zlang_append_text(emitter, ";\n")) {
            return false;
        }
        for (size_t _other = 0U; _other < _all_count_k; ++_other) {
            if (!zlang_expression_equal(program, _candidate_k, _all_candidates[_other]) ||
                zlang_find_cse(emitter, _all_candidates[_other]) != NULL) {
                continue;
            }
            emitter->cse_expression_indices_[emitter->cse_count_] = _all_candidates[_other];
            snprintf(emitter->cse_names_[emitter->cse_count_], sizeof(emitter->cse_names_[0]), "%s",
                     _name);
            ++emitter->cse_count_;
        }
    }
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_if_statement(const zlang_program_t *program,
                                    const zlang_ast_statement_t *statement,
                                    zlang_emitter_t *emitter, uint32_t depth,
                                    bool save_values) { // NOLINT(misc-no-recursion)
    bool _result_k = true;
    if (save_values) {
        const size_t _saved_cse_k = emitter->cse_count_;
        _result_k = zlang_save_condition_values(program, statement, emitter, depth);
        if (_result_k && emitter->cse_count_ > _saved_cse_k) {
            _result_k = zlang_append_indent(emitter, depth);
        }
    }
    if (_result_k) {
        _result_k = zlang_append_text(emitter, "if ") &&
                    zlang_emit_condition(program, statement->expression_, emitter) &&
                    zlang_append_text(emitter, " {\n");
    }
    if (_result_k) {
        const zlang_emitter_scope_t _if_scope_k = zlang_save_scope(emitter);
        _result_k = zlang_emit_body(program, statement->body_start_, statement->body_count_,
                                    emitter, depth + 1U) &&
                    zlang_append_indent(emitter, depth) && zlang_append_text(emitter, "}");
        zlang_restore_scope(emitter, _if_scope_k);
    }
    if (_result_k && statement->else_count_ == 0U) {
        _result_k = zlang_append_text(emitter, "\n");
    } else if (_result_k && statement->else_count_ == 1U &&
               program->statements_[statement->else_start_].kind_ == zlang_statement_if_) {
        _result_k = zlang_append_text(emitter, " else ") &&
                    zlang_emit_if_statement(program, &program->statements_[statement->else_start_],
                                            emitter, depth, false);
    } else if (_result_k) {
        const zlang_emitter_scope_t _else_scope_k = zlang_save_scope(emitter);
        _result_k = zlang_append_text(emitter, " else {\n") &&
                    zlang_emit_body(program, statement->else_start_, statement->else_count_,
                                    emitter, depth + 1U) &&
                    zlang_append_indent(emitter, depth) && zlang_append_text(emitter, "}\n");
        zlang_restore_scope(emitter, _else_scope_k);
    }
    return _result_k;
}

static bool zlang_emit_string_storage(const zlang_program_t *program,
                                      const zlang_ast_statement_t *statement,
                                      const zlang_emitted_symbol_t *symbol,
                                      zlang_emitter_t *emitter, uint32_t depth, bool is_static) {
    if (statement->capacity_ == UINT32_MAX) {
        return false;
    }
    char _capacity[ZLANG_EMITTED_NAME_CAPACITY];
    if (!zlang_capacity_text(emitter, statement, statement->capacity_, _capacity,
                             sizeof(_capacity))) {
        return false;
    }
    if ((is_static && !zlang_append_text(emitter, "static ")) ||
        !zlang_append_text(emitter, "char ") ||
        !zlang_append_text(emitter, symbol->storage_name_) || !zlang_append_text(emitter, "[") ||
        !zlang_append_text(emitter, _capacity) || !zlang_append_text(emitter, " + ZLANG_ONE") ||
        !zlang_append_text(emitter, "] = { ZLANG_ZERO };\n")) {
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
        (statement->is_const_ && !zlang_append_text(emitter, "const ")) ||
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
            !zlang_append_text(emitter, ", ZLANG_ZERO }")) {
            return false;
        }
    } else if (_is_concat_k && !zlang_is_literal_string(program, statement->expression_)) {
        if (!zlang_emit_string_concat_call(program, statement->expression_, symbol->storage_name_,
                                           _capacity, emitter)) {
            return false;
        }
    } else if (!zlang_emit_string_concat_call(program, statement->expression_,
                                              symbol->storage_name_, _capacity, emitter)) {
        return false;
    }
    return zlang_append_text(emitter, ";\n");
}

static bool zlang_expression_is_index_write_target(const zlang_program_t *program, size_t index) {
    for (size_t _owner = 0U; _owner < program->expression_count_; ++_owner) {
        const zlang_ast_expression_t *const _owner_k = &program->expressions_[_owner];
        if (_owner_k->kind_ == zlang_expression_index_assignment_ && _owner_k->left_ == index) {
            return true;
        }
    }
    return false;
}

static bool zlang_expression_is_write_target(const zlang_program_t *program, size_t index,
                                             const char *name) {
    for (size_t _candidate = 0U; _candidate < program->expression_count_; ++_candidate) {
        const zlang_ast_expression_t *const _expression_k = &program->expressions_[_candidate];
        if (_expression_k->kind_ == zlang_expression_assignment_ && _expression_k->left_ == index) {
            return true;
        }
        if (_expression_k->kind_ == zlang_expression_index_assignment_ &&
            _expression_k->left_ == index) {
            return true;
        }
        if (_candidate == index && _expression_k->kind_ == zlang_expression_index_ &&
            _expression_k->name_ != NULL && name != NULL &&
            strcmp(_expression_k->name_, name) == 0 &&
            zlang_expression_is_index_write_target(program, _candidate)) {
            return true;
        }
    }
    return false;
}

static bool zlang_name_is_read(const zlang_program_t *program, const char *name) {
    if (name == NULL) {
        return false;
    }
    for (size_t _index = 0U; _index < program->expression_count_; ++_index) {
        const zlang_ast_expression_t *const _expression_k = &program->expressions_[_index];
        const bool _is_field_k = _expression_k->kind_ == zlang_expression_field_ ||
                                 _expression_k->kind_ == zlang_expression_field_assignment_;
        const char *const _candidate_k =
            _is_field_k ? _expression_k->base_name_ : _expression_k->name_;
        if ((_expression_k->kind_ != zlang_expression_identifier_ &&
             _expression_k->kind_ != zlang_expression_index_ && !_is_field_k) ||
            _candidate_k == NULL || strcmp(_candidate_k, name) != 0) {
            continue;
        }
        if (_is_field_k || !zlang_expression_is_write_target(program, _index, name)) {
            return true;
        }
    }
    return false;
}

static bool zlang_append_unused_guard(const zlang_program_t *program, zlang_emitter_t *emitter,
                                      const char *source_name, const char *generated_name,
                                      uint32_t depth) {
    if (zlang_name_is_read(program, source_name)) {
        return true;
    }
    return zlang_append_indent(emitter, depth) && zlang_append_text(emitter, "(void)") &&
           zlang_append_text(emitter, generated_name) && zlang_append_text(emitter, ";\n");
}

static bool zlang_emit_unused_parameter_guards(const zlang_program_t *program,
                                               const zlang_ast_statement_t *statement,
                                               zlang_emitter_t *emitter, uint32_t depth) {
    for (size_t _index = 0U; _index < statement->parameter_count_; ++_index) {
        if (statement->parameter_start_ + _index >= program->parameter_count_) {
            return false;
        }
        const zlang_parameter_t *const _parameter_k =
            &program->parameters_[statement->parameter_start_ + _index];
        const zlang_emitted_symbol_t *const _symbol_k =
            zlang_find_active_symbol(emitter, _parameter_k->name_);
        if (_symbol_k == NULL || !zlang_append_unused_guard(program, emitter, _parameter_k->name_,
                                                            _symbol_k->generated_name_, depth)) {
            return false;
        }
    }
    return true;
}

static bool zlang_emit_for_initializer(const zlang_program_t *program,
                                       const zlang_ast_statement_t *statement,
                                       zlang_emitter_t *emitter) {
    if (statement->kind_ == zlang_statement_expression_) {
        return zlang_emit_expression(program, statement->expression_, emitter);
    }
    if (statement->kind_ != zlang_statement_declaration_) {
        return false;
    }
    char _name[ZLANG_EMITTED_NAME_CAPACITY];
    const bool _is_string_k = strcmp(statement->type_name_, "string") == 0;
    if (!zlang_register_symbol(emitter, statement->name_, statement->type_name_, "_",
                               statement->capacity_, statement->has_explicit_capacity_, false,
                               false, false, _name, sizeof(_name))) {
        return false;
    }
    if (_is_string_k && statement->has_explicit_capacity_) {
        return false;
    }
    const char *const _emitted_type_k =
        zlang_generated_type(program, emitter, statement->type_name_);
    if (_emitted_type_k == NULL ||
        (statement->is_const_ && !zlang_append_text(emitter, "const ")) ||
        !zlang_append_text(emitter, _emitted_type_k) || !zlang_append_text(emitter, " ") ||
        !zlang_append_text(emitter, _name) || !zlang_append_text(emitter, " = ")) {
        return false;
    }
    if (statement->expression_ != ZLANG_NO_EXPRESSION) {
        return zlang_emit_expression(program, statement->expression_, emitter);
    }
    return zlang_append_text(emitter, zlang_default_initializer(statement->type_name_));
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_statement(const zlang_program_t *program,
                                 const zlang_ast_statement_t *statement, zlang_emitter_t *emitter,
                                 uint32_t depth) { // NOLINT(misc-no-recursion)
    if (!zlang_emit_attached_comments(program, emitter, depth, statement->line_)) {
        return false;
    }
    const char *_written_name_k = NULL;
    bool _writes_k = false;
    if (statement->expression_ < program->expression_count_) {
        const zlang_ast_expression_t *const _expression_k =
            &program->expressions_[statement->expression_];
        _writes_k = _expression_k->kind_ == zlang_expression_index_assignment_ ||
                    _expression_k->kind_ == zlang_expression_field_assignment_ ||
                    _expression_k->kind_ == zlang_expression_append_call_;
        if ((_expression_k->kind_ == zlang_expression_assignment_ ||
             _expression_k->kind_ == zlang_expression_compound_assignment_) &&
            _expression_k->left_ < program->expression_count_ &&
            program->expressions_[_expression_k->left_].kind_ == zlang_expression_identifier_) {
            _written_name_k = program->expressions_[_expression_k->left_].name_;
            _writes_k = true;
        }
    }
    if (statement->update_expression_ < program->expression_count_) {
        const zlang_ast_expression_t *const _update_k =
            &program->expressions_[statement->update_expression_];
        if ((_update_k->kind_ == zlang_expression_assignment_ ||
             _update_k->kind_ == zlang_expression_compound_assignment_) &&
            _update_k->left_ < program->expression_count_ &&
            program->expressions_[_update_k->left_].kind_ == zlang_expression_identifier_) {
            _written_name_k = program->expressions_[_update_k->left_].name_;
            _writes_k = true;
        }
    }
    zlang_invalidate_pointer_cse(emitter, _written_name_k);
    if (_writes_k) {
        emitter->cse_count_ = 0U;
    }
    if (!zlang_materialize_field_references(program, statement, emitter, depth) ||
        !zlang_append_indent(emitter, depth)) {
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
        if (!zlang_register_symbol(emitter, statement->name_, statement->type_name_, "_",
                                   statement->capacity_, statement->has_explicit_capacity_,
                                   _needs_storage_k, false, false, _name, sizeof(_name))) {
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
        if (statement->is_const_ && !zlang_append_text(emitter, "const ")) {
            return false;
        }
        const char *const _emitted_type_k =
            zlang_generated_type(program, emitter, statement->type_name_);
        if (_emitted_type_k == NULL || !zlang_append_text(emitter, _emitted_type_k) ||
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
        } else if (zlang_find_record_statement(program, statement->type_name_) != NULL) {
            if (!zlang_append_text(emitter, "{ ZLANG_ZERO }")) {
                return false;
            }
        } else if (!zlang_append_text(emitter, zlang_default_initializer(statement->type_name_))) {
            return false;
        }
        if (!zlang_append_text(emitter, ";\n")) {
            return false;
        }
        return zlang_append_unused_guard(program, emitter, statement->name_, _name, depth);
    }
    case zlang_statement_sequence_declaration_: {
        char _name[ZLANG_EMITTED_NAME_CAPACITY];
        const bool _string_elements_k =
            statement->has_element_capacity_ && strcmp(statement->type_name_, "string") == 0;
        if (statement->capacity_ == 0U ||
            !zlang_register_symbol(emitter, statement->name_, statement->type_name_, "_",
                                   statement->capacity_, statement->has_explicit_capacity_,
                                   _string_elements_k, true, false, _name, sizeof(_name))) {
            return false;
        }
        const zlang_emitted_sequence_t *const _entry_k =
            zlang_find_sequence_entry(program, emitter, statement);
        if (_entry_k == NULL) {
            return false;
        }
        if (zlang_is_static(emitter, statement) && !zlang_append_text(emitter, "static ")) {
            return false;
        }
        if (!zlang_append_text(emitter, _entry_k->type_name_) || !zlang_append_text(emitter, " ") ||
            !zlang_append_text(emitter, _name) || !zlang_append_text(emitter, " = ")) {
            return false;
        }
        if (_string_elements_k) {
            if (!zlang_append_text(emitter, "{ ZLANG_ZERO };\n") ||
                !zlang_append_unused_guard(program, emitter, statement->name_, _name, depth)) {
                return false;
            }
            if (statement->initializer_count_ == 0U) {
                return true;
            }
        } else if (zlang_find_record_statement(program, statement->type_name_) != NULL) {
            if (!zlang_append_text(emitter, "{ { ")) {
                return false;
            }
            for (uint32_t _item = 0U; _item < statement->capacity_; ++_item) {
                if ((_item > 0U && !zlang_append_text(emitter, ", ")) ||
                    !zlang_append_text(emitter, "{ ZLANG_ZERO }")) {
                    return false;
                }
            }
            if (!zlang_append_text(emitter, " }, ") ||
                !zlang_append_text(emitter, _entry_k->length_macro_) ||
                !zlang_append_text(emitter, " };\n")) {
                return false;
            }
            return zlang_append_unused_guard(program, emitter, statement->name_, _name, depth);
        } else {
            if (!zlang_append_text(emitter, "{ { ") ||
                !zlang_emit_sequence_values(program, statement, emitter) ||
                !zlang_append_text(emitter, " }, ") ||
                !zlang_append_text(emitter, _entry_k->length_macro_) ||
                !zlang_append_text(emitter, " };\n")) {
                return false;
            }
            return zlang_append_unused_guard(program, emitter, statement->name_, _name, depth);
        }
        if (zlang_is_static(emitter, statement)) {
            return false;
        }
        const zlang_emitted_symbol_t *const _symbol_k =
            zlang_find_active_symbol(emitter, statement->name_);
        if (_symbol_k == NULL || _symbol_k->storage_name_[0] == '\0') {
            return false;
        }
        char _element_capacity[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_sequence_element_capacity_text(program, emitter, statement, _element_capacity,
                                                  sizeof(_element_capacity))) {
            return false;
        }
        for (size_t _index = 0U; _index < statement->initializer_count_; ++_index) {
            char _slot[ZLANG_EMITTED_NAME_CAPACITY + 32];
            const int _written_k = snprintf(_slot, sizeof(_slot), "%s.storage_[%lu]",
                                            _symbol_k->generated_name_, (unsigned long)_index);
            if (_written_k < 0 || (size_t)_written_k >= sizeof(_slot) ||
                !zlang_append_indent(emitter, depth) ||
                !zlang_append_text(emitter, _symbol_k->generated_name_) ||
                !zlang_append_text(emitter, ".data_[") ||
                !zlang_append_unsigned(emitter, (uint32_t)_index) ||
                !zlang_append_text(emitter, "] = ") ||
                !zlang_emit_string_concat_call(program, statement->initializer_start_ + _index,
                                               _slot, _element_capacity, emitter) ||
                !zlang_append_text(emitter, ";\n")) {
                return false;
            }
        }
        return zlang_append_indent(emitter, depth) &&
               zlang_append_text(emitter, _symbol_k->generated_name_) &&
               zlang_append_text(emitter, ".length_ = ") &&
               zlang_append_text(emitter, _entry_k->length_macro_) &&
               zlang_append_text(emitter, ";\n");
    }
    case zlang_statement_expression_: {
        const zlang_ast_expression_t *const _expression_k =
            statement->expression_ < program->expression_count_
                ? &program->expressions_[statement->expression_]
                : NULL;
        if (_expression_k != NULL && _expression_k->kind_ == zlang_expression_index_assignment_) {
            return zlang_emit_index_assignment(program, _expression_k, emitter, depth);
        }
        if (_expression_k != NULL && _expression_k->kind_ == zlang_expression_field_assignment_ &&
            _expression_k->left_ < program->expression_count_ &&
            program->expressions_[_expression_k->left_].base_name_ != NULL &&
            program->expressions_[_expression_k->left_].right_ != ZLANG_NO_EXPRESSION) {
            return zlang_emit_field_assignment(program, _expression_k, emitter);
        }
        if (!zlang_emit_expression(program, statement->expression_, emitter)) {
            return false;
        }
        return zlang_append_text(emitter, ";\n");
    }
    case zlang_statement_break_:
        return zlang_append_text(emitter, "break;\n");
    case zlang_statement_continue_:
        return zlang_append_text(emitter, "continue;\n");
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
        const zlang_emitter_scope_t _statement_scope_k = zlang_save_scope(emitter);
        const bool _result_k = zlang_emit_if_statement(program, statement, emitter, depth, true);
        zlang_restore_scope(emitter, _statement_scope_k);
        return _result_k;
    }
    case zlang_statement_while_: {
        if (!zlang_append_text(emitter, "while ") ||
            !zlang_emit_condition(program, statement->expression_, emitter) ||
            !zlang_append_text(emitter, " {\n")) {
            return false;
        }
        const zlang_emitter_scope_t _while_scope_k = zlang_save_scope(emitter);
        const bool _result_k = zlang_emit_body(program, statement->body_start_,
                                               statement->body_count_, emitter, depth + 1U) &&
                               zlang_append_indent(emitter, depth) &&
                               zlang_append_text(emitter, "}\n");
        zlang_restore_scope(emitter, _while_scope_k);
        return _result_k;
    }
    case zlang_statement_for_: {
        if (!zlang_append_text(emitter, "for (")) {
            return false;
        }
        const zlang_emitter_scope_t _for_scope_k = zlang_save_scope(emitter);
        if (statement->initializer_statement_ != ZLANG_NO_STATEMENT &&
            statement->initializer_statement_ < program->statement_count_ &&
            !zlang_emit_for_initializer(
                program, &program->statements_[statement->initializer_statement_], emitter)) {
            return false;
        }
        const bool _result_k =
            zlang_append_text(emitter, "; ") &&
            zlang_emit_condition(program, statement->expression_, emitter) &&
            zlang_append_text(emitter, "; ") &&
            zlang_emit_expression(program, statement->update_expression_, emitter) &&
            zlang_append_text(emitter, ") {\n") &&
            zlang_emit_body(program, statement->body_start_, statement->body_count_, emitter,
                            depth + 1U) &&
            zlang_append_indent(emitter, depth) && zlang_append_text(emitter, "}\n");
        zlang_restore_scope(emitter, _for_scope_k);
        return _result_k;
    }
    case zlang_statement_repeat_: {
        char _repeat_name[32];
        const int _repeat_written_k =
            snprintf(_repeat_name, sizeof(_repeat_name), "_zlang_repeat_%lu",
                     (unsigned long)emitter->next_index_++);
        if (_repeat_written_k < 0 || (size_t)_repeat_written_k >= sizeof(_repeat_name) ||
            !zlang_append_text(emitter, "for (uint32_t ") ||
            !zlang_append_text(emitter, _repeat_name) ||
            !zlang_append_text(emitter, " = ZLANG_ZERO; ") ||
            !zlang_append_text(emitter, _repeat_name) || !zlang_append_text(emitter, " < ") ||
            !zlang_emit_expression(program, statement->expression_, emitter) ||
            !zlang_append_text(emitter, "; ++") || !zlang_append_text(emitter, _repeat_name) ||
            !zlang_append_text(emitter, ") {\n")) {
            return false;
        }
        const zlang_emitter_scope_t _repeat_scope_k = zlang_save_scope(emitter);
        const bool _result_k = zlang_emit_body(program, statement->body_start_,
                                               statement->body_count_, emitter, depth + 1U) &&
                               zlang_append_indent(emitter, depth) &&
                               zlang_append_text(emitter, "}\n");
        zlang_restore_scope(emitter, _repeat_scope_k);
        return _result_k;
    }
    case zlang_statement_foreach_: {
        const zlang_emitter_scope_t _foreach_scope_k = zlang_save_scope(emitter);
        char _loop_index[32];
        const int _loop_written_k = snprintf(_loop_index, sizeof(_loop_index), "_zlang_foreach_%lu",
                                             (unsigned long)emitter->next_index_++);
        char _item_name[ZLANG_EMITTED_NAME_CAPACITY];
        char _user_index_name[ZLANG_EMITTED_NAME_CAPACITY];
        const char *const _iterable_k = zlang_resolve_name(emitter, statement->iterable_name_);
        if (_loop_written_k < 0 || (size_t)_loop_written_k >= sizeof(_loop_index) ||
            (statement->index_name_ != NULL &&
             !zlang_register_symbol(emitter, statement->index_name_, "unsigned", "_", 0U, false,
                                    false, false, false, _user_index_name,
                                    sizeof(_user_index_name))) ||
            !zlang_register_symbol(emitter, statement->name_, statement->type_name_, "_", 0U, false,
                                   false, false, false, _item_name, sizeof(_item_name))) {
            return false;
        }
        if (!zlang_append_text(emitter, "for (uint32_t ") ||
            !zlang_append_text(emitter, _loop_index) ||
            !zlang_append_text(emitter, " = ZLANG_ZERO; ") ||
            !zlang_append_text(emitter, _loop_index) || !zlang_append_text(emitter, " < ") ||
            !zlang_append_text(emitter, _iterable_k) ||
            !zlang_append_text(emitter, ".length_; ++") ||
            !zlang_append_text(emitter, _loop_index) || !zlang_append_text(emitter, ") {\n")) {
            return false;
        }
        if (statement->index_name_ != NULL) {
            if (!zlang_append_indent(emitter, depth + 1U) ||
                !zlang_append_text(emitter, "uint32_t ") ||
                !zlang_append_text(emitter, _user_index_name) ||
                !zlang_append_text(emitter, " = ") || !zlang_append_text(emitter, _loop_index) ||
                !zlang_append_text(emitter, ";\n") ||
                !zlang_append_unused_guard(program, emitter, statement->index_name_,
                                           _user_index_name, depth + 1U)) {
                return false;
            }
        }
        const char *const _emitted_type_k =
            zlang_generated_type(program, emitter, statement->type_name_);
        if (_emitted_type_k == NULL || !zlang_append_indent(emitter, depth + 1U) ||
            !zlang_append_text(emitter, _emitted_type_k) || !zlang_append_text(emitter, " ") ||
            !zlang_append_text(emitter, _item_name) || !zlang_append_text(emitter, " = ") ||
            !zlang_append_text(emitter, _iterable_k) || !zlang_append_text(emitter, ".data_[") ||
            !zlang_append_text(emitter, _loop_index) || !zlang_append_text(emitter, "];\n") ||
            !zlang_append_unused_guard(program, emitter, statement->name_, _item_name,
                                       depth + 1U)) {
            return false;
        }
        const bool _result_k = zlang_emit_body(program, statement->body_start_,
                                               statement->body_count_, emitter, depth + 1U) &&
                               zlang_append_indent(emitter, depth) &&
                               zlang_append_text(emitter, "}\n");
        zlang_restore_scope(emitter, _foreach_scope_k);
        return _result_k;
    }
    case zlang_statement_function_: {
        char _name[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_register_symbol(emitter, statement->name_, statement->type_name_, "_u_", 0U,
                                   false, false, false, false, _name, sizeof(_name))) {
            return false;
        }
        const zlang_emitter_scope_t _function_scope_k = zlang_save_scope(emitter);
        const char *const _return_type_k =
            zlang_generated_type(program, emitter, statement->type_name_);
        const bool _has_parameters_k = statement->parameter_count_ > 0U;
        if (_return_type_k == NULL || !zlang_append_text(emitter, _return_type_k) ||
            !zlang_append_text(emitter, " ") || !zlang_append_text(emitter, _name) ||
            !zlang_append_text(emitter, "(") ||
            (!_has_parameters_k && !zlang_append_text(emitter, "void"))) {
            return false;
        }
        for (size_t _index = 0U; _index < statement->parameter_count_; ++_index) {
            const zlang_parameter_t *const _parameter_k =
                &program->parameters_[statement->parameter_start_ + _index];
            char _parameter_name[ZLANG_EMITTED_NAME_CAPACITY];
            if (!zlang_register_symbol(emitter, _parameter_k->name_, _parameter_k->type_name_, "",
                                       0U, false, false, false, _parameter_k->is_reference_,
                                       _parameter_name, sizeof(_parameter_name))) {
                return false;
            }
            const char *const _parameter_type_k =
                zlang_generated_type(program, emitter, _parameter_k->type_name_);
            if (_parameter_type_k == NULL || (_index > 0U && !zlang_append_text(emitter, ", ")) ||
                (_parameter_k->is_const_ && !zlang_append_text(emitter, "const ")) ||
                !zlang_append_text(emitter, _parameter_type_k) ||
                (_parameter_k->is_reference_ && !zlang_append_text(emitter, " *")) ||
                (!_parameter_k->is_reference_ && !zlang_append_text(emitter, " ")) ||
                !zlang_append_text(emitter, _parameter_name)) {
                return false;
            }
        }
        const bool _result_k =
            zlang_append_text(emitter, ") {\n") &&
            zlang_emit_unused_parameter_guards(program, statement, emitter, depth + 1U) &&
            zlang_emit_body(program, statement->body_start_, statement->body_count_, emitter,
                            depth + 1U) &&
            zlang_append_indent(emitter, depth) && zlang_append_text(emitter, "}\n\n");
        zlang_restore_scope(emitter, _function_scope_k);
        return _result_k;
    }
    case zlang_statement_block_: {
        const zlang_emitter_scope_t _block_scope_k = zlang_save_scope(emitter);
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

static bool zlang_emit_common_constants(zlang_emitter_t *emitter) {
    return zlang_append_text(emitter, "/* generated value constants */\n"
                                      "#define ZLANG_ZERO 0U\n"
                                      "#define ZLANG_ONE 1U\n"
                                      "#define ZLANG_TWO 2U\n\n");
}

static bool zlang_append_part_count(zlang_emitter_t *emitter, size_t count) {
    if (count == 1U) {
        return zlang_append_text(emitter, "ZLANG_ONE");
    }
    if (count == 2U) {
        return zlang_append_text(emitter, "ZLANG_TWO");
    }
    return zlang_append_unsigned(emitter, (uint32_t)count) && zlang_append_text(emitter, "U");
}

static const char *zlang_find_cse(const zlang_emitter_t *emitter, size_t expression_index) {
    if (emitter == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < emitter->cse_count_; ++_index) {
        if (emitter->cse_expression_indices_[_index] == expression_index) {
            return emitter->cse_names_[_index];
        }
    }
    return NULL;
}

static const char *zlang_find_pointer_cse(const zlang_emitter_t *emitter, size_t expression_index) {
    if (emitter == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < emitter->cse_pointer_count_; ++_index) {
        if (emitter->cse_pointer_indices_[_index] == expression_index) {
            return emitter->cse_pointer_names_[_index];
        }
    }
    return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
static const char *zlang_first_identifier_name(const zlang_program_t *program, size_t index) {
    if (index == ZLANG_NO_EXPRESSION || index >= program->expression_count_) {
        return NULL;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ == zlang_expression_identifier_ && _expression_k->name_ != NULL) {
        return _expression_k->name_;
    }
    const char *const _left_k = zlang_first_identifier_name(program, _expression_k->left_);
    if (_left_k != NULL) {
        return _left_k;
    }
    const char *const _right_k = zlang_first_identifier_name(program, _expression_k->right_);
    if (_right_k != NULL) {
        return _right_k;
    }
    for (size_t _argument = 0U;
         _argument < _expression_k->argument_count_ && _argument < ZLANG_CALL_ARGUMENT_CAPACITY;
         ++_argument) {
        const char *const _argument_k =
            zlang_first_identifier_name(program, _expression_k->argument_indices_[_argument]);
        if (_argument_k != NULL) {
            return _argument_k;
        }
    }
    return NULL;
}

static void zlang_invalidate_pointer_cse(zlang_emitter_t *emitter, const char *name) {
    if (emitter == NULL || name == NULL) {
        return;
    }
    size_t _kept_k = 0U;
    for (size_t _index = 0U; _index < emitter->cse_pointer_count_; ++_index) {
        if (zlang_same_text(emitter->cse_pointer_base_names_[_index], name) ||
            zlang_same_text(emitter->cse_pointer_dependency_names_[_index], name)) {
            continue;
        }
        if (_kept_k != _index) {
            emitter->cse_pointer_indices_[_kept_k] = emitter->cse_pointer_indices_[_index];
            (void)zlang_copy_name(emitter->cse_pointer_names_[_index],
                                  emitter->cse_pointer_names_[_kept_k],
                                  sizeof(emitter->cse_pointer_names_[_kept_k]));
        }
        emitter->cse_pointer_base_names_[_kept_k] = emitter->cse_pointer_base_names_[_index];
        emitter->cse_pointer_dependency_names_[_kept_k] =
            emitter->cse_pointer_dependency_names_[_index];
        ++_kept_k;
    }
    emitter->cse_pointer_count_ = _kept_k;
}

static bool zlang_materialize_field_references(const zlang_program_t *program,
                                               const zlang_ast_statement_t *statement,
                                               zlang_emitter_t *emitter, uint32_t depth) {
    if (statement == NULL) {
        return true;
    }
    size_t _candidates[64];
    size_t _candidate_count_k = 0U;
    if (!zlang_collect_statement_field_references(program, statement, _candidates,
                                                  &_candidate_count_k,
                                                  sizeof(_candidates) / sizeof(_candidates[0]))) {
        return false;
    }
    for (size_t _candidate = 0U; _candidate < _candidate_count_k; ++_candidate) {
        const size_t _index_k = _candidates[_candidate];
        if (zlang_find_pointer_cse(emitter, _index_k) != NULL) {
            continue;
        }
        const zlang_ast_expression_t *const _field_k = &program->expressions_[_index_k];
        const zlang_emitted_symbol_t *const _symbol_k =
            zlang_find_active_symbol(emitter, _field_k->base_name_);
        const char *const _type_k =
            _symbol_k != NULL ? zlang_generated_type(program, emitter, _symbol_k->type_name_)
                              : NULL;
        if (_type_k == NULL) {
            return false;
        }
        char _pointer_name[32];
        const int _written_k =
            snprintf(_pointer_name, sizeof(_pointer_name), "_zlang_field_ptr_%lu",
                     (unsigned long)emitter->next_index_++);
        if (_written_k < 0 || (size_t)_written_k >= sizeof(_pointer_name) ||
            emitter->cse_pointer_count_ >= ZLANG_EMITTED_CSE_CAPACITY) {
            return false;
        }
        if (!zlang_append_indent(emitter, depth) || !zlang_append_text(emitter, _type_k) ||
            !zlang_append_text(emitter, " *") || !zlang_append_text(emitter, _pointer_name) ||
            !zlang_append_text(emitter, " = (") || !zlang_append_text(emitter, _type_k) ||
            !zlang_append_text(emitter, " *)zlang_array_get(") ||
            !zlang_append_text(emitter, _symbol_k->generated_name_) ||
            !zlang_append_text(emitter, ".data_, ") ||
            !zlang_emit_expression(program, _field_k->right_, emitter) ||
            !zlang_append_text(emitter, ", ") ||
            !zlang_append_text(emitter, _symbol_k->generated_name_) ||
            !zlang_append_text(emitter, ".length_, sizeof(") ||
            !zlang_append_text(emitter, _symbol_k->generated_name_) ||
            !zlang_append_text(emitter, ".data_[0]));\n")) {
            return false;
        }
        for (size_t _other = 0U; _other < program->expression_count_; ++_other) {
            const zlang_ast_expression_t *const _candidate_k = &program->expressions_[_other];
            if (_candidate_k->kind_ != zlang_expression_field_ ||
                !zlang_same_text(_candidate_k->base_name_, _field_k->base_name_) ||
                !zlang_expression_equal(program, _candidate_k->right_, _field_k->right_) ||
                zlang_find_pointer_cse(emitter, _other) != NULL) {
                continue;
            }
            emitter->cse_pointer_indices_[emitter->cse_pointer_count_] = _other;
            snprintf(emitter->cse_pointer_names_[emitter->cse_pointer_count_],
                     sizeof(emitter->cse_pointer_names_[0]), "%s", _pointer_name);
            emitter->cse_pointer_base_names_[emitter->cse_pointer_count_] = _field_k->base_name_;
            emitter->cse_pointer_dependency_names_[emitter->cse_pointer_count_] =
                zlang_first_identifier_name(program, _field_k->right_);
            ++emitter->cse_pointer_count_;
        }
    }
    return true;
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

static bool zlang_emit_generated_declarations(const zlang_program_t *program,
                                              zlang_emitter_t *emitter) {
    if (emitter->emitted_user_type_count_ > 0U) {
        if (!zlang_append_text(emitter, "/* generated user types */\n")) {
            return false;
        }
        for (size_t _index = 0U; _index < emitter->emitted_user_type_count_; ++_index) {
            const zlang_emitted_user_type_t *const _entry_k = &emitter->emitted_user_types_[_index];
            const zlang_ast_statement_t *const _statement_k = _entry_k->statement_;
            if (!zlang_emit_attached_comments(program, emitter, 0U, _statement_k->line_) ||
                !zlang_append_text(emitter, _statement_k->kind_ == zlang_statement_enum_
                                                ? "typedef enum "
                                                : "typedef struct ") ||
                !zlang_append_text(emitter, _entry_k->type_name_) ||
                !zlang_append_text(emitter, " {\n")) {
                return false;
            }
            if (_statement_k->kind_ == zlang_statement_enum_) {
                const size_t _end_k =
                    _statement_k->initializer_start_ + _statement_k->initializer_count_;
                for (size_t _item = _statement_k->initializer_start_;
                     _item < _end_k && _item < program->expression_count_; ++_item) {
                    const zlang_ast_expression_t *const _item_k = &program->expressions_[_item];
                    const char *const _generated_k =
                        zlang_generated_enum_item(emitter, _item_k->name_);
                    if (!zlang_emit_attached_comments(program, emitter, 1U, _item_k->line_) ||
                        _generated_k == NULL || !zlang_append_text(emitter, "    ") ||
                        !zlang_append_text(emitter, _generated_k) ||
                        !zlang_append_text(emitter, " = ") ||
                        !zlang_append_unsigned(emitter, (uint32_t)_item_k->integer_value_) ||
                        !zlang_append_text(emitter, "U,\n")) {
                        return false;
                    }
                }
            } else {
                const size_t _end_k = _statement_k->body_start_ + _statement_k->body_count_;
                for (size_t _field = _statement_k->body_start_;
                     _field < _end_k && _field < program->statement_count_; ++_field) {
                    const zlang_ast_statement_t *const _field_k = &program->statements_[_field];
                    const char *const _type_k =
                        zlang_generated_type(program, emitter, _field_k->type_name_);
                    if (!zlang_emit_attached_comments(program, emitter, 1U, _field_k->line_) ||
                        _type_k == NULL || !zlang_append_text(emitter, "    ") ||
                        !zlang_append_text(emitter, _type_k) || !zlang_append_text(emitter, " ") ||
                        !zlang_append_field_name(emitter, _field_k->name_) ||
                        !zlang_append_text(emitter, ";\n")) {
                        return false;
                    }
                }
            }
            if (!zlang_append_text(emitter, "} ") ||
                !zlang_append_text(emitter, _entry_k->type_name_) ||
                !zlang_append_text(emitter, ";\n\n")) {
                return false;
            }
        }
    }
    if (emitter->emitted_sequence_count_ > 0U) {
        if (!zlang_append_text(emitter, "/* generated capacities */\n")) {
            return false;
        }
        for (size_t _index = 0U; _index < emitter->emitted_sequence_count_; ++_index) {
            const zlang_emitted_sequence_t *const _entry_k = &emitter->emitted_sequences_[_index];
            if (!zlang_append_text(emitter, "#define ") ||
                !zlang_append_text(emitter, _entry_k->capacity_macro_) ||
                !zlang_append_text(emitter, " ") ||
                !zlang_append_unsigned(emitter, _entry_k->statement_->capacity_) ||
                !zlang_append_text(emitter, "U\n")) {
                return false;
            }
            if (!zlang_append_text(emitter, "#define ") ||
                !zlang_append_text(emitter, _entry_k->length_macro_) ||
                !zlang_append_text(emitter, " ") ||
                !zlang_append_unsigned(emitter,
                                       (uint32_t)_entry_k->statement_->initializer_count_) ||
                !zlang_append_text(emitter, "U\n")) {
                return false;
            }
            if (_entry_k->statement_->has_element_capacity_ &&
                (!zlang_append_text(emitter, "#define ") ||
                 !zlang_append_text(emitter, _entry_k->element_macro_) ||
                 !zlang_append_text(emitter, " ") ||
                 !zlang_append_unsigned(emitter, _entry_k->statement_->element_capacity_) ||
                 !zlang_append_text(emitter, "U\n"))) {
                return false;
            }
        }
        if (emitter->emitted_capacity_count_ > 0U) {
            for (size_t _index = 0U; _index < emitter->emitted_capacity_count_; ++_index) {
                const zlang_emitted_capacity_t *const _entry_k =
                    &emitter->emitted_capacities_[_index];
                const zlang_ast_statement_t *const _statement_k = _entry_k->statement_;
                if (!zlang_append_text(emitter, "#define ") ||
                    !zlang_append_text(emitter, _entry_k->macro_) ||
                    !zlang_append_text(emitter, " ") ||
                    !zlang_append_unsigned(emitter, _statement_k->capacity_) ||
                    !zlang_append_text(emitter, "U\n")) {
                    return false;
                }
            }
        }
        if (!zlang_append_text(emitter, "\n/* generated sequence types */\n")) {
            return false;
        }
        for (size_t _index = 0U; _index < emitter->emitted_sequence_count_; ++_index) {
            const zlang_emitted_sequence_t *const _entry_k = &emitter->emitted_sequences_[_index];
            const zlang_ast_statement_t *const _statement_k = _entry_k->statement_;
            if (!zlang_append_text(emitter, "typedef struct ") ||
                !zlang_append_text(emitter, _entry_k->type_name_) ||
                !zlang_append_text(emitter, " {\n")) {
                return false;
            }
            if (_statement_k->has_element_capacity_) {
                if (!zlang_append_text(emitter, "    char storage_[") ||
                    !zlang_append_text(emitter, _entry_k->capacity_macro_) ||
                    !zlang_append_text(emitter, "][") ||
                    !zlang_append_text(emitter, _entry_k->element_macro_) ||
                    !zlang_append_text(emitter, " + ZLANG_ONE];\n    zlang_string_t data_[") ||
                    !zlang_append_text(emitter, _entry_k->capacity_macro_) ||
                    !zlang_append_text(emitter, "];\n")) {
                    return false;
                }
            } else {
                const char *const _element_type_k =
                    zlang_generated_type(program, emitter, _statement_k->type_name_);
                if (_element_type_k == NULL || !zlang_append_text(emitter, "    ") ||
                    !zlang_append_text(emitter, _element_type_k) ||
                    !zlang_append_text(emitter, " data_[") ||
                    !zlang_append_text(emitter, _entry_k->capacity_macro_) ||
                    !zlang_append_text(emitter, "];\n")) {
                    return false;
                }
            }
            if (!zlang_append_text(emitter, "    uint32_t length_;\n} ") ||
                !zlang_append_text(emitter, _entry_k->type_name_) ||
                !zlang_append_text(emitter, ";\n\n")) {
                return false;
            }
        }
    }
    return zlang_emit_literal_declarations(emitter, program);
}

static bool zlang_is_type_definition(const zlang_ast_statement_t *statement) {
    return statement->kind_ == zlang_statement_record_ || statement->kind_ == zlang_statement_enum_;
}

static bool zlang_is_file_scope(const zlang_emitter_t *emitter,
                                const zlang_ast_statement_t *statement) {
    return statement->kind_ == zlang_statement_function_ ||
           ((statement->kind_ == zlang_statement_declaration_ ||
             statement->kind_ == zlang_statement_sequence_declaration_) &&
            zlang_is_static(emitter, statement));
}

bool zlang_emit_program(const zlang_program_t *program, zlang_emitter_t *emitter) {
    if (emitter->plan_ == NULL || !zlang_emit_prologue(emitter) ||
        !zlang_emit_common_constants(emitter) || !zlang_collect_user_types(emitter, program) ||
        !zlang_collect_sequences(emitter, program) || !zlang_collect_capacities(emitter, program) ||
        !zlang_collect_literals(emitter, program) ||
        !zlang_emit_generated_declarations(program, emitter)) {
        return false;
    }

    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (zlang_is_nested(program, _index) || zlang_is_type_definition(_statement_k)) {
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
    bool _has_previous_k = false;
    bool _previous_grouped_k = false;
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (zlang_is_nested(program, _index) || zlang_is_type_definition(_statement_k) ||
            zlang_is_file_scope(emitter, _statement_k)) {
            continue;
        }
        const bool _grouped_k = zlang_is_grouped_statement(program, _statement_k);
        if (zlang_should_insert_blank_line(_has_previous_k, _previous_grouped_k, _grouped_k,
                                           _statement_k->kind_) &&
            !zlang_append_text(emitter, "\n")) {
            return false;
        }
        if (!zlang_emit_statement(program, _statement_k, emitter, 1U)) {
            return false;
        }
        _previous_grouped_k = _grouped_k;
        _has_previous_k = true;
    }
    if (!zlang_emit_remaining_comments(program, emitter, 1U)) {
        return false;
    }
    if (_has_previous_k && !zlang_append_text(emitter, "\n")) {
        return false;
    }
    return zlang_append_text(emitter, "    return ZLANG_ZERO;\n}\n");
}
