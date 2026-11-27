/** @file
 * @brief C99 emitter for the Zlang abstract syntax tree.
 */

#include "zlang/compiler/emitter.h"
#include "zlang/compiler/generics.h"
#include "zlang/compiler/stdlib.h"

#include <stdio.h>
#include <string.h>

static const zlang_ast_statement_t *zlang_find_trait_statement(const zlang_program_t *program,
                                                               const char *name);
static bool zlang_statement_in_method(const zlang_program_t *program,
                                      const zlang_ast_statement_t *structure, size_t index);
static bool zlang_struct_is_polymorphic(const zlang_program_t *program, const char *name);
static bool zlang_vtable_type_name(const zlang_program_t *program, zlang_emitter_t *emitter,
                                   const char *struct_name, char *output, size_t capacity);
static bool zlang_vtable_instance_name(zlang_emitter_t *emitter, const char *struct_name,
                                       char *output, size_t capacity);
static bool zlang_type_tag_name(zlang_emitter_t *emitter, const char *struct_name, char *output,
                                size_t capacity);
static bool zlang_is_struct_ancestor(const zlang_program_t *program, const char *ancestor,
                                     const char *descendant);
static uint32_t zlang_struct_depth(const zlang_program_t *program, const char *ancestor,
                                   const char *descendant);
static const zlang_ast_statement_t *zlang_find_field_owner(const zlang_program_t *program,
                                                           const zlang_ast_statement_t *structure,
                                                           const char *name, uint32_t *depth,
                                                           uint32_t limit);
static const zlang_ast_statement_t *zlang_find_method(const zlang_program_t *program,
                                                      const char *struct_name, const char *name,
                                                      const char **owner_struct_name,
                                                      bool *is_trait);
static const zlang_ast_statement_t *zlang_struct_root_name(const zlang_program_t *program,
                                                           const char *name);
static bool zlang_method_is_virtual(const zlang_program_t *program, const char *struct_name,
                                    const char *name);
static bool zlang_vtable_wrapper_name(const char *struct_name, const char *method_name,
                                      char *output, size_t capacity);
static bool zlang_is_vtable_slot(const zlang_program_t *program, const char *root_name,
                                 size_t method_index);
static const zlang_ast_statement_t *zlang_effective_method(const zlang_program_t *program,
                                                           const char *struct_name,
                                                           const char *method_name);
static bool zlang_emit_base_chain(zlang_emitter_t *emitter, uint32_t depth, bool is_reference);
// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_expression_as(const zlang_program_t *program, size_t index,
                                     zlang_emitter_t *emitter, const char *expected_type,
                                     bool is_reference);
static bool zlang_emit_vtable_thunk_prototypes(const zlang_program_t *program,
                                               zlang_emitter_t *emitter);
static bool zlang_emit_vtable_thunk_definitions(const zlang_program_t *program,
                                                zlang_emitter_t *emitter);

static const char *zlang_std_domain(const char *specifier) {
    if (specifier == NULL) {
        return NULL;
    }
    const char *const _prefix_k = "@std/";
    const size_t _prefix_length_k = strlen(_prefix_k);
    return strncmp(specifier, _prefix_k, _prefix_length_k) == 0 ? specifier + _prefix_length_k
                                                                : specifier;
}

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
    memset(emitter->emitted_user_results_, 0, sizeof(emitter->emitted_user_results_));
    emitter->emitted_user_result_count_ = 0U;
    emitter->emitted_enum_item_count_ = 0U;
    emitter->cse_count_ = 0U;
    emitter->cse_pointer_count_ = 0U;
    emitter->emit_comments_ = true;
    emitter->emit_timing_ = false;
    emitter->emit_dump_ = false;
    emitter->force_timing_ = false;
    emitter->force_dump_ = false;
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
    memset(emitter->requested_array_helpers_, 0, sizeof(emitter->requested_array_helpers_));
    emitter->requested_array_helper_count_ = 0U;
    emitter->collecting_array_helpers_ = false;
    memset(emitter->emitted_functions_, 0, sizeof(emitter->emitted_functions_));
    emitter->emitted_function_count_ = 0U;
    memset(emitter->array_specializations_, 0, sizeof(emitter->array_specializations_));
    emitter->array_specialization_count_ = 0U;
    emitter->active_specialization_ = NULL;
    emitter->uses_stateful_std_ = false;
    emitter->activated_macro_count_ = 0U;
    emitter->in_function_ = false;
    emitter->has_explicit_main_ = false;
    emitter->program_ = NULL;
    emitter->current_struct_ = NULL;
    emitter->current_return_type_ = NULL;
    emitter->current_self_name_ = NULL;
    emitter->current_self_is_pointer_ = false;
    (void)zlang_find_trait_statement;
    (void)zlang_struct_is_polymorphic;
    (void)zlang_vtable_type_name;
    (void)zlang_vtable_instance_name;
    (void)zlang_type_tag_name;
    (void)zlang_is_struct_ancestor;
    (void)zlang_struct_depth;
    (void)zlang_find_field_owner;
    (void)zlang_find_method;
    (void)zlang_method_is_virtual;
    (void)zlang_vtable_wrapper_name;
    (void)zlang_is_vtable_slot;
    (void)zlang_effective_method;
    (void)zlang_emit_base_chain;
    (void)zlang_emit_expression_as;
    (void)zlang_emit_vtable_thunk_prototypes;
    (void)zlang_emit_vtable_thunk_definitions;
    for (size_t _index = 0U; _index < ZLANG_EMITTED_CSE_CAPACITY; ++_index) {
        emitter->cse_pointer_base_names_[_index] = NULL;
        emitter->cse_pointer_dependency_names_[_index] = NULL;
    }
    if (capacity > 0U) {
        buffer[0] = '\0';
    }
}

static bool zlang_append_text(zlang_emitter_t *emitter, const char *text) {
    if (emitter->collecting_array_helpers_) {
        return true;
    }
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

static bool zlang_append_u64(zlang_emitter_t *emitter, uint64_t value) {
    char _digits[24];
    const int _written_k = snprintf(_digits, sizeof(_digits), "%llu", (unsigned long long)value);
    return _written_k >= 0 && (size_t)_written_k < sizeof(_digits) &&
           zlang_append_text(emitter, _digits);
}

static const char *zlang_find_cse(const zlang_emitter_t *emitter, size_t expression_index);

static const char *zlang_find_pointer_cse(const zlang_emitter_t *emitter, size_t expression_index);

static bool zlang_materialize_field_references(const zlang_program_t *program,
                                               const zlang_ast_statement_t *statement,
                                               zlang_emitter_t *emitter, uint32_t depth);

static void zlang_invalidate_pointer_cse(zlang_emitter_t *emitter, const char *name);

static bool zlang_expression_equal(const zlang_program_t *program, size_t left, size_t right);

// NOLINTNEXTLINE(misc-no-recursion)
static const zlang_std_function_t *zlang_std_call(const zlang_program_t *program,
                                                  const zlang_emitter_t *emitter,
                                                  const zlang_ast_expression_t *call);

static const zlang_ast_statement_t *
zlang_find_field_statement(const zlang_program_t *program, const zlang_ast_statement_t *structure,
                           const char *name);

static bool zlang_emit_expression(const zlang_program_t *program, size_t index,
                                  zlang_emitter_t *emitter);

static bool zlang_emit_index_expression(const zlang_program_t *program, size_t index,
                                        zlang_emitter_t *emitter);

static bool zlang_array_helper_name(const char *prefix, uint32_t rank, const char *suffix,
                                    char *output, size_t capacity);

static void zlang_struct_array_helper(zlang_emitter_t *emitter, const char *name,
                                      const char *type_name, uint32_t rank, bool is_set);

static bool zlang_register_function_names(const zlang_program_t *program, zlang_emitter_t *emitter);

static bool zlang_collect_stateful_usage(const zlang_program_t *program, zlang_emitter_t *emitter);

static bool zlang_function_marked(const zlang_emitter_t *emitter, const char *name);

static bool zlang_is_nested(const zlang_program_t *program, size_t index);
static bool zlang_is_generic_template(const zlang_ast_statement_t *statement);

static bool zlang_spec_extent_macro(const zlang_program_t *program, const zlang_emitter_t *emitter,
                                    const zlang_array_specialization_t *specialization,
                                    uint32_t axis, char *output, size_t capacity);

static const zlang_array_specialization_t *zlang_find_specialization(const zlang_emitter_t *emitter,
                                                                     size_t function_index);

static bool zlang_function_generated_name(const zlang_program_t *program,
                                          const zlang_emitter_t *emitter,
                                          const zlang_ast_statement_t *function,
                                          const zlang_array_specialization_t *specialization,
                                          char *output, size_t capacity);

static const zlang_array_specialization_t *
zlang_find_call_specialization(const zlang_program_t *program, const zlang_emitter_t *emitter,
                               const zlang_ast_expression_t *call);

static bool zlang_array_type_suffix(const char *type_name, char *output, size_t capacity);

static const char *zlang_generated_type(const zlang_program_t *program,
                                        const zlang_emitter_t *emitter, const char *type_name);

static bool zlang_same_text(const char *left, const char *right);

static bool zlang_activate_macro(zlang_emitter_t *emitter, const char *macro);

// NOLINTNEXTLINE(misc-no-recursion)
static const char *zlang_expression_type(const zlang_program_t *program,
                                         const zlang_emitter_t *emitter, size_t index);

static bool zlang_append_unsigned(zlang_emitter_t *emitter, uint32_t value) {
    char _digits[16];
    const int _written_k = snprintf(_digits, sizeof(_digits), "%u", (unsigned int)value);
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
    if (strcmp(type_name, "int32") == 0) {
        return "int32_t";
    }
    if (strcmp(type_name, "uint32") == 0) {
        return "uint32_t";
    }
    if (strcmp(type_name, "int64") == 0) {
        return "int64_t";
    }
    if (strcmp(type_name, "uint64") == 0) {
        return "uint64_t";
    }
    if (strcmp(type_name, "float") == 0) {
        return "float";
    }
    if (strcmp(type_name, "double") == 0) {
        return "double";
    }
    if (strcmp(type_name, "boolean") == 0) {
        return "bool";
    }
    if (strcmp(type_name, "string") == 0) {
        return "zlang_string_t";
    }
    if (strcmp(type_name, "arguments") == 0) {
        return "zlang_arguments_t";
    }
    return "void";
}

static const char *zlang_default_initializer(const char *type_name) {
    if (strcmp(type_name, "int32") == 0) {
        return "0";
    }
    if (strcmp(type_name, "uint32") == 0) {
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
    if (statement != NULL && statement->name_ != NULL && strncmp(statement->name_, "m_", 2U) == 0 &&
        (statement->kind_ == zlang_statement_declaration_ ||
         statement->kind_ == zlang_statement_array_declaration_)) {
        return true;
    }
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
        if (_statement_k->kind_ == zlang_statement_array_declaration_ &&
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

static void zlang_struct_array_helper(zlang_emitter_t *emitter, const char *name,
                                      const char *type_name, uint32_t rank, bool is_set) {
    for (size_t _index = 0U; _index < emitter->requested_array_helper_count_; ++_index) {
        if (strcmp(emitter->requested_array_helpers_[_index].name_, name) == 0) {
            return;
        }
    }
    if (emitter->requested_array_helper_count_ >= ZLANG_EMITTED_ARRAY_HELPER_CAPACITY) {
        return;
    }
    zlang_emitted_array_helper_t *const _entry_k =
        &emitter->requested_array_helpers_[emitter->requested_array_helper_count_];
    if (!zlang_copy_name(name, _entry_k->name_, sizeof(_entry_k->name_)) ||
        !zlang_copy_name(type_name, _entry_k->type_name_, sizeof(_entry_k->type_name_))) {
        return;
    }
    _entry_k->rank_ = rank;
    _entry_k->is_set_ = is_set;
    ++emitter->requested_array_helper_count_;
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
        (!zlang_make_unique_name(emitter, _user_name, generated, generated_capacity))) {
        return false;
    }
    zlang_emitted_symbol_t *const _symbol_k =
        &emitter->emitted_symbols_[emitter->active_symbol_count_];
    memset(_symbol_k, 0, sizeof(*_symbol_k));
    _symbol_k->source_name_ = source_name;
    _symbol_k->type_name_ = type_name;
    if (zlang_std_view_element_type(type_name, _symbol_k->view_element_,
                                    sizeof(_symbol_k->view_element_))) {
        (void)zlang_activate_macro(emitter, "ZLANG_STD_VIEW");
    }
    _symbol_k->capacity_ = capacity;
    _symbol_k->has_explicit_capacity_ = has_explicit_capacity;
    _symbol_k->is_sequence_ = is_sequence;
    _symbol_k->is_reference_ = is_reference;
    _symbol_k->is_array_ = false;
    _symbol_k->array_rank_ = 0U;
    (void)snprintf(_symbol_k->generated_name_, sizeof(_symbol_k->generated_name_), "%s", generated);
    if (needs_storage) {
        const int _storage_written_k = snprintf(
            _symbol_k->storage_name_, sizeof(_symbol_k->storage_name_), "zlang_gen_%s_storage_%lu",
            source_name, (unsigned long)emitter->next_index_++);
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

static bool zlang_register_self_symbol(zlang_emitter_t *emitter, const char *type_name,
                                       bool is_pointer) {
    if (emitter == NULL || type_name == NULL ||
        emitter->active_symbol_count_ >= ZLANG_EMITTED_SYMBOL_CAPACITY) {
        return false;
    }
    zlang_emitted_symbol_t *const _symbol_k =
        &emitter->emitted_symbols_[emitter->active_symbol_count_++];
    memset(_symbol_k, 0, sizeof(*_symbol_k));
    _symbol_k->source_name_ = "self";
    _symbol_k->type_name_ = type_name;
    _symbol_k->is_reference_ = is_pointer;
    return zlang_copy_name("self", _symbol_k->generated_name_,
                           sizeof(_symbol_k->generated_name_)) &&
           zlang_copy_name("self", _symbol_k->storage_name_, sizeof(_symbol_k->storage_name_)) &&
           (emitter->active_symbol_count_ > emitter->emitted_symbol_count_
                ? (emitter->emitted_symbol_count_ = emitter->active_symbol_count_, true)
                : true);
}

static bool zlang_register_method_parameter(zlang_emitter_t *emitter,
                                            const zlang_parameter_t *parameter, char *generated,
                                            size_t generated_capacity) {
    if (emitter == NULL || parameter == NULL || parameter->name_ == NULL || generated == NULL ||
        emitter->active_symbol_count_ >= ZLANG_EMITTED_SYMBOL_CAPACITY) {
        return false;
    }
    const int _written_k =
        snprintf(generated, generated_capacity, "app_param_%s", parameter->name_);
    if (_written_k < 0 || (size_t)_written_k >= generated_capacity) {
        return false;
    }
    for (size_t _index = 0U; _index < emitter->active_symbol_count_; ++_index) {
        const zlang_emitted_symbol_t *const _symbol_k = &emitter->emitted_symbols_[_index];
        if (zlang_same_text(_symbol_k->generated_name_, generated)) {
            return false;
        }
    }
    zlang_emitted_symbol_t *const _symbol_k =
        &emitter->emitted_symbols_[emitter->active_symbol_count_++];
    memset(_symbol_k, 0, sizeof(*_symbol_k));
    _symbol_k->source_name_ = parameter->name_;
    _symbol_k->type_name_ = parameter->type_name_;
    if (zlang_std_view_element_type(parameter->type_name_, _symbol_k->view_element_,
                                    sizeof(_symbol_k->view_element_))) {
        (void)zlang_activate_macro(emitter, "ZLANG_STD_VIEW");
    }
    _symbol_k->is_array_ = parameter->is_array_;
    _symbol_k->array_rank_ = parameter->rank_;
    for (uint32_t _axis = 0U; _axis < parameter->rank_ && _axis < ZLANG_ARRAY_MAX_RANK; ++_axis) {
        _symbol_k->array_extents_[_axis] = parameter->extents_[_axis];
    }
    _symbol_k->is_sequence_ = parameter->is_array_;
    _symbol_k->is_reference_ = parameter->is_reference_;
    zlang_copy_name(generated, _symbol_k->generated_name_, sizeof(_symbol_k->generated_name_));
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

static zlang_emitted_symbol_t *zlang_find_mutable_emitted_symbol(zlang_emitter_t *emitter,
                                                                 const char *source_name) {
    if (emitter == NULL || source_name == NULL) {
        return NULL;
    }
    for (size_t _index = emitter->active_symbol_count_; _index > 0U; --_index) {
        zlang_emitted_symbol_t *const _symbol_k = &emitter->emitted_symbols_[_index - 1U];
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

static bool zlang_current_field(const zlang_program_t *program, const zlang_emitter_t *emitter,
                                const char *name, const zlang_ast_statement_t **field);

static bool zlang_emit_symbol_lvalue(zlang_emitter_t *emitter, const char *source_name) {
    const zlang_emitted_symbol_t *const _symbol_k = zlang_find_active_symbol(emitter, source_name);
    if (_symbol_k == NULL) {
        if (emitter->program_ != NULL && emitter->current_struct_ != NULL &&
            emitter->current_self_name_ != NULL) {
            uint32_t _depth_k = 0U;
            const zlang_ast_statement_t *const _field_k = zlang_find_field_owner(
                emitter->program_, emitter->current_struct_, source_name, &_depth_k, 0U);
            if (_field_k != NULL) {
                if (_depth_k > 0U) {
                    return zlang_append_text(emitter, emitter->current_self_name_) &&
                           zlang_emit_base_chain(emitter, _depth_k,
                                                 emitter->current_self_is_pointer_) &&
                           zlang_append_text(emitter, ".") &&
                           zlang_append_field_name(emitter, _field_k->name_);
                }
                return zlang_append_text(emitter, emitter->current_self_name_) &&
                       zlang_append_text(emitter, emitter->current_self_is_pointer_ ? "->" : ".") &&
                       zlang_append_field_name(emitter, _field_k->name_);
            }
        }
        return false;
    }
    if (_symbol_k->is_reference_) {
        return zlang_append_text(emitter, "(*") &&
               zlang_append_text(emitter, _symbol_k->generated_name_) &&
               zlang_append_text(emitter, ")");
    }
    return zlang_append_text(emitter, _symbol_k->generated_name_);
}

static bool zlang_emit_symbol_value(zlang_emitter_t *emitter, const char *source_name) {
    return zlang_emit_symbol_lvalue(emitter, source_name);
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

static bool zlang_generic_emitted_name(const char *source, char *output, size_t capacity) {
    if (source == NULL || source[0] == '\0' || capacity == 0U) {
        return false;
    }
    size_t _length_k = 0U;
    for (size_t _index = 0U; source[_index] != '\0'; ++_index) {
        unsigned char _character_k = (unsigned char)source[_index];
        if (_character_k >= 'A' && _character_k <= 'Z') {
            _character_k = (unsigned char)(_character_k - 'A' + 'a');
        }
        const bool _valid_k = (_character_k >= 'a' && _character_k <= 'z') ||
                              (_character_k >= '0' && _character_k <= '9') || _character_k == '_';
        if (_length_k + 2U >= capacity) {
            return false;
        }
        output[_length_k++] = (char)(_valid_k && _character_k <= 0x7FU ? _character_k : '_');
    }
    while (_length_k > 0U && output[_length_k - 1U] == '_' && strchr(source, '<') != NULL) {
        --_length_k;
    }
    output[_length_k] = '\0';
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

static bool zlang_canonical_array_name(const zlang_program_t *program,
                                       const zlang_emitter_t *emitter, const char *element_type,
                                       uint32_t rank, const uint32_t *extents, char *output,
                                       size_t capacity) {
    if (output == NULL || capacity == 0U) {
        return false;
    }
    output[0] = '\0';
    const char *const _c_type_k = zlang_generated_type(program, emitter, element_type);
    char _suffix_k[ZLANG_EMITTED_NAME_CAPACITY];
    if (_c_type_k == NULL || !zlang_array_type_suffix(_c_type_k, _suffix_k, sizeof(_suffix_k))) {
        return false;
    }
    if (!zlang_assemble_name("zlang_gen_array_", _suffix_k, "", output, capacity)) {
        return false;
    }
    for (uint32_t _axis = 0U; _axis < rank; ++_axis) {
        const int _axis_written_k = snprintf(output + strlen(output), capacity - strlen(output),
                                             "_%lu", (unsigned long)extents[_axis]);
        if (_axis_written_k < 0 || (size_t)_axis_written_k >= capacity - strlen(output)) {
            return false;
        }
    }
    const int _tail_written_k = snprintf(output + strlen(output), capacity - strlen(output), "_t");
    return _tail_written_k >= 0 && (size_t)_tail_written_k < capacity - strlen(output);
}

static bool zlang_make_sequence_names(const zlang_program_t *program, zlang_emitter_t *emitter,
                                      const char *source, const zlang_ast_statement_t *statement,
                                      bool has_element_capacity, char *type_name,
                                      char *capacity_macro, char *element_macro, char *length_macro,
                                      char extent_macros[][ZLANG_EMITTED_NAME_CAPACITY],
                                      size_t capacity) {
    char _lower_base[ZLANG_EMITTED_NAME_CAPACITY];
    char _upper_base[ZLANG_EMITTED_NAME_CAPACITY];
    if (!zlang_copy_name(source, _lower_base, sizeof(_lower_base)) ||
        !zlang_upper_name(source, _upper_base, sizeof(_upper_base))) {
        return false;
    }
    char _canonical_name[ZLANG_EMITTED_NAME_CAPACITY];
    char _suffix[16];
    for (uint32_t _attempt = 0U;; ++_attempt) {
        const bool _canonical_k = zlang_canonical_array_name(
            program, emitter, statement->type_name_, statement->rank_, statement->extents_,
            _canonical_name, sizeof(_canonical_name));
        const bool _type_name_ok_k =
            statement->use_canonical_type_
                ? zlang_copy_name(_canonical_name, type_name, capacity)
                : zlang_assemble_name("app_array_", _lower_base, "_t", type_name, capacity);
        if ((statement->use_canonical_type_ && !_canonical_k) || !_type_name_ok_k ||
            !zlang_assemble_name("ZLANG_", _upper_base, "_CAPACITY", capacity_macro, capacity) ||
            !zlang_assemble_name("ZLANG_", _upper_base, "_ELEMENT_CAPACITY", element_macro,
                                 capacity)) {
            return false;
        }
        for (uint32_t _rank = 0U; _rank < statement->rank_; ++_rank) {
            char _rank_suffix[32];
            const int _rank_written_k =
                snprintf(_rank_suffix, sizeof(_rank_suffix), "_EXTENT_%u", (unsigned int)_rank);
            char _extent_base[ZLANG_EMITTED_NAME_CAPACITY];
            if (_rank_written_k < 0 || (size_t)_rank_written_k >= sizeof(_rank_suffix) ||
                !zlang_assemble_name("ZLANG_", _upper_base, _rank_suffix, _extent_base,
                                     sizeof(_extent_base)) ||
                !zlang_copy_name(_extent_base, extent_macros[_rank], ZLANG_EMITTED_NAME_CAPACITY)) {
                return false;
            }
            if (_rank == 0U) {
                char _length_name[ZLANG_EMITTED_NAME_CAPACITY];
                if (!zlang_assemble_name("ZLANG_", _upper_base, "_LENGTH", _length_name,
                                         sizeof(_length_name)) ||
                    !zlang_copy_name(_length_name, length_macro, capacity)) {
                    return false;
                }
            }
        }
        if (statement->use_canonical_type_ && zlang_generated_name_used(emitter, type_name)) {
            return true;
        }
        bool _used_k =
            (!statement->use_canonical_type_ && zlang_generated_name_used(emitter, type_name)) ||
            zlang_generated_name_used(emitter, capacity_macro) ||
            zlang_generated_name_used(emitter, length_macro);
        if (has_element_capacity && zlang_generated_name_used(emitter, element_macro)) {
            _used_k = true;
        }
        for (uint32_t _rank = 0U; !_used_k && _rank < statement->rank_; ++_rank) {
            _used_k = zlang_generated_name_used(emitter, extent_macros[_rank]);
        }
        if (!_used_k) {
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
        if (_statement_k->kind_ != zlang_statement_array_declaration_ ||
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
        if (!zlang_make_sequence_names(program, emitter, _statement_k->name_, _statement_k,
                                       _statement_k->has_element_capacity_, _entry_k->type_name_,
                                       _entry_k->capacity_macro_, _entry_k->element_macro_,
                                       _entry_k->length_macro_, _entry_k->extent_macros_,
                                       sizeof(_entry_k->type_name_))) {
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

static bool zlang_append_array_data(zlang_emitter_t *emitter,
                                    const zlang_emitted_symbol_t *symbol) {
    if (symbol == NULL) {
        return false;
    }
    return zlang_append_text(emitter, symbol->generated_name_) &&
           zlang_append_text(emitter, symbol->is_reference_ ? "->" : ".") &&
           zlang_append_text(emitter, "data_");
}

static const zlang_emitted_sequence_t *zlang_find_shape_entry(const zlang_emitter_t *emitter,
                                                              const char *element_type,
                                                              uint32_t rank,
                                                              const uint32_t *extents) {
    for (size_t _index = 0U; _index < emitter->emitted_sequence_count_; ++_index) {
        const zlang_emitted_sequence_t *const _entry_k = &emitter->emitted_sequences_[_index];
        const zlang_ast_statement_t *const _statement_k = _entry_k->statement_;
        if (_statement_k->type_name_ == NULL || element_type == NULL ||
            strcmp(_statement_k->type_name_, element_type) != 0 || _statement_k->rank_ != rank) {
            continue;
        }
        bool _same_k = true;
        for (uint32_t _axis = 0U; _axis < rank; ++_axis) {
            if (_statement_k->extents_[_axis] != extents[_axis]) {
                _same_k = false;
                break;
            }
        }
        if (_same_k) {
            return _entry_k;
        }
    }
    return NULL;
}

static bool zlang_append_array_extent_axis(zlang_emitter_t *emitter,
                                           const zlang_emitted_symbol_t *symbol, uint32_t axis) {
    if (symbol == NULL || axis >= symbol->array_rank_ || axis >= ZLANG_ARRAY_MAX_RANK) {
        return false;
    }
    return zlang_append_unsigned(emitter, symbol->array_extents_[axis]);
}

static bool zlang_append_array_extent(zlang_emitter_t *emitter,
                                      const zlang_emitted_symbol_t *symbol) {
    return zlang_append_array_extent_axis(emitter, symbol, 0U);
}

static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_emit_array_field_call(const zlang_program_t *program, zlang_emitter_t *emitter,
                            const zlang_ast_expression_t *field,
                            const zlang_ast_statement_t *field_statement,
                            const zlang_emitted_symbol_t *base_symbol, bool is_set,
                            size_t value_expression) {
    const char *const _type_k = zlang_generated_type(program, emitter, field_statement->type_name_);
    char _suffix_k[ZLANG_EMITTED_NAME_CAPACITY];
    char _helper_name_k[ZLANG_EMITTED_NAME_CAPACITY];
    if (_type_k == NULL || field->argument_count_ != field_statement->rank_ ||
        field_statement->rank_ < 2U ||
        !zlang_array_type_suffix(_type_k, _suffix_k, sizeof(_suffix_k))) {
        return false;
    }
    char _suffix_with_underscore_k[ZLANG_EMITTED_NAME_CAPACITY];
    if (!zlang_assemble_name("_", _suffix_k, "", _suffix_with_underscore_k,
                             sizeof(_suffix_with_underscore_k)) ||
        !zlang_array_helper_name(is_set ? "zlang_gen_array_set_" : "zlang_gen_array_get_",
                                 field_statement->rank_, _suffix_with_underscore_k, _helper_name_k,
                                 sizeof(_helper_name_k))) {
        return false;
    }
    zlang_struct_array_helper(emitter, _helper_name_k, _type_k, field_statement->rank_, is_set);
    if (!zlang_append_text(emitter, _helper_name_k) || !zlang_append_text(emitter, "(") ||
        !zlang_append_text(emitter, base_symbol->generated_name_) ||
        !zlang_append_text(emitter, base_symbol->is_reference_ ? "->" : ".") ||
        !zlang_append_field_name(emitter, field_statement->name_) ||
        !zlang_append_text(emitter, ".data_")) {
        return false;
    }
    for (uint32_t _axis = 0U; _axis < field_statement->rank_; ++_axis) {
        if (!zlang_append_text(emitter, ", ") ||
            !zlang_append_unsigned(emitter, field_statement->extents_[_axis])) {
            return false;
        }
    }
    for (uint32_t _axis = 0U; _axis < field->argument_count_; ++_axis) {
        if (!zlang_append_text(emitter, ", ") ||
            !zlang_emit_index_expression(program, field->argument_indices_[_axis], emitter)) {
            return false;
        }
    }
    if (is_set && (!zlang_append_text(emitter, ", ") ||
                   !zlang_emit_expression(program, value_expression, emitter))) {
        return false;
    }
    return zlang_append_text(emitter, ")");
}

static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_emit_array_typed_call(const zlang_program_t *program, zlang_emitter_t *emitter,
                            const zlang_emitted_symbol_t *symbol,
                            const zlang_ast_expression_t *tuple, bool is_set,
                            size_t value_expression) {
    if (symbol == NULL || tuple == NULL || tuple->argument_count_ < 2U) {
        return false;
    }
    const zlang_ast_statement_t *const _statement_k =
        zlang_find_sequence(emitter, symbol->source_name_);
    const zlang_emitted_sequence_t *const _entry_k =
        _statement_k != NULL ? zlang_find_sequence_entry(NULL, emitter, _statement_k)
                             : zlang_find_shape_entry(emitter, symbol->type_name_,
                                                      symbol->array_rank_, symbol->array_extents_);
    const char *const _type_k = zlang_generated_type(program, emitter, symbol->type_name_);
    char _suffix_k[ZLANG_EMITTED_NAME_CAPACITY];
    char _helper_name_k[ZLANG_EMITTED_NAME_CAPACITY];
    if (_entry_k == NULL || symbol->array_rank_ != tuple->argument_count_ || _type_k == NULL ||
        !zlang_array_type_suffix(_type_k, _suffix_k, sizeof(_suffix_k))) {
        return false;
    }
    char _suffix_with_underscore_k[ZLANG_EMITTED_NAME_CAPACITY];
    if (!zlang_assemble_name("_", _suffix_k, "", _suffix_with_underscore_k,
                             sizeof(_suffix_with_underscore_k)) ||
        !zlang_array_helper_name(is_set ? "zlang_gen_array_set_" : "zlang_gen_array_get_",
                                 symbol->array_rank_, _suffix_with_underscore_k, _helper_name_k,
                                 sizeof(_helper_name_k))) {
        return false;
    }
    zlang_struct_array_helper(emitter, _helper_name_k, _type_k, symbol->array_rank_, is_set);
    if (!zlang_append_text(emitter, _helper_name_k) || !zlang_append_text(emitter, "(") ||
        !zlang_append_array_data(emitter, symbol)) {
        return false;
    }
    for (uint32_t _axis = 0U; _axis < symbol->array_rank_; ++_axis) {
        if (!zlang_append_text(emitter, ", ") ||
            !zlang_append_unsigned(emitter, symbol->array_extents_[_axis])) {
            return false;
        }
    }
    for (uint32_t _axis = 0U; _axis < tuple->argument_count_; ++_axis) {
        if (!zlang_append_text(emitter, ", ") ||
            !zlang_emit_index_expression(program, tuple->argument_indices_[_axis], emitter)) {
            return false;
        }
    }
    if (is_set && (!zlang_append_text(emitter, ", ") ||
                   !zlang_emit_expression(program, value_expression, emitter))) {
        return false;
    }
    return zlang_append_text(emitter, ")");
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

static bool zlang_is_user_result_type(const zlang_program_t *program, const char *type_name) {
    const zlang_generic_application_t *const _application_k =
        type_name != NULL ? zlang_generic_find_application(program, type_name) : NULL;
    return _application_k != NULL && _application_k->base_name_ != NULL &&
           strcmp(_application_k->base_name_, "result_type") == 0 &&
           _application_k->argument_count_ == 2U && _application_k->arguments_[1] != NULL &&
           strcmp(_application_k->arguments_[1], "status_code") == 0;
}

static const zlang_emitted_user_result_t *zlang_find_user_result(const zlang_emitter_t *emitter,
                                                                 const char *name) {
    if (emitter == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < emitter->emitted_user_result_count_; ++_index) {
        const zlang_emitted_user_result_t *const _entry_k = &emitter->emitted_user_results_[_index];
        if (_entry_k->name_ != NULL && strcmp(_entry_k->name_, name) == 0) {
            return _entry_k;
        }
    }
    return NULL;
}

static const char *zlang_generated_type(const zlang_program_t *program,
                                        const zlang_emitter_t *emitter, const char *type_name) {
    const zlang_emitted_user_result_t *const _result_k = zlang_find_user_result(emitter, type_name);
    if (_result_k != NULL) {
        return _result_k->type_name_;
    }
    (void)program;
    const zlang_emitted_user_type_t *const _entry_k = zlang_find_user_type(emitter, type_name);
    if (_entry_k != NULL) {
        return _entry_k->type_name_;
    }
    const char *const _native_k = zlang_std_native_c_type(type_name);
    if (_native_k != NULL) {
        return _native_k;
    }
    if (zlang_std_is_view_type(type_name)) {
        return "zlang_view_t";
    }
    if (type_name != NULL &&
        (strcmp(type_name, "int32") == 0 || strcmp(type_name, "uint32") == 0 ||
         strcmp(type_name, "int64") == 0 || strcmp(type_name, "uint64") == 0 ||
         strcmp(type_name, "float") == 0 || strcmp(type_name, "double") == 0 ||
         strcmp(type_name, "boolean") == 0 || strcmp(type_name, "string") == 0 ||
         strcmp(type_name, "arguments") == 0 || strcmp(type_name, "void") == 0)) {
        return zlang_c_type(type_name);
    }
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

static bool zlang_has_dump_method(const zlang_program_t *program, const char *type_name) {
    const char *_owner_k = NULL;
    bool _is_trait_k = false;
    const zlang_ast_statement_t *const _method_k =
        zlang_find_method(program, type_name, "dump", &_owner_k, &_is_trait_k);
    return _method_k != NULL && _method_k->parameter_count_ == 0U &&
           _method_k->type_name_ != NULL && strcmp(_method_k->type_name_, "string") == 0;
}

static const char *zlang_user_result_payload(const zlang_program_t *program,
                                             const char *type_name) {
    const zlang_generic_application_t *const _application_k =
        type_name != NULL ? zlang_generic_find_application(program, type_name) : NULL;
    if (_application_k == NULL || _application_k->base_name_ == NULL ||
        strcmp(_application_k->base_name_, "result_type") != 0 ||
        _application_k->argument_count_ != 2U || _application_k->arguments_[1] == NULL ||
        strcmp(_application_k->arguments_[1], "status_code") != 0) {
        return NULL;
    }
    const char *const _payload_k = _application_k->arguments_[0];
    return zlang_has_dump_method(program, _payload_k) ? _payload_k : NULL;
}

static bool zlang_collect_user_result_types(zlang_emitter_t *emitter,
                                            const zlang_program_t *program) {
    for (size_t _index = 0U; _index < program->generic_application_count_; ++_index) {
        const zlang_generic_application_t *const _application_k =
            &program->generic_applications_[_index];
        if (zlang_find_user_result(emitter, _application_k->name_) != NULL) {
            continue;
        }
        const char *const _payload_k = zlang_user_result_payload(program, _application_k->name_);
        if (_payload_k == NULL) {
            continue;
        }
        if (emitter->emitted_user_result_count_ >= ZLANG_EMITTED_USER_TYPE_CAPACITY) {
            return false;
        }
        zlang_emitted_user_result_t *const _entry_k =
            &emitter->emitted_user_results_[emitter->emitted_user_result_count_++];
        memset(_entry_k, 0, sizeof(*_entry_k));
        _entry_k->name_ = _application_k->name_;
        _entry_k->payload_name_ = _payload_k;
        char _lower_k[ZLANG_EMITTED_NAME_CAPACITY];
        char _base_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_generic_emitted_name(_payload_k, _lower_k, sizeof(_lower_k)) ||
            !zlang_assemble_name("app_result_", _lower_k, "_status_t", _base_k, sizeof(_base_k)) ||
            !zlang_make_unique_name(emitter, _base_k, _entry_k->type_name_,
                                    sizeof(_entry_k->type_name_))) {
            return false;
        }
        char _dump_base_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_assemble_name("app_result_", _lower_k, "_status_dump", _dump_base_k,
                                 sizeof(_dump_base_k)) ||
            !zlang_make_unique_name(emitter, _dump_base_k, _entry_k->dump_name_,
                                    sizeof(_entry_k->dump_name_))) {
            return false;
        }
    }
    return true;
}

static bool zlang_collect_user_types(zlang_emitter_t *emitter, const zlang_program_t *program) {
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ != zlang_statement_struct_ &&
            _statement_k->kind_ != zlang_statement_enum_) {
            continue;
        }
        if (zlang_is_generic_template(_statement_k)) {
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
        const char *const _prefix_k =
            _statement_k->kind_ == zlang_statement_enum_ ? "app_enum_" : "app_struct_";
        if (!zlang_generic_emitted_name(_statement_k->name_, _lower_name, sizeof(_lower_name)) ||
            !zlang_assemble_name(_prefix_k, _lower_name, "_t", _base, sizeof(_base)) ||
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

static const zlang_ast_statement_t *zlang_find_struct_statement(const zlang_program_t *program,
                                                                const char *name) {
    if (program == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_struct_ && _statement_k->name_ != NULL &&
            strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

static bool zlang_statement_implements_interface(const zlang_program_t *program,
                                                 const zlang_ast_statement_t *structure,
                                                 const char *interface_name) {
    if (structure == NULL || interface_name == NULL) {
        return false;
    }
    for (size_t _index = 0U; _index < structure->implements_count_; ++_index) {
        const char *const _name_k = structure->implements_names_[_index];
        if (_name_k != NULL && strcmp(_name_k, interface_name) == 0) {
            return true;
        }
    }
    (void)program;
    return false;
}

static const zlang_ast_statement_t *zlang_find_interface(const zlang_program_t *program,
                                                         const char *name) {
    if (program == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_interface_ && _statement_k->name_ != NULL &&
            strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

static const zlang_ast_statement_t *zlang_find_trait_statement(const zlang_program_t *program,
                                                               const char *name) {
    if (program == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_trait_ && _statement_k->name_ != NULL &&
            strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
static const zlang_ast_statement_t *zlang_struct_root(const zlang_program_t *program,
                                                      const char *name, uint32_t depth) {
    if (name == NULL || depth > 16U) {
        return NULL;
    }
    const zlang_ast_statement_t *const _structure_k = zlang_find_struct_statement(program, name);
    if (_structure_k == NULL || _structure_k->base_name_ == NULL) {
        return _structure_k;
    }
    return zlang_struct_root(program, _structure_k->base_name_, depth + 1U);
}

static const zlang_ast_statement_t *zlang_struct_root_name(const zlang_program_t *program,
                                                           const char *name) {
    return zlang_struct_root(program, name, 0U);
}

static bool zlang_struct_has_descendant(const zlang_program_t *program, const char *name) {
    if (name == NULL) {
        return false;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_struct_ && _statement_k->base_name_ != NULL &&
            strcmp(_statement_k->base_name_, name) == 0) {
            return true;
        }
    }
    return false;
}

static bool zlang_struct_is_polymorphic(const zlang_program_t *program, const char *name) {
    const zlang_ast_statement_t *const _structure_k = zlang_find_struct_statement(program, name);
    return _structure_k != NULL &&
           (_structure_k->base_name_ != NULL || zlang_struct_has_descendant(program, name));
}

static bool zlang_is_struct_ancestor(const zlang_program_t *program, const char *ancestor,
                                     const char *descendant) {
    const zlang_ast_statement_t *const _root_k = zlang_struct_root_name(program, descendant);
    const zlang_ast_statement_t *const _ancestor_k = zlang_find_struct_statement(program, ancestor);
    if (_root_k == NULL || _ancestor_k == NULL) {
        return false;
    }
    return strcmp(_root_k->name_, _ancestor_k->name_) == 0;
}

static uint32_t zlang_struct_depth(const zlang_program_t *program, const char *ancestor,
                                   const char *descendant) {
    const zlang_ast_statement_t *const _root_k = zlang_struct_root_name(program, descendant);
    const zlang_ast_statement_t *const _ancestor_k = zlang_find_struct_statement(program, ancestor);
    if (_root_k == NULL || _ancestor_k == NULL || strcmp(_root_k->name_, _ancestor_k->name_) != 0) {
        return 0U;
    }
    uint32_t _depth_k = 0U;
    const zlang_ast_statement_t *const _current_k =
        zlang_find_struct_statement(program, descendant);
    for (const char *_name_k = _current_k != NULL ? _current_k->name_ : NULL;
         _name_k != NULL && strcmp(_name_k, ancestor) != 0 && _depth_k < 16U;) {
        const zlang_ast_statement_t *const _next_k = zlang_find_struct_statement(program, _name_k);
        if (_next_k == NULL || _next_k->base_name_ == NULL) {
            return 0U;
        }
        _name_k = _next_k->base_name_;
        ++_depth_k;
    }
    return _depth_k;
}

static const zlang_ast_statement_t *zlang_find_method_local(const zlang_program_t *program,
                                                            const zlang_ast_statement_t *owner,
                                                            const char *name) {
    if (owner == NULL || name == NULL) {
        return NULL;
    }
    const size_t _end_k = owner->body_start_ + owner->body_count_;
    for (size_t _index = owner->body_start_; _index < _end_k && _index < program->statement_count_;
         ++_index) {
        const zlang_ast_statement_t *const _method_k = &program->statements_[_index];
        if (_method_k->kind_ == zlang_statement_function_ && _method_k->name_ != NULL &&
            strcmp(_method_k->name_, name) == 0) {
            return _method_k;
        }
    }
    return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
static const zlang_ast_statement_t *zlang_find_method_any(const zlang_program_t *program,
                                                          const char *struct_name, const char *name,
                                                          const char **owner_struct_name,
                                                          bool *is_trait, uint32_t depth) {
    if (struct_name == NULL || name == NULL || depth > 16U) {
        return NULL;
    }
    const zlang_ast_statement_t *const _structure_k =
        zlang_find_struct_statement(program, struct_name);
    if (_structure_k == NULL) {
        return NULL;
    }
    const zlang_ast_statement_t *const _own_k =
        zlang_find_method_local(program, _structure_k, name);
    if (_own_k != NULL) {
        if (owner_struct_name != NULL) {
            *owner_struct_name = _structure_k->name_;
        }
        if (is_trait != NULL) {
            *is_trait = false;
        }
        return _own_k;
    }
    for (size_t _trait = 0U; _trait < _structure_k->uses_count_; ++_trait) {
        const zlang_ast_statement_t *const _trait_k =
            zlang_find_trait_statement(program, _structure_k->uses_names_[_trait]);
        const zlang_ast_statement_t *const _method_k =
            zlang_find_method_local(program, _trait_k, name);
        if (_method_k != NULL) {
            if (owner_struct_name != NULL) {
                *owner_struct_name = _structure_k->name_;
            }
            if (is_trait != NULL) {
                *is_trait = true;
            }
            return _method_k;
        }
    }
    if (_structure_k->base_name_ == NULL) {
        return NULL;
    }
    return zlang_find_method_any(program, _structure_k->base_name_, name, owner_struct_name,
                                 is_trait, depth + 1U);
}

static const zlang_ast_statement_t *zlang_find_method(const zlang_program_t *program,
                                                      const char *struct_name, const char *name,
                                                      const char **owner_struct_name,
                                                      bool *is_trait) {
    return zlang_find_method_any(program, struct_name, name, owner_struct_name, is_trait, 0U);
}

// NOLINTNEXTLINE(misc-no-recursion)
static const zlang_ast_statement_t *zlang_find_field_owner(const zlang_program_t *program,
                                                           const zlang_ast_statement_t *structure,
                                                           const char *name, uint32_t *depth,
                                                           uint32_t limit) {
    if (structure == NULL || name == NULL || depth == NULL || limit > 16U) {
        return NULL;
    }
    const zlang_ast_statement_t *const _field_k =
        zlang_find_field_statement(program, structure, name);
    if (_field_k != NULL) {
        return _field_k;
    }
    if (structure->base_name_ == NULL) {
        return NULL;
    }
    const zlang_ast_statement_t *const _base_k =
        zlang_find_struct_statement(program, structure->base_name_);
    const zlang_ast_statement_t *const _owner_k =
        zlang_find_field_owner(program, _base_k, name, depth, limit + 1U);
    if (_owner_k != NULL) {
        ++*depth;
    }
    return _owner_k;
}

static bool zlang_method_is_virtual(const zlang_program_t *program, const char *struct_name,
                                    const char *name) {
    const char *_owner_k = NULL;
    bool _is_trait_k = false;
    const zlang_ast_statement_t *const _method_k =
        zlang_find_method(program, struct_name, name, &_owner_k, &_is_trait_k);
    return !_is_trait_k && _method_k != NULL && !_method_k->is_static_ &&
           !_method_k->is_constructor_ && zlang_struct_is_polymorphic(program, struct_name);
}

static bool zlang_slot_name(const char *method_name, char *output, size_t capacity) {
    char _lower_k[ZLANG_EMITTED_NAME_CAPACITY];
    return zlang_lower_name(method_name, _lower_k, sizeof(_lower_k)) &&
           zlang_assemble_name("zlang_slot_", _lower_k, "_", output, capacity);
}

static bool zlang_vtable_wrapper_name(const char *struct_name, const char *method_name,
                                      char *output, size_t capacity) {
    char _lower_struct_k[ZLANG_EMITTED_NAME_CAPACITY];
    char _lower_method_k[ZLANG_EMITTED_NAME_CAPACITY];
    char _base_k[ZLANG_EMITTED_NAME_CAPACITY];
    if (!zlang_lower_name(struct_name, _lower_struct_k, sizeof(_lower_struct_k)) ||
        !zlang_lower_name(method_name, _lower_method_k, sizeof(_lower_method_k)) ||
        !zlang_assemble_name("zlang_vtable_", _lower_struct_k, "_", _base_k, sizeof(_base_k)) ||
        strlen(_base_k) + strlen(_lower_method_k) >= capacity) {
        return false;
    }
    if (!zlang_copy_name(_base_k, output, capacity)) {
        return false;
    }
    memcpy(output + strlen(_base_k), _lower_method_k, strlen(_lower_method_k) + 1U);
    return true;
}

static bool zlang_is_vtable_slot(const zlang_program_t *program, const char *root_name,
                                 size_t method_index) {
    const zlang_ast_statement_t *const _method_k = &program->statements_[method_index];
    if (_method_k->kind_ != zlang_statement_function_ || _method_k->owner_name_ == NULL ||
        _method_k->is_static_ || _method_k->is_constructor_ || _method_k->name_ == NULL) {
        return false;
    }
    const zlang_ast_statement_t *const _owner_k =
        zlang_find_struct_statement(program, _method_k->owner_name_);
    const zlang_ast_statement_t *const _root_k =
        zlang_struct_root_name(program, _method_k->owner_name_);
    if (_owner_k == NULL || _root_k == NULL || strcmp(_root_k->name_, root_name) != 0) {
        return false;
    }
    for (size_t _index = 0U; _index < method_index; ++_index) {
        const zlang_ast_statement_t *const _earlier_k = &program->statements_[_index];
        if (_earlier_k->kind_ != zlang_statement_function_ || _earlier_k->owner_name_ == NULL ||
            _earlier_k->is_static_ || _earlier_k->is_constructor_ || _earlier_k->name_ == NULL ||
            strcmp(_earlier_k->name_, _method_k->name_) != 0) {
            continue;
        }
        const zlang_ast_statement_t *const _earlier_root_k =
            zlang_struct_root_name(program, _earlier_k->owner_name_);
        if (_earlier_root_k != NULL && strcmp(_earlier_root_k->name_, root_name) == 0) {
            return false;
        }
    }
    return true;
}

static const zlang_ast_statement_t *zlang_effective_method(const zlang_program_t *program,
                                                           const char *struct_name,
                                                           const char *method_name) {
    const char *_owner_k = NULL;
    bool _is_trait_k = false;
    const zlang_ast_statement_t *const _method_k =
        zlang_find_method(program, struct_name, method_name, &_owner_k, &_is_trait_k);
    return !_is_trait_k ? _method_k : NULL;
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

static const zlang_ast_statement_t *zlang_find_import_statement(const zlang_program_t *program,
                                                                const char *name) {
    if (program == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_import_ && _statement_k->name_ != NULL &&
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

static const zlang_ast_statement_t *
zlang_find_field_statement(const zlang_program_t *program, const zlang_ast_statement_t *structure,
                           const char *name) {
    if (structure == NULL || name == NULL) {
        return NULL;
    }
    const size_t _end_k = structure->body_start_ + structure->body_count_;
    for (size_t _index = structure->body_start_;
         _index < _end_k && _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _field_k = &program->statements_[_index];
        if ((_field_k->kind_ != zlang_statement_declaration_ &&
             _field_k->kind_ != zlang_statement_array_declaration_) ||
            _field_k->name_ == NULL || strcmp(_field_k->name_, name) != 0) {
            continue;
        }
        return _field_k;
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
        if (_expression_k->is_int64_) {
            return "int64";
        }
        if (_expression_k->is_uint64_) {
            return "uint64";
        }
        return _expression_k->is_unsigned_ ? "uint32" : "int32";
    case zlang_expression_float_literal_:
        return _expression_k->is_float_ ? "float" : "double";
    case zlang_expression_conversion_:
        return _expression_k->name_;
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
        if (emitter != NULL && emitter->current_struct_ != NULL) {
            const zlang_ast_statement_t *const _field_k = zlang_find_field_owner(
                program, emitter->current_struct_, _expression_k->name_, &(uint32_t){0}, 0U);
            if (_field_k != NULL) {
                return _field_k->type_name_;
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
            zlang_find_struct_statement(program, _record_name_k);
        uint32_t _field_depth_k = 0U;
        const zlang_ast_statement_t *const _field_k =
            zlang_find_field_owner(program, _record_k,
                                   _expression_k->kind_ == zlang_expression_field_
                                       ? _expression_k->name_
                                       : program->expressions_[_expression_k->left_].name_,
                                   &_field_depth_k, 0U);
        return _field_k != NULL ? _field_k->type_name_ : NULL;
    }
    case zlang_expression_add_:
    case zlang_expression_sub_:
    case zlang_expression_mul_:
    case zlang_expression_div_:
    case zlang_expression_mod_:
    case zlang_expression_assignment_:
    case zlang_expression_compound_assignment_: {
        const char *const _left_k = zlang_expression_type(program, emitter, _expression_k->left_);
        if (_left_k != NULL) {
            return _left_k;
        }
        const zlang_ast_expression_t *const _target_k =
            &program->expressions_[_expression_k->left_];
        const zlang_ast_statement_t *_field_k = NULL;
        if (emitter->program_ != NULL && emitter->current_struct_ != NULL &&
            zlang_current_field(emitter->program_, emitter, _target_k->name_, &_field_k)) {
            return _field_k->type_name_;
        }
        return NULL;
    }
    case zlang_expression_negative_: {
        const char *const _operand_k =
            zlang_expression_type(program, emitter, _expression_k->left_);
        return _operand_k != NULL ? _operand_k : "int32";
    }
    case zlang_expression_length_:
        return "uint32";
    case zlang_expression_view_:
        return _expression_k->resolved_type_name_;
    case zlang_expression_index_: {
        const zlang_emitted_symbol_t *const _symbol_k =
            zlang_find_active_symbol(emitter, _expression_k->name_);
        if (_symbol_k == NULL) {
            return NULL;
        }
        if (_symbol_k->view_element_[0] != '\0') {
            return _symbol_k->view_element_;
        }
        if (!_symbol_k->is_sequence_ && strcmp(_symbol_k->type_name_, "string") == 0) {
            return "uint32";
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
        const zlang_std_function_t *const _std_k =
            zlang_find_import_statement(program, _expression_k->name_) != NULL
                ? zlang_std_call(program, emitter, _expression_k)
                : NULL;
        if (_std_k != NULL) {
            return _std_k->return_type_;
        }
        const zlang_emitted_symbol_t *const _symbol_k =
            zlang_find_active_symbol(emitter, _expression_k->name_);
        if (_symbol_k != NULL) {
            return _symbol_k->type_name_;
        }
        const zlang_ast_statement_t *const _function_k =
            zlang_find_function_statement(program, _expression_k->name_);
        return _function_k != NULL ? _function_k->type_name_ : NULL;
    }
    case zlang_expression_method_call_: {
        const zlang_emitted_symbol_t *const _receiver_k =
            zlang_find_active_symbol(emitter, _expression_k->base_name_);
        const char *_receiver_type_k = NULL;
        if (_receiver_k != NULL) {
            _receiver_type_k = _receiver_k->type_name_;
        } else if (zlang_find_struct_statement(program, _expression_k->base_name_) != NULL) {
            _receiver_type_k = _expression_k->base_name_;
        }
        if (_receiver_type_k == NULL) {
            return NULL;
        }
        const zlang_std_result_t *const _native_k = zlang_std_find_result(_receiver_type_k);
        if (_native_k != NULL) {
            return zlang_std_native_result_method_type(_native_k, _expression_k->name_);
        }
        const char *_owner_k = NULL;
        bool _is_trait_k = false;
        const zlang_ast_statement_t *const _method_k = zlang_find_method(
            program, _receiver_type_k, _expression_k->name_, &_owner_k, &_is_trait_k);
        return _method_k != NULL ? _method_k->type_name_ : NULL;
    }
    default:
        return NULL;
    }
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_method_generated_name(const char *struct_name, const char *method_name,
                                        bool is_constructor, char *output, size_t capacity) {
    char _struct_k[32U];
    char _method_k[24U];
    if (!zlang_generic_emitted_name(struct_name, _struct_k, sizeof(_struct_k)) ||
        (!is_constructor && !zlang_lower_name(method_name, _method_k, sizeof(_method_k)))) {
        return false;
    }
    const int _written_k = is_constructor
                               ? snprintf(output, capacity, "app_fn_%s_construct", _struct_k)
                               : snprintf(output, capacity, "app_fn_%s_%s", _struct_k, _method_k);
    return _written_k >= 0 && (size_t)_written_k < capacity;
}

static bool zlang_current_field(const zlang_program_t *program, const zlang_emitter_t *emitter,
                                const char *name, const zlang_ast_statement_t **field) {
    if (program == NULL || emitter == NULL || emitter->current_struct_ == NULL || field == NULL) {
        return false;
    }
    uint32_t _depth_k = 0U;
    const zlang_ast_statement_t *const _field_k =
        zlang_find_field_owner(program, emitter->current_struct_, name, &_depth_k, 0U);
    if (_field_k == NULL) {
        return false;
    }
    *field = _field_k;
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_index_expression(const zlang_program_t *program, size_t index,
                                        zlang_emitter_t *emitter) {
    if (index >= program->expression_count_) {
        return false;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    const char *const _type_k = zlang_expression_type(program, emitter, index);
    const bool _literal_k = _expression_k->kind_ == zlang_expression_integer_literal_ &&
                            _expression_k->integer_value_ >= 0;
    const bool _needs_cast_k = !_literal_k && _type_k != NULL && strcmp(_type_k, "int32") == 0;
    return (!_needs_cast_k || zlang_append_text(emitter, "(uint32_t)")) &&
           zlang_emit_expression(program, index, emitter);
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

static bool zlang_expression_is_comparison(zlang_expression_kind_t kind) {
    return kind == zlang_expression_equal_ || kind == zlang_expression_less_ ||
           kind == zlang_expression_greater_ || kind == zlang_expression_less_equal_ ||
           kind == zlang_expression_greater_equal_ || kind == zlang_expression_not_equal_;
}

static const char *zlang_array_get_function(const char *type_name) {
    if (type_name == NULL) {
        return NULL;
    }
    if (strcmp(type_name, "int32") == 0) {
        return "zlang_array_get_i32(";
    }
    if (strcmp(type_name, "uint32") == 0) {
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

static const char *zlang_arithmetic_function(zlang_expression_kind_t kind, const char *type_name) {
    if (type_name == NULL) {
        return NULL;
    }
    const bool _unsigned_k = strcmp(type_name, "uint32") == 0;
    const bool _signed_64_k = strcmp(type_name, "int64") == 0;
    const bool _unsigned_64_k = strcmp(type_name, "uint64") == 0;
    switch (kind) {
    case zlang_expression_add_:
        if (_signed_64_k) {
            return "zlang_add_i64(";
        }
        if (_unsigned_64_k) {
            return "zlang_add_u64(";
        }
        return _unsigned_k ? "zlang_add_u32(" : "zlang_add_i32(";
    case zlang_expression_sub_:
        if (_signed_64_k) {
            return "zlang_sub_i64(";
        }
        if (_unsigned_64_k) {
            return "zlang_sub_u64(";
        }
        return _unsigned_k ? "zlang_sub_u32(" : "zlang_sub_i32(";
    case zlang_expression_mul_:
        if (_signed_64_k) {
            return "zlang_mul_i64(";
        }
        if (_unsigned_64_k) {
            return "zlang_mul_u64(";
        }
        return _unsigned_k ? "zlang_mul_u32(" : "zlang_mul_i32(";
    case zlang_expression_div_:
        if (_signed_64_k) {
            return "zlang_div_i64(";
        }
        if (_unsigned_64_k) {
            return "zlang_div_u64(";
        }
        return _unsigned_k ? "zlang_div_u32(" : "zlang_div_i32(";
    case zlang_expression_mod_:
        if (_signed_64_k) {
            return "zlang_mod_i64(";
        }
        if (_unsigned_64_k) {
            return "zlang_mod_u64(";
        }
        return _unsigned_k ? "zlang_mod_u32(" : "zlang_mod_i32(";
    default:
        return NULL;
    }
}

static const char *zlang_arithmetic_operator(zlang_expression_kind_t kind) {
    switch (kind) {
    case zlang_expression_add_:
        return "+";
    case zlang_expression_sub_:
        return "-";
    case zlang_expression_mul_:
        return "*";
    case zlang_expression_div_:
        return "/";
    default:
        return NULL;
    }
}

static const char *zlang_conversion_function(const char *target, const char *source) {
    if (target == NULL || source == NULL) {
        return NULL;
    }
    if (strcmp(target, source) == 0) {
        return NULL;
    }
    const bool _source_float_k = strcmp(source, "float") == 0 || strcmp(source, "double") == 0;
    const bool _source_i64_k = strcmp(source, "int32") == 0 || strcmp(source, "int64") == 0;
    const bool _source_u32_k = strcmp(source, "uint32") == 0;
    const bool _source_u64_k = strcmp(source, "uint64") == 0;
    if (strcmp(target, "int32") == 0) {
        if (_source_i64_k) {
            return "zlang_to_i32_i64(";
        }
        if (_source_u32_k) {
            return "zlang_to_i32_u32(";
        }
        if (_source_u64_k) {
            return "zlang_to_i32_u64(";
        }
        if (_source_float_k) {
            return "zlang_to_i32_f64(";
        }
    }
    if (strcmp(target, "uint32") == 0) {
        if (_source_i64_k) {
            return "zlang_to_u32_i64(";
        }
        if (_source_u32_k) {
            return "zlang_to_u32_u32(";
        }
        if (_source_u64_k) {
            return "zlang_to_u32_u64(";
        }
        if (_source_float_k) {
            return "zlang_to_u32_f64(";
        }
    }
    if (strcmp(target, "int64") == 0) {
        if (_source_i64_k || _source_u32_k) {
            return "zlang_to_i64_i64(";
        }
        if (_source_u64_k) {
            return "zlang_to_i64_u64(";
        }
        if (_source_float_k) {
            return "zlang_to_i64_f64(";
        }
    }
    if (strcmp(target, "uint64") == 0) {
        if (_source_i64_k) {
            return "zlang_to_u64_i64(";
        }
        if (_source_u32_k || _source_u64_k) {
            return "zlang_to_u64_u64(";
        }
        if (_source_float_k) {
            return "zlang_to_u64_f64(";
        }
    }
    return NULL;
}

static bool
// NOLINTNEXTLINE(misc-no-recursion)
zlang_emit_comparison(const zlang_program_t *program, const zlang_ast_expression_t *expression,
                      zlang_emitter_t *emitter) { // NOLINT(misc-no-recursion)
    const char *const _left_type_k = zlang_expression_type(program, emitter, expression->left_);
    if (_left_type_k != NULL && strcmp(_left_type_k, "string") == 0) {
        const bool _inequality_k = expression->kind_ == zlang_expression_not_equal_;
        if (expression->kind_ != zlang_expression_equal_ && !_inequality_k) {
            return false;
        }
        if (_inequality_k && !zlang_append_text(emitter, "(!")) {
            return false;
        }
        const bool _result_k = zlang_append_text(emitter, "zlang_string_equal(") &&
                               zlang_emit_expression(program, expression->left_, emitter) &&
                               zlang_append_text(emitter, ", ") &&
                               zlang_emit_expression(program, expression->right_, emitter) &&
                               zlang_append_text(emitter, ")") &&
                               (!_inequality_k || zlang_append_text(emitter, ")"));
        return _result_k;
    }
    return zlang_emit_expression(program, expression->left_, emitter) &&
           zlang_append_text(emitter, zlang_comparison_operator(expression->kind_)) &&
           zlang_emit_expression(program, expression->right_, emitter);
}

static bool zlang_emit_string_concat_call(const zlang_program_t *program, size_t index,
                                          const char *storage, const char *capacity,
                                          zlang_emitter_t *emitter);

static bool zlang_emit_literal_view(const zlang_program_t *program, size_t index,
                                    zlang_emitter_t *emitter);

static size_t zlang_concat_part_count(const zlang_program_t *program, size_t index);

static bool zlang_emit_concat_parts(const zlang_program_t *program, size_t index,
                                    zlang_emitter_t *emitter);

static const zlang_ast_statement_t *zlang_find_array_decl(const zlang_program_t *program,
                                                          const char *name);

static const char *zlang_program_declared_type(const zlang_program_t *program, const char *name) {
    if (program == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if ((_statement_k->kind_ == zlang_statement_declaration_ ||
             _statement_k->kind_ == zlang_statement_array_declaration_) &&
            _statement_k->name_ != NULL && strcmp(_statement_k->name_, name) == 0) {
            return _statement_k->type_name_;
        }
    }
    for (size_t _index = 0U; _index < program->parameter_count_; ++_index) {
        const zlang_parameter_t *const _parameter_k = &program->parameters_[_index];
        if (_parameter_k->name_ != NULL && strcmp(_parameter_k->name_, name) == 0) {
            return _parameter_k->type_name_;
        }
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_foreach_) {
            if (_statement_k->name_ != NULL && strcmp(_statement_k->name_, name) == 0) {
                for (size_t _array = 0U; _array < program->statement_count_; ++_array) {
                    const zlang_ast_statement_t *const _array_k = &program->statements_[_array];
                    if (_array_k->kind_ == zlang_statement_array_declaration_ &&
                        _array_k->name_ != NULL && _statement_k->iterable_name_ != NULL &&
                        strcmp(_array_k->name_, _statement_k->iterable_name_) == 0) {
                        return _array_k->type_name_;
                    }
                }
            }
            if (_statement_k->index_name_ != NULL && strcmp(_statement_k->index_name_, name) == 0) {
                return "uint32";
            }
            for (uint32_t _axis = 0U; _axis < _statement_k->foreach_rank_; ++_axis) {
                if (_statement_k->foreach_names_[_axis] != NULL &&
                    strcmp(_statement_k->foreach_names_[_axis], name) == 0) {
                    return "uint32";
                }
            }
        }
        if (_statement_k->initializer_statement_ < program->statement_count_) {
            const zlang_ast_statement_t *const _initializer_k =
                &program->statements_[_statement_k->initializer_statement_];
            if ((_initializer_k->kind_ == zlang_statement_declaration_ ||
                 _initializer_k->kind_ == zlang_statement_array_declaration_) &&
                _initializer_k->name_ != NULL && strcmp(_initializer_k->name_, name) == 0) {
                return _initializer_k->type_name_;
            }
        }
    }
    return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
static const char *zlang_call_argument_type(const zlang_program_t *program,
                                            const zlang_emitter_t *emitter, size_t index);

// NOLINTNEXTLINE(misc-no-recursion)
static const char *zlang_analyze_type_fallback(const zlang_program_t *program,
                                               const zlang_emitter_t *emitter, size_t index) {
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    switch (_expression_k->kind_) {
    case zlang_expression_add_:
    case zlang_expression_sub_:
    case zlang_expression_mul_:
    case zlang_expression_div_:
    case zlang_expression_mod_:
    case zlang_expression_negative_:
        return zlang_call_argument_type(program, emitter, _expression_k->left_);
    case zlang_expression_integer_literal_:
        if (_expression_k->is_int64_) {
            return "int64";
        }
        if (_expression_k->is_uint64_) {
            return "uint64";
        }
        return _expression_k->is_unsigned_ ? "uint32" : "int32";
    case zlang_expression_float_literal_:
        return _expression_k->is_float_ ? "float" : "double";
    case zlang_expression_boolean_literal_:
        return "boolean";
    case zlang_expression_string_literal_:
    case zlang_expression_string_concat_:
        return "string";
    case zlang_expression_index_: {
        const zlang_ast_statement_t *const _array_k =
            zlang_find_array_decl(program, _expression_k->name_);
        return _array_k != NULL ? _array_k->type_name_ : NULL;
    }
    case zlang_expression_field_: {
        const char *const _base_name_k = _expression_k->base_name_ != NULL
                                             ? _expression_k->base_name_
                                             : program->expressions_[_expression_k->left_].name_;
        const char *const _record_name_k = zlang_program_declared_type(program, _base_name_k);
        if (_record_name_k == NULL) {
            return NULL;
        }
        const zlang_ast_statement_t *const _record_k =
            zlang_find_struct_statement(program, _record_name_k);
        if (_record_k == NULL) {
            return NULL;
        }
        const zlang_ast_statement_t *const _field_k =
            zlang_find_field_statement(program, _record_k, _expression_k->name_);
        return _field_k != NULL ? _field_k->type_name_ : NULL;
    }
    case zlang_expression_call_: {
        for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
            const zlang_ast_statement_t *const _function_k = &program->statements_[_index];
            if (_function_k->kind_ == zlang_statement_function_ && _function_k->name_ != NULL &&
                strcmp(_function_k->name_, _expression_k->name_) == 0) {
                return _function_k->type_name_;
            }
        }
        return zlang_std_call(program, emitter, _expression_k) != NULL
                   ? zlang_std_call(program, emitter, _expression_k)->return_type_
                   : NULL;
    }
    case zlang_expression_conversion_:
        return _expression_k->name_;
    case zlang_expression_enum_item_: {
        for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
            const zlang_ast_statement_t *const _enum_k = &program->statements_[_index];
            if (_enum_k->kind_ == zlang_statement_enum_ && _enum_k->name_ != NULL) {
                const size_t _end_k = _enum_k->initializer_start_ + _enum_k->initializer_count_;
                for (size_t _item = _enum_k->initializer_start_;
                     _item < _end_k && _item < program->expression_count_; ++_item) {
                    const zlang_ast_expression_t *const _item_k = &program->expressions_[_item];
                    if (_item_k->kind_ == zlang_expression_enum_item_ && _item_k->name_ != NULL &&
                        _expression_k->name_ != NULL &&
                        strcmp(_item_k->name_, _expression_k->name_) == 0) {
                        return _enum_k->name_;
                    }
                }
            }
        }
        return NULL;
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
    case zlang_expression_length_:
        return "uint32";
    default:
        return NULL;
    }
}

// NOLINTNEXTLINE(misc-no-recursion)
static const char *zlang_call_argument_type(const zlang_program_t *program,
                                            const zlang_emitter_t *emitter, size_t index) {
    if (index < program->expression_count_ &&
        program->expressions_[index].resolved_type_name_ != NULL) {
        return program->expressions_[index].resolved_type_name_;
    }
    const char *const _type_k = zlang_expression_type(program, emitter, index);
    if (_type_k != NULL || index >= program->expression_count_) {
        return _type_k;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ == zlang_expression_identifier_) {
        const char *const _declared_k = zlang_program_declared_type(program, _expression_k->name_);
        if (_declared_k != NULL) {
            return _declared_k;
        }
    }
    return zlang_analyze_type_fallback(program, emitter, index);
}

// NOLINTNEXTLINE(misc-no-recursion)
static const zlang_std_function_t *zlang_std_call(const zlang_program_t *program,
                                                  const zlang_emitter_t *emitter,
                                                  const zlang_ast_expression_t *call) {
    const char *_argument_types_k[ZLANG_STD_MAX_PARAMETERS] = {NULL};
    const size_t _argument_count_k = call->argument_count_ < ZLANG_STD_MAX_PARAMETERS
                                         ? call->argument_count_
                                         : ZLANG_STD_MAX_PARAMETERS;
    for (size_t _argument = 0U; _argument < _argument_count_k; ++_argument) {
        _argument_types_k[_argument] =
            zlang_call_argument_type(program, emitter, call->argument_indices_[_argument]);
    }
    const zlang_ast_statement_t *const _import_k =
        zlang_find_import_statement(program, call->name_);
    if (_import_k == NULL) {
        return NULL;
    }
    const zlang_std_function_t *const _resolved_k = zlang_std_resolve(
        zlang_std_domain(_import_k->type_name_), call->name_, _argument_types_k, _argument_count_k);
    if (_resolved_k != NULL) {
        return _resolved_k;
    }
    if (strcmp(call->name_, "write") == 0 && _argument_count_k == 1U &&
        _argument_types_k[0] == NULL) {
        return zlang_std_resolve(zlang_std_domain(_import_k->type_name_), call->name_,
                                 (const char *const[]){"uint32"}, 1U);
    }
    return NULL;
}

static bool zlang_emit_base_chain(zlang_emitter_t *emitter, uint32_t depth, bool is_reference) {
    for (uint32_t _index = 0U; _index < depth; ++_index) {
        if (!_index && is_reference) {
            if (!zlang_append_text(emitter, "->zlang_base_")) {
                return false;
            }
            continue;
        }
        if (!zlang_append_text(emitter, ".") || !zlang_append_text(emitter, "zlang_base_")) {
            return false;
        }
    }
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_expression_as(const zlang_program_t *program, size_t index,
                                     zlang_emitter_t *emitter, const char *expected_type,
                                     bool is_reference) {
    const char *const _actual_k = zlang_call_argument_type(program, emitter, index);
    if (expected_type == NULL || !zlang_is_struct_ancestor(program, expected_type, _actual_k)) {
        return is_reference ? zlang_emit_reference_argument(program, index, emitter)
                            : zlang_emit_expression(program, index, emitter);
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    const zlang_emitted_symbol_t *const _symbol_k =
        _expression_k->kind_ == zlang_expression_identifier_
            ? zlang_find_active_symbol(emitter, _expression_k->name_)
            : NULL;
    const char *const _type_k = zlang_generated_type(program, emitter, expected_type);
    const uint32_t _depth_k = zlang_struct_depth(program, expected_type, _actual_k);
    if (_type_k == NULL) {
        return false;
    }
    if (is_reference) {
        if (_symbol_k != NULL && _symbol_k->is_reference_) {
            return zlang_append_text(emitter, "(") && zlang_append_text(emitter, _type_k) &&
                   zlang_append_text(emitter, " *)") &&
                   zlang_append_text(emitter, _symbol_k->generated_name_);
        }
        if (_symbol_k != NULL) {
            return zlang_append_text(emitter, "&") &&
                   zlang_append_text(emitter, _symbol_k->generated_name_) &&
                   zlang_emit_base_chain(emitter, _depth_k, false);
        }
        return false;
    }
    if (_symbol_k != NULL && !_symbol_k->is_reference_) {
        return zlang_append_text(emitter, _symbol_k->generated_name_) &&
               zlang_emit_base_chain(emitter, _depth_k, false);
    }
    if (_symbol_k != NULL && _symbol_k->is_reference_) {
        return zlang_append_text(emitter, "(*") &&
               zlang_append_text(emitter, _symbol_k->generated_name_) &&
               zlang_append_text(emitter, ")") && zlang_emit_base_chain(emitter, _depth_k, false);
    }
    if (!zlang_emit_expression(program, index, emitter)) {
        return false;
    }
    return _depth_k == 0U || zlang_emit_base_chain(emitter, _depth_k, false);
}

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
        if (_expression_k->is_uint64_) {
            return zlang_append_u64(emitter, _expression_k->integer64_value_) &&
                   zlang_append_text(emitter, "ULL");
        }
        if (_expression_k->is_int64_) {
            return zlang_append_u64(emitter, _expression_k->integer64_value_) &&
                   zlang_append_text(emitter, "LL");
        }
        if (_expression_k->is_unsigned_) {
            return zlang_append_unsigned(emitter, (uint32_t)_expression_k->integer_value_) &&
                   zlang_append_text(emitter, "U");
        }
        return zlang_append_integer(emitter, _expression_k->integer_value_);
    case zlang_expression_float_literal_: {
        char _number_k[64];
        const int _written_k =
            snprintf(_number_k, sizeof(_number_k), _expression_k->is_float_ ? "%.9g" : "%.17g",
                     _expression_k->float_value_);
        if (_written_k < 0 || (size_t)_written_k >= sizeof(_number_k)) {
            return false;
        }
        bool _has_decimal_k = false;
        bool _has_exponent_k = false;
        for (char *_cursor_k = _number_k; *_cursor_k != '\0'; ++_cursor_k) {
            if (*_cursor_k == '.') {
                _has_decimal_k = true;
            } else if (*_cursor_k == 'e' || *_cursor_k == 'E') {
                _has_exponent_k = true;
            }
        }
        if (_has_exponent_k && !_has_decimal_k) {
            const char *_e_k = strchr(_number_k, 'e');
            if (_e_k == NULL) {
                _e_k = strchr(_number_k, 'E');
            }
            const size_t _mantissa_k = (size_t)(_e_k - _number_k);
            if (_mantissa_k + 2U >= sizeof(_number_k)) {
                return false;
            }
            memmove(_number_k + _mantissa_k + 2U, _number_k + _mantissa_k,
                    strlen(_number_k + _mantissa_k) + 1U);
            _number_k[_mantissa_k] = '.';
            _number_k[_mantissa_k + 1U] = '0';
        } else if (!_has_decimal_k) {
            const size_t _length_k = strlen(_number_k);
            if (_length_k + 2U >= sizeof(_number_k)) {
                return false;
            }
            memmove(_number_k + _length_k + 2U, _number_k + _length_k, 1U);
            _number_k[_length_k] = '.';
            _number_k[_length_k + 1U] = '0';
        }
        return zlang_append_text(emitter, _number_k) &&
               (!_expression_k->is_float_ || zlang_append_text(emitter, "f"));
    }
    case zlang_expression_conversion_: {
        const char *const _source_k = zlang_expression_type(program, emitter, _expression_k->left_);
        const bool _enum_source_k =
            _source_k != NULL && zlang_find_enum_statement(program, _source_k) != NULL;
        const char *const _function_k =
            _enum_source_k ? NULL : zlang_conversion_function(_expression_k->name_, _source_k);
        const char *const _c_type_k = zlang_c_type(_expression_k->name_);
        if (_c_type_k == NULL ||
            (_function_k != NULL && !zlang_append_text(emitter, _function_k))) {
            return false;
        }
        if (_function_k == NULL &&
            (!zlang_append_text(emitter, "(") || !zlang_append_text(emitter, _c_type_k) ||
             !zlang_append_text(emitter, ")"))) {
            return false;
        }
        return zlang_emit_expression(program, _expression_k->left_, emitter) &&
               (_function_k == NULL || zlang_append_text(emitter, ")"));
    }
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
        const zlang_ast_statement_t *_field_k = NULL;
        if (zlang_current_field(program, emitter, _expression_k->name_, &_field_k) &&
            emitter->current_self_name_ != NULL) {
            uint32_t _depth_k = 0U;
            const zlang_ast_statement_t *const _owner_k = zlang_find_field_owner(
                program, emitter->current_struct_, _expression_k->name_, &_depth_k, 0U);
            if (_owner_k == NULL) {
                return false;
            }
            if (_depth_k > 0U) {
                return zlang_append_text(emitter, emitter->current_self_name_) &&
                       zlang_emit_base_chain(emitter, _depth_k,
                                             emitter->current_self_is_pointer_) &&
                       zlang_append_text(emitter, ".") &&
                       zlang_append_field_name(emitter, _field_k->name_);
            }
            return zlang_append_text(emitter, emitter->current_self_name_) &&
                   zlang_append_text(emitter, emitter->current_self_is_pointer_ ? "->" : ".") &&
                   zlang_append_field_name(emitter, _field_k->name_);
        }
        const char *const _status_k = zlang_std_status_constant_c_name(_expression_k->name_);
        if (_status_k != NULL) {
            return zlang_append_text(emitter, _status_k);
        }
        const char *const _stream_k = zlang_std_stream_constant_c_name(_expression_k->name_);
        if (_stream_k != NULL) {
            return zlang_append_text(emitter, _stream_k);
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
        const bool _floating_k = _left_type_k != NULL && (strcmp(_left_type_k, "float") == 0 ||
                                                          strcmp(_left_type_k, "double") == 0);
        if (_floating_k) {
            const char *const _operator_k = zlang_arithmetic_operator(_expression_k->kind_);
            return _operator_k != NULL && zlang_append_text(emitter, "(") &&
                   zlang_emit_expression(program, _expression_k->left_, emitter) &&
                   zlang_append_text(emitter, " ") && zlang_append_text(emitter, _operator_k) &&
                   zlang_append_text(emitter, " ") &&
                   zlang_emit_expression(program, _expression_k->right_, emitter) &&
                   zlang_append_text(emitter, ")");
        }
        const char *const _function_k =
            zlang_arithmetic_function(_expression_k->kind_, _left_type_k);
        if (_function_k == NULL || !zlang_append_text(emitter, _function_k) ||
            !zlang_emit_expression(program, _expression_k->left_, emitter) ||
            !zlang_append_text(emitter, ", ") ||
            !zlang_emit_expression(program, _expression_k->right_, emitter) ||
            !zlang_append_text(emitter, ")")) {
            return false;
        }
        return true;
    }
    case zlang_expression_negative_: {
        const char *const _type_k = zlang_expression_type(program, emitter, _expression_k->left_);
        if (_type_k != NULL && strcmp(_type_k, "int32") == 0) {
            return zlang_append_text(emitter, "zlang_neg_i32(") &&
                   zlang_emit_expression(program, _expression_k->left_, emitter) &&
                   zlang_append_text(emitter, ")");
        }
        if (_type_k != NULL && strcmp(_type_k, "int64") == 0) {
            return zlang_append_text(emitter, "zlang_neg_i64(") &&
                   zlang_emit_expression(program, _expression_k->left_, emitter) &&
                   zlang_append_text(emitter, ")");
        }
        return zlang_append_text(emitter, "(-") &&
               zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, ")");
    }
    case zlang_expression_length_: {
        const zlang_emitted_symbol_t *const _symbol_k =
            zlang_find_active_symbol(emitter, _expression_k->name_);
        return zlang_append_array_extent(emitter, _symbol_k);
    }
    case zlang_expression_compound_assignment_: {
        const zlang_ast_expression_t *const _target_k =
            &program->expressions_[_expression_k->left_];
        const char *const _left_type_k = zlang_expression_type(program, emitter, index);
        const char *const _function_k =
            zlang_arithmetic_function(_expression_k->operator_, _left_type_k);
        if (_function_k == NULL) {
            return false;
        }
        const bool _field_target_k =
            zlang_find_active_symbol(emitter, _target_k->name_) == NULL &&
            emitter->program_ != NULL && emitter->current_struct_ != NULL &&
            zlang_find_field_statement(emitter->program_, emitter->current_struct_,
                                       _target_k->name_) != NULL;
        if (_field_target_k) {
            return zlang_emit_symbol_lvalue(emitter, _target_k->name_) &&
                   zlang_append_text(emitter, " = ") && zlang_append_text(emitter, _function_k) &&
                   zlang_emit_symbol_value(emitter, _target_k->name_) &&
                   zlang_append_text(emitter, ", ") &&
                   zlang_emit_expression(program, _expression_k->right_, emitter) &&
                   zlang_append_text(emitter, ")");
        }
        if (!zlang_emit_symbol_lvalue(emitter, _target_k->name_) ||
            !zlang_append_text(emitter, " = ") || !zlang_append_text(emitter, _function_k) ||
            !zlang_emit_symbol_value(emitter, _target_k->name_) ||
            !zlang_append_text(emitter, ", ") ||
            !zlang_emit_expression(program, _expression_k->right_, emitter) ||
            !zlang_append_text(emitter, ")")) {
            return false;
        }
        return true;
    }
    case zlang_expression_match_: {
        const size_t _count_k = _expression_k->argument_count_;
        if (_count_k < 2U || (_count_k % 2U) != 0U || !zlang_append_text(emitter, "(")) {
            return false;
        }
        for (size_t _arm = 0U; _arm < _count_k; _arm += 2U) {
            const size_t _pattern_index_k = _expression_k->argument_indices_[_arm];
            const size_t _value_index_k = _expression_k->argument_indices_[_arm + 1U];
            const zlang_ast_expression_t *const _pattern_k =
                _pattern_index_k < program->expression_count_
                    ? &program->expressions_[_pattern_index_k]
                    : NULL;
            const bool _wildcard_k =
                _pattern_k != NULL && _pattern_k->kind_ == zlang_expression_wildcard_;
            if (_wildcard_k) {
                if (!zlang_emit_expression(program, _value_index_k, emitter)) {
                    return false;
                }
                break;
            }
            if (!zlang_emit_expression(program, _expression_k->left_, emitter) ||
                !zlang_append_text(emitter, " == ") ||
                !zlang_emit_expression(program, _pattern_index_k, emitter) ||
                !zlang_append_text(emitter, " ? ") ||
                !zlang_emit_expression(program, _value_index_k, emitter) ||
                !zlang_append_text(emitter, " : ")) {
                return false;
            }
        }
        return zlang_append_text(emitter, ")");
    }
    case zlang_expression_range_:
    case zlang_expression_wildcard_:
        return false;
    case zlang_expression_equal_:
    case zlang_expression_less_:
    case zlang_expression_greater_:
    case zlang_expression_less_equal_:
    case zlang_expression_greater_equal_:
    case zlang_expression_not_equal_: {
        return zlang_emit_comparison(program, _expression_k, emitter);
    }
    case zlang_expression_ternary_: {
        if (_expression_k->argument_count_ != 1U || !zlang_append_text(emitter, "(") ||
            !zlang_emit_expression(program, _expression_k->left_, emitter) ||
            !zlang_append_text(emitter, " ? ") ||
            !zlang_emit_expression(program, _expression_k->right_, emitter) ||
            !zlang_append_text(emitter, " : ") ||
            !zlang_emit_expression(program, _expression_k->argument_indices_[0], emitter) ||
            !zlang_append_text(emitter, ")")) {
            return false;
        }
        return true;
    }
    case zlang_expression_bit_not_:
        return zlang_append_text(emitter, "~") &&
               zlang_emit_expression(program, _expression_k->left_, emitter);
    case zlang_expression_bit_and_:
    case zlang_expression_bit_or_:
    case zlang_expression_bit_xor_:
    case zlang_expression_shift_left_:
    case zlang_expression_shift_right_: {
        const char *_operator_k = " >> ";
        if (_expression_k->kind_ == zlang_expression_bit_and_) {
            _operator_k = " & ";
        } else if (_expression_k->kind_ == zlang_expression_bit_or_) {
            _operator_k = " | ";
        } else if (_expression_k->kind_ == zlang_expression_bit_xor_) {
            _operator_k = " ^ ";
        } else if (_expression_k->kind_ == zlang_expression_shift_left_) {
            _operator_k = " << ";
        }
        return zlang_append_text(emitter, "(") &&
               zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, _operator_k) &&
               zlang_emit_expression(program, _expression_k->right_, emitter) &&
               zlang_append_text(emitter, ")");
    }
    case zlang_expression_not_: {
        const bool _parenthesize_k =
            zlang_expression_is_comparison(program->expressions_[_expression_k->left_].kind_);
        return zlang_append_text(emitter, _parenthesize_k ? "(!(" : "(!") &&
               zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, _parenthesize_k ? "))" : ")");
    }
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
        const zlang_emitted_symbol_t *const _value_symbol_k =
            _value_k->kind_ == zlang_expression_identifier_
                ? zlang_find_active_symbol(emitter, _value_k->name_)
                : NULL;
        if (_symbol_k != NULL && _symbol_k->is_array_ && _value_symbol_k != NULL &&
            _value_symbol_k->is_array_) {
            return zlang_append_text(emitter, "memcpy(") &&
                   zlang_append_text(emitter,
                                     _symbol_k->is_reference_ ? _symbol_k->generated_name_ : "&") &&
                   (!_symbol_k->is_reference_
                        ? zlang_append_text(emitter, _symbol_k->generated_name_)
                        : true) &&
                   zlang_append_text(emitter, ", ") &&
                   zlang_append_text(emitter, _value_symbol_k->is_reference_
                                                  ? _value_symbol_k->generated_name_
                                                  : "&") &&
                   (!_value_symbol_k->is_reference_
                        ? zlang_append_text(emitter, _value_symbol_k->generated_name_)
                        : true) &&
                   zlang_append_text(emitter, ", sizeof(") &&
                   zlang_append_text(emitter, _symbol_k->is_reference_ ? "*" : "") &&
                   zlang_append_text(emitter, _symbol_k->generated_name_) &&
                   zlang_append_text(emitter, "))");
        }
        const char *_target_type_k = zlang_expression_type(program, emitter, _expression_k->left_);
        if (_target_type_k == NULL && _target_k->kind_ == zlang_expression_identifier_ &&
            emitter->current_struct_ != NULL) {
            uint32_t _field_depth_k = 0U;
            const zlang_ast_statement_t *const _field_k = zlang_find_field_owner(
                program, emitter->current_struct_, _target_k->name_, &_field_depth_k, 0U);
            if (_field_k != NULL) {
                _target_type_k = _field_k->type_name_;
            }
        }
        return zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, " = ") &&
               (_target_type_k == NULL
                    ? zlang_emit_expression(program, _expression_k->right_, emitter)
                    : zlang_emit_expression_as(program, _expression_k->right_, emitter,
                                               _target_type_k, false));
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
            if (_symbol_k == NULL && emitter->current_self_name_ != NULL &&
                zlang_same_text(_field_k->base_name_, emitter->current_self_name_) &&
                emitter->current_struct_ != NULL) {
                uint32_t _self_field_depth_k = 0U;
                const zlang_ast_statement_t *const _field_statement_k = zlang_find_field_owner(
                    program, emitter->current_struct_, _field_k->name_, &_self_field_depth_k, 0U);
                if (_field_statement_k == NULL) {
                    return false;
                }
                return zlang_append_text(emitter, emitter->current_self_name_) &&
                       zlang_append_text(emitter, emitter->current_self_is_pointer_ ? "->" : ".") &&
                       zlang_append_field_name(emitter, _field_k->name_);
            }
            if (_symbol_k == NULL || _generated_type_k == NULL) {
                return false;
            }
            const zlang_ast_statement_t *const _record_k =
                zlang_find_struct_statement(program, _symbol_k->type_name_);
            uint32_t _field_depth_k = 0U;
            const zlang_ast_statement_t *const _field_statement_k =
                zlang_find_field_owner(program, _record_k, _field_k->name_, &_field_depth_k, 0U);
            if (_field_statement_k != NULL &&
                _field_statement_k->kind_ == zlang_statement_array_declaration_ &&
                _field_k->argument_count_ > 0U) {
                return zlang_emit_array_field_call(
                    program, emitter, _field_k, _field_statement_k, _symbol_k,
                    _expression_k->kind_ == zlang_expression_field_assignment_,
                    _expression_k->right_);
            }
            if (_field_k->right_ != ZLANG_NO_EXPRESSION) {
                if (!zlang_append_text(emitter, "(*((") ||
                    !zlang_append_text(emitter, _generated_type_k) ||
                    !zlang_append_text(emitter, " *)zlang_array_get(") ||
                    !zlang_append_text(emitter, _symbol_k->generated_name_) ||
                    !zlang_append_text(emitter, ".data_, ") ||
                    !zlang_emit_index_expression(program, _field_k->right_, emitter) ||
                    !zlang_append_text(emitter, ", ") ||
                    !zlang_append_array_extent(emitter, _symbol_k) ||
                    !zlang_append_text(emitter, ", sizeof(") ||
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
            if (_field_k->right_ == ZLANG_NO_EXPRESSION && _field_depth_k > 0U) {
                if (!zlang_emit_base_chain(emitter, _field_depth_k, _symbol_k->is_reference_)) {
                    return false;
                }
                _separator_k = ".";
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
    case zlang_expression_view_: {
        const zlang_ast_expression_t *const _source_k =
            &program->expressions_[_expression_k->argument_indices_[0]];
        const zlang_emitted_symbol_t *const _symbol_k =
            _source_k->kind_ == zlang_expression_identifier_
                ? zlang_find_active_symbol(emitter, _source_k->name_)
                : NULL;
        if (_symbol_k == NULL || !_symbol_k->is_array_ || _symbol_k->array_rank_ != 1U ||
            !zlang_activate_macro(emitter, "ZLANG_STD_VIEW")) {
            return false;
        }
        return zlang_append_text(emitter, "zlang_view_make(") &&
               zlang_append_text(emitter, _symbol_k->generated_name_) &&
               zlang_append_text(emitter, ".data_, ") &&
               zlang_append_array_extent(emitter, _symbol_k) && zlang_append_text(emitter, ", ") &&
               zlang_emit_expression(program, _expression_k->argument_indices_[1], emitter) &&
               zlang_append_text(emitter, ", ") &&
               zlang_emit_expression(program, _expression_k->argument_indices_[2], emitter) &&
               zlang_append_text(emitter, ", sizeof(") &&
               zlang_append_text(emitter, _symbol_k->generated_name_) &&
               zlang_append_text(emitter, ".data_[0]))");
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
        char _element_k[ZLANG_GENERIC_NAME_CAPACITY];
        if (zlang_std_view_element_type(_symbol_k->type_name_, _element_k, sizeof(_element_k))) {
            const char *const _generated_k = zlang_generated_type(program, emitter, _element_k);
            return _generated_k != NULL && zlang_append_text(emitter, "(*(") &&
                   zlang_append_text(emitter, _generated_k) &&
                   zlang_append_text(emitter, " *)zlang_array_get(") &&
                   zlang_append_text(emitter, _name_k) && zlang_append_text(emitter, ".data_, ") &&
                   zlang_emit_index_expression(program, _expression_k->right_, emitter) &&
                   zlang_append_text(emitter, ", ") && zlang_append_text(emitter, _name_k) &&
                   zlang_append_text(emitter, ".length_, ") &&
                   zlang_append_text(emitter, _name_k) &&
                   zlang_append_text(emitter, ".element_size_))");
        }
        if (_expression_k->argument_count_ > 1U) {
            return zlang_emit_array_typed_call(program, emitter, _symbol_k, _expression_k, false,
                                               ZLANG_NO_EXPRESSION);
        }
        if (_expression_k->is_proven_ && _symbol_k->is_sequence_) {
            return zlang_append_text(emitter, _name_k) && zlang_append_text(emitter, ".data_[") &&
                   zlang_emit_index_expression(program, _expression_k->right_, emitter) &&
                   zlang_append_text(emitter, "]");
        }
        if (!_symbol_k->is_sequence_ && strcmp(_symbol_k->type_name_, "string") == 0) {
            return zlang_append_text(emitter, "zlang_string_byte(") &&
                   zlang_append_text(emitter, _name_k) && zlang_append_text(emitter, ", ") &&
                   zlang_emit_index_expression(program, _expression_k->right_, emitter) &&
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
                !zlang_emit_index_expression(program, _expression_k->right_, emitter) ||
                !zlang_append_text(emitter, ", ") ||
                !zlang_append_array_extent(emitter, _symbol_k) ||
                !zlang_append_text(emitter, ", sizeof(") || !zlang_append_text(emitter, _name_k) ||
                !zlang_append_text(emitter, ".data_[0]))))")) {
                return false;
            }
            return true;
        }
        if (!zlang_append_text(emitter, _function_k) || !zlang_append_text(emitter, _name_k) ||
            !zlang_append_text(emitter, ".data_, ") ||
            !zlang_emit_index_expression(program, _expression_k->right_, emitter) ||
            !zlang_append_text(emitter, ", ") || !zlang_append_array_extent(emitter, _symbol_k) ||
            !zlang_append_text(emitter, ")")) {
            return false;
        }
        return true;
    }
    case zlang_expression_construct_: {
        char _name_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_method_generated_name(_expression_k->name_, "constructor", true, _name_k,
                                         sizeof(_name_k)) ||
            !zlang_append_text(emitter, _name_k) || !zlang_append_text(emitter, "(")) {
            return false;
        }
        for (size_t _argument = 0U;
             _argument < _expression_k->argument_count_ && _argument < ZLANG_CALL_ARGUMENT_CAPACITY;
             ++_argument) {
            if ((_argument > 0U && !zlang_append_text(emitter, ", ")) ||
                !zlang_emit_expression(program, _expression_k->argument_indices_[_argument],
                                       emitter)) {
                return false;
            }
        }
        return zlang_append_text(emitter, ")");
    }
    case zlang_expression_method_call_: {
        const zlang_ast_statement_t *const _static_owner_k =
            zlang_find_struct_statement(program, _expression_k->base_name_);
        const bool _static_k = _static_owner_k != NULL;
        const zlang_emitted_symbol_t *_receiver_symbol_k = NULL;
        const char *_receiver_type_k = NULL;
        if (_static_k) {
            _receiver_type_k = _expression_k->base_name_;
        } else {
            _receiver_symbol_k = zlang_find_active_symbol(emitter, _expression_k->base_name_);
            if (_receiver_symbol_k == NULL) {
                return false;
            }
            _receiver_type_k = _receiver_symbol_k->type_name_;
        }
        const zlang_std_result_t *const _native_result_k =
            _receiver_type_k != NULL ? zlang_std_find_result(_receiver_type_k) : NULL;
        if (_native_result_k != NULL) {
            const char *const _method_type_k =
                zlang_std_native_result_method_type(_native_result_k, _expression_k->name_);
            if (_expression_k->argument_count_ != 0U || _method_type_k == NULL ||
                _receiver_symbol_k == NULL) {
                return false;
            }
            const bool _is_ok_k = strcmp(_expression_k->name_, "is_ok") == 0;
            const bool _is_message_k = strcmp(_expression_k->name_, "message") == 0;
            const bool _is_time_k = strcmp(_expression_k->name_, "time") == 0;
            const bool _is_dump_k = strcmp(_expression_k->name_, "dump") == 0;
            if (_is_time_k || _is_dump_k) {
                const char *const _tag_k = zlang_std_native_result_tag(_native_result_k);
                if (_tag_k == NULL || !zlang_append_text(emitter, "zlang_std_result_") ||
                    !zlang_append_text(emitter, _tag_k) ||
                    !zlang_append_text(emitter, _is_time_k ? "_time(" : "_dump(") ||
                    (_is_dump_k && !zlang_append_text(emitter, "&")) ||
                    !zlang_emit_symbol_value(emitter, _receiver_symbol_k->source_name_) ||
                    !zlang_append_text(emitter, ")")) {
                    return false;
                }
                if (_is_time_k) {
                    emitter->emit_timing_ = true;
                } else {
                    emitter->emit_dump_ = true;
                }
                return true;
            }
            if (!_is_ok_k && !_is_message_k && strcmp(_expression_k->name_, "get") != 0) {
                return false;
            }
            const char *_field_k = "value";
            if (_is_ok_k) {
                _field_k = "code";
            } else if (_is_message_k) {
                _field_k = "what";
            }
            if (_is_ok_k && !zlang_append_text(emitter, "zlang_std_status_is_ok(")) {
                return false;
            }
            if (!zlang_emit_symbol_value(emitter, _receiver_symbol_k->source_name_) ||
                !zlang_append_text(emitter, ".") || !zlang_append_field_name(emitter, _field_k)) {
                return false;
            }
            return !_is_ok_k || zlang_append_text(emitter, ")");
        }
        const zlang_emitted_user_result_t *const _user_result_k =
            zlang_find_user_result(emitter, _receiver_type_k);
        if (_user_result_k != NULL) {
            if (_expression_k->argument_count_ != 0U || _receiver_symbol_k == NULL) {
                return false;
            }
            const bool _is_ok_k = strcmp(_expression_k->name_, "is_ok") == 0;
            const bool _is_message_k = strcmp(_expression_k->name_, "message") == 0;
            const bool _is_time_k = strcmp(_expression_k->name_, "time") == 0;
            const bool _is_dump_k = strcmp(_expression_k->name_, "dump") == 0;
            const bool _is_get_k = strcmp(_expression_k->name_, "get") == 0;
            if (!_is_ok_k && !_is_message_k && !_is_time_k && !_is_dump_k && !_is_get_k) {
                return false;
            }
            if (_is_time_k || _is_dump_k) {
                char _helper_k[ZLANG_EMITTED_NAME_CAPACITY];
                if (_is_time_k) {
                    const size_t _length_k = strlen(_user_result_k->type_name_);
                    if (_length_k + 6U > sizeof(_helper_k)) {
                        return false;
                    }
                    memcpy(_helper_k, _user_result_k->type_name_, _length_k);
                    memcpy(_helper_k + _length_k, "_time", 6U);
                } else {
                    zlang_copy_name(_user_result_k->dump_name_, _helper_k, sizeof(_helper_k));
                }
                if (!zlang_append_text(emitter, _helper_k) || !zlang_append_text(emitter, "(") ||
                    (_is_dump_k && !zlang_append_text(emitter, "&")) ||
                    !zlang_emit_symbol_value(emitter, _receiver_symbol_k->source_name_) ||
                    !zlang_append_text(emitter, ")")) {
                    return false;
                }
                if (_is_time_k) {
                    emitter->emit_timing_ = true;
                } else {
                    emitter->emit_dump_ = true;
                }
                return true;
            }
            const char *_field_k = "value";
            if (_is_ok_k) {
                _field_k = "code";
            } else if (_is_message_k) {
                _field_k = "what";
            }
            if (_is_ok_k && !zlang_append_text(emitter, "zlang_std_status_is_ok(")) {
                return false;
            }
            if (!zlang_emit_symbol_value(emitter, _receiver_symbol_k->source_name_) ||
                !zlang_append_text(emitter, ".") || !zlang_append_field_name(emitter, _field_k)) {
                return false;
            }
            return !_is_ok_k || zlang_append_text(emitter, ")");
        }
        const char *_owner_k = NULL;
        bool _is_trait_k = false;
        const zlang_ast_statement_t *const _method_k = zlang_find_method(
            program, _receiver_type_k, _expression_k->name_, &_owner_k, &_is_trait_k);
        if (_method_k == NULL || _owner_k == NULL) {
            return false;
        }
        const zlang_emitted_user_type_t *const _owner_type_k =
            zlang_find_user_type(emitter, _owner_k);
        if (_owner_type_k == NULL) {
            return false;
        }
        char _name_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_method_generated_name(_owner_k, _expression_k->name_, false, _name_k,
                                         sizeof(_name_k))) {
            return false;
        }
        const bool _virtual_k = !_static_k && !_is_trait_k && !_method_k->is_static_ &&
                                !_method_k->is_constructor_ &&
                                zlang_struct_is_polymorphic(program, _receiver_type_k);
        if (_virtual_k) {
            const zlang_ast_statement_t *const _root_k =
                zlang_struct_root_name(program, _receiver_type_k);
            const zlang_emitted_user_type_t *const _root_type_k =
                zlang_find_user_type(emitter, _root_k != NULL ? _root_k->name_ : NULL);
            char _slot_k[ZLANG_EMITTED_NAME_CAPACITY];
            if (_root_type_k == NULL ||
                !zlang_slot_name(_expression_k->name_, _slot_k, sizeof(_slot_k))) {
                return false;
            }
            if (!zlang_append_text(emitter, "((") ||
                !zlang_append_text(emitter, _root_type_k->type_name_) ||
                !zlang_append_text(emitter, " *)")) {
                return false;
            }
            if (_receiver_symbol_k->is_reference_) {
                if (!zlang_append_text(emitter, _receiver_symbol_k->generated_name_)) {
                    return false;
                }
            } else {
                if (!zlang_append_text(emitter, "&") ||
                    !zlang_append_text(emitter, _receiver_symbol_k->generated_name_)) {
                    return false;
                }
            }
            if (!zlang_append_text(emitter, ")->zlang_vtable_->") ||
                !zlang_append_text(emitter, _slot_k) || !zlang_append_text(emitter, "(") ||
                !zlang_append_text(emitter, "(") ||
                !zlang_append_text(emitter, _root_type_k->type_name_) ||
                !zlang_append_text(emitter, " *)")) {
                return false;
            }
            if (_receiver_symbol_k->is_reference_) {
                if (!zlang_append_text(emitter, _receiver_symbol_k->generated_name_)) {
                    return false;
                }
            } else {
                if (!zlang_append_text(emitter, "&") ||
                    !zlang_append_text(emitter, _receiver_symbol_k->generated_name_)) {
                    return false;
                }
            }
        } else {
            if (!zlang_append_text(emitter, _name_k) || !zlang_append_text(emitter, "(")) {
                return false;
            }
            if (!_static_k) {
                if (_receiver_symbol_k->is_reference_) {
                    if (!zlang_append_text(emitter, "(") ||
                        !zlang_append_text(emitter, _owner_type_k->type_name_) ||
                        !zlang_append_text(emitter, " *)") ||
                        !zlang_append_text(emitter, _receiver_symbol_k->generated_name_)) {
                        return false;
                    }
                } else {
                    const uint32_t _depth_k =
                        zlang_struct_depth(program, _owner_k, _receiver_symbol_k->type_name_);
                    if (!zlang_append_text(emitter, "&") ||
                        !zlang_append_text(emitter, _receiver_symbol_k->generated_name_) ||
                        !zlang_emit_base_chain(emitter, _depth_k, false)) {
                        return false;
                    }
                }
            }
        }
        for (size_t _argument = 0U;
             _argument < _expression_k->argument_count_ && _argument < ZLANG_CALL_ARGUMENT_CAPACITY;
             ++_argument) {
            const zlang_parameter_t *_parameter_k = NULL;
            if (_argument < _method_k->parameter_count_ &&
                _method_k->parameter_start_ + _argument < program->parameter_count_) {
                _parameter_k = &program->parameters_[_method_k->parameter_start_ + _argument];
            }
            if ((_argument > 0U || !_static_k) && !zlang_append_text(emitter, ", ")) {
                return false;
            }
            if (!zlang_emit_expression_as(program, _expression_k->argument_indices_[_argument],
                                          emitter,
                                          _parameter_k != NULL ? _parameter_k->type_name_ : NULL,
                                          _parameter_k != NULL && _parameter_k->is_reference_)) {
                return false;
            }
        }
        return zlang_append_text(emitter, ")");
    }
    case zlang_expression_call_: {
        const zlang_std_function_t *const _std_k =
            zlang_find_import_statement(program, _expression_k->name_) != NULL
                ? zlang_std_call(program, emitter, _expression_k)
                : NULL;
        if (_std_k != NULL) {
            if (!zlang_append_text(emitter, _std_k->c_symbol_) ||
                !zlang_append_text(emitter, "(")) {
                return false;
            }
            if (_std_k->is_stateful_ &&
                !zlang_append_text(emitter,
                                   emitter->in_function_ ? "zlang_context" : "&_zlang_context")) {
                return false;
            }
            for (size_t _argument = 0U; _argument < _expression_k->argument_count_; ++_argument) {
                if ((_argument > 0U || _std_k->is_stateful_) && !zlang_append_text(emitter, ", ")) {
                    return false;
                }
                const size_t _argument_index_k = _expression_k->argument_indices_[_argument];
                const bool _reference_k = _argument < _std_k->parameter_count_ &&
                                          _std_k->parameter_is_reference_[_argument];
                const bool _argument_ok_k =
                    _reference_k
                        ? zlang_emit_reference_argument(program, _argument_index_k, emitter)
                        : zlang_emit_expression(program, _argument_index_k, emitter);
                if (!_argument_ok_k) {
                    return false;
                }
                bool _destination_just_emitted_k = false;
                if (_argument < _std_k->parameter_count_ &&
                    _std_k->parameter_is_reference_[_argument] &&
                    _std_k->parameter_types_[_argument] != NULL &&
                    strcmp(_std_k->parameter_types_[_argument], "string") == 0 &&
                    (strcmp(_std_k->name_, "read") == 0 ||
                     strcmp(_std_k->name_, "read_line") == 0)) {
                    _destination_just_emitted_k = true;
                }
                if (_destination_just_emitted_k) {
                    const zlang_ast_expression_t *const _destination_k =
                        &program->expressions_[_argument_index_k];
                    const zlang_emitted_symbol_t *const _symbol_k =
                        zlang_find_active_symbol(emitter, _destination_k->name_);
                    if (_symbol_k == NULL || _symbol_k->storage_name_[0] == '\0' ||
                        _symbol_k->capacity_ == 0U || !zlang_append_text(emitter, ", ") ||
                        !zlang_append_text(emitter, _symbol_k->storage_name_) ||
                        !zlang_append_text(emitter, ", ") ||
                        !zlang_append_unsigned(emitter, _symbol_k->capacity_)) {
                        return false;
                    }
                }
            }
            return zlang_append_text(emitter, ")");
        }
        char _call_name_k[ZLANG_EMITTED_NAME_CAPACITY];
        const zlang_array_specialization_t *const _call_specialization_k =
            zlang_find_call_specialization(program, emitter, _expression_k);
        const zlang_ast_statement_t *const _call_function_k =
            zlang_find_function_statement(program, _expression_k->name_);
        if (_call_function_k == NULL) {
            return false;
        }
        if (_call_function_k->is_extern_) {
            if (!zlang_copy_name(_call_function_k->name_, _call_name_k, sizeof(_call_name_k))) {
                return false;
            }
        } else if (!zlang_function_generated_name(program, emitter, _call_function_k,
                                                  _call_specialization_k, _call_name_k,
                                                  sizeof(_call_name_k))) {
            return false;
        }
        if (!zlang_append_text(emitter, _call_name_k) || !zlang_append_text(emitter, "(")) {
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
            if (!zlang_emit_expression_as(program, _index_k, emitter,
                                          _parameter_k != NULL ? _parameter_k->type_name_ : NULL,
                                          _parameter_k != NULL && _parameter_k->is_reference_)) {
                return false;
            }
        }
        if (zlang_function_marked(emitter, _expression_k->name_) &&
            ((_expression_k->argument_count_ > 0U && !zlang_append_text(emitter, ", ")) ||
             !zlang_append_text(emitter,
                                emitter->in_function_ ? "zlang_context" : "&_zlang_context"))) {
            return false;
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
    if (_expression_k->kind_ == zlang_expression_string_literal_) {
        return zlang_append_text(emitter, "&") && zlang_emit_literal_view(program, index, emitter);
    }
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
               zlang_emit_index_expression(program, _expression_k->right_, emitter) &&
               zlang_append_text(emitter, ", ") && zlang_append_array_extent(emitter, _symbol_k) &&
               zlang_append_text(emitter, ", sizeof(") &&
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
            uint32_t _depth_k = 0U;
            const zlang_ast_statement_t *const _owner_k = zlang_find_field_owner(
                program, zlang_find_struct_statement(program, _symbol_k->type_name_),
                _field_k->name_, &_depth_k, 0U);
            if (_owner_k == NULL) {
                return false;
            }
            return zlang_append_text(emitter, "&") &&
                   zlang_append_text(emitter, _symbol_k->generated_name_) &&
                   zlang_append_text(emitter, _symbol_k->is_reference_ ? "->" : ".") &&
                   zlang_emit_base_chain(emitter, _depth_k, false) &&
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
static bool zlang_array_type_suffix(const char *type_name, char *output, size_t capacity) {
    if (type_name == NULL || capacity < 4U) {
        return false;
    }
    if (strcmp(type_name, "int32_t") == 0) {
        return zlang_copy_name("i32", output, capacity);
    }
    if (strcmp(type_name, "uint32_t") == 0) {
        return zlang_copy_name("u32", output, capacity);
    }
    if (strcmp(type_name, "int64_t") == 0) {
        return zlang_copy_name("i64", output, capacity);
    }
    if (strcmp(type_name, "uint64_t") == 0) {
        return zlang_copy_name("u64", output, capacity);
    }
    if (strcmp(type_name, "float") == 0) {
        return zlang_copy_name("f32", output, capacity);
    }
    if (strcmp(type_name, "double") == 0) {
        return zlang_copy_name("f64", output, capacity);
    }
    if (strcmp(type_name, "bool") == 0) {
        return zlang_copy_name("bool", output, capacity);
    }
    if (strcmp(type_name, "zlang_string_t") == 0) {
        return zlang_copy_name("string", output, capacity);
    }
    const char *const _base_k = strncmp(type_name, "zlang_", 6U) == 0 ? type_name + 6U : type_name;
    size_t _length_k = strlen(_base_k);
    if (_length_k > 3U && strcmp(_base_k + _length_k - 3U, "_t") == 0) {
        _length_k -= 3U;
    }
    if (_length_k == 0U || _length_k + 1U >= capacity) {
        return false;
    }
    memcpy(output, _base_k, _length_k);
    output[_length_k] = '\0';
    return true;
}

static const zlang_ast_statement_t *zlang_find_array_decl(const zlang_program_t *program,
                                                          const char *name) {
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_array_declaration_ &&
            _statement_k->name_ != NULL && strcmp(_statement_k->name_, name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

static size_t zlang_find_function_index(const zlang_program_t *program, const char *name) {
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_function_ && _statement_k->name_ != NULL &&
            strcmp(_statement_k->name_, name) == 0) {
            return _index;
        }
    }
    return ZLANG_NO_STATEMENT;
}

static bool zlang_function_has_inferred_array(const zlang_program_t *program,
                                              const zlang_ast_statement_t *function) {
    if (function->is_array_return_) {
        return true;
    }
    const size_t _end_k = function->parameter_start_ + function->parameter_count_;
    for (size_t _index = function->parameter_start_;
         _index < _end_k && _index < program->parameter_count_; ++_index) {
        if (program->parameters_[_index].is_array_) {
            return true;
        }
    }
    return false;
}

static void zlang_add_specialization(zlang_emitter_t *emitter, size_t function_index,
                                     const zlang_ast_statement_t *shape) {
    if (shape == NULL || emitter->array_specialization_count_ >= ZLANG_MAX_ARRAY_SPECIALIZATIONS) {
        return;
    }
    for (size_t _index = 0U; _index < emitter->array_specialization_count_; ++_index) {
        const zlang_array_specialization_t *const _entry_k =
            &emitter->array_specializations_[_index];
        if (_entry_k->function_statement_ != function_index || _entry_k->rank_ != shape->rank_ ||
            (_entry_k->type_name_ == NULL) != (shape->type_name_ == NULL) ||
            (_entry_k->type_name_ != NULL &&
             strcmp(_entry_k->type_name_, shape->type_name_) != 0)) {
            continue;
        }
        bool _same_k = true;
        for (uint32_t _axis = 0U; _axis < shape->rank_; ++_axis) {
            if (_entry_k->extents_[_axis] != shape->extents_[_axis]) {
                _same_k = false;
                break;
            }
        }
        if (_same_k) {
            return;
        }
    }
    zlang_array_specialization_t *const _entry_k =
        &emitter->array_specializations_[emitter->array_specialization_count_++];
    _entry_k->function_statement_ = function_index;
    _entry_k->type_name_ = shape->type_name_;
    _entry_k->rank_ = shape->rank_;
    for (uint32_t _axis = 0U; _axis < shape->rank_; ++_axis) {
        _entry_k->extents_[_axis] = shape->extents_[_axis];
    }
}

static void zlang_mark_canonical_shape(const zlang_program_t *program,
                                       const zlang_ast_statement_t *target) {
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        if (&program->statements_[_index] == target) {
            program->statements_[_index].use_canonical_type_ = true;
            return;
        }
    }
}

static bool zlang_collect_array_specializations(const zlang_program_t *program,
                                                zlang_emitter_t *emitter) {
    for (size_t _expression = 0U; _expression < program->expression_count_; ++_expression) {
        const zlang_ast_expression_t *const _call_k = &program->expressions_[_expression];
        if (_call_k->kind_ != zlang_expression_call_ || _call_k->name_ == NULL) {
            continue;
        }
        const size_t _function_index_k = zlang_find_function_index(program, _call_k->name_);
        if (_function_index_k == ZLANG_NO_STATEMENT ||
            !zlang_function_has_inferred_array(program, &program->statements_[_function_index_k])) {
            continue;
        }
        for (size_t _argument = 0U; _argument < _call_k->argument_count_; ++_argument) {
            const size_t _argument_k = _call_k->argument_indices_[_argument];
            if (_argument_k >= program->expression_count_ ||
                program->expressions_[_argument_k].kind_ != zlang_expression_identifier_) {
                continue;
            }
            const zlang_ast_statement_t *const _shape_k =
                zlang_find_array_decl(program, program->expressions_[_argument_k].name_);
            if (_shape_k != NULL) {
                zlang_mark_canonical_shape(program, _shape_k);
                zlang_add_specialization(emitter, _function_index_k, _shape_k);
                break;
            }
        }
        if (!zlang_function_has_inferred_array(program, &program->statements_[_function_index_k])) {
            continue;
        }
        for (size_t _statement = 0U; _statement < program->statement_count_; ++_statement) {
            const zlang_ast_statement_t *const _candidate_k = &program->statements_[_statement];
            if (_candidate_k->kind_ == zlang_statement_array_declaration_ &&
                _candidate_k->expression_ == _expression) {
                zlang_mark_canonical_shape(program, _candidate_k);
                zlang_add_specialization(emitter, _function_index_k, _candidate_k);
                break;
            }
        }
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _function_k = &program->statements_[_index];
        if (_function_k->kind_ != zlang_statement_function_ ||
            !zlang_function_has_inferred_array(program, _function_k) ||
            zlang_find_specialization(emitter, _index) != NULL) {
            continue;
        }
        zlang_ast_statement_t _shape_k = {0};
        _shape_k.type_name_ = _function_k->type_name_;
        _shape_k.rank_ =
            _function_k->is_array_return_ ? _function_k->return_rank_ : _function_k->rank_;
        for (uint32_t _axis = 0U; _axis < _shape_k.rank_; ++_axis) {
            _shape_k.extents_[_axis] = _function_k->is_array_return_
                                           ? _function_k->return_extents_[_axis]
                                           : _function_k->extents_[_axis];
        }
        zlang_add_specialization(emitter, _index, &_shape_k);
    }
    return true;
}

static const zlang_array_specialization_t *zlang_find_specialization(const zlang_emitter_t *emitter,
                                                                     size_t function_index) {
    for (size_t _index = 0U; _index < emitter->array_specialization_count_; ++_index) {
        if (emitter->array_specializations_[_index].function_statement_ == function_index) {
            return &emitter->array_specializations_[_index];
        }
    }
    return NULL;
}

static bool zlang_register_function_names(const zlang_program_t *program,
                                          zlang_emitter_t *emitter) {
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _function_k = &program->statements_[_index];
        if (_function_k->kind_ != zlang_statement_function_ || zlang_is_nested(program, _index) ||
            zlang_is_generic_template(_function_k)) {
            continue;
        }
        size_t _specialization_count_k = 0U;
        for (size_t _spec = 0U; _spec < emitter->array_specialization_count_; ++_spec) {
            if (emitter->array_specializations_[_spec].function_statement_ == _index) {
                ++_specialization_count_k;
            }
        }
        const size_t _iterations_k = _specialization_count_k == 0U ? 1U : _specialization_count_k;
        for (size_t _iteration = 0U; _iteration < _iterations_k; ++_iteration) {
            const zlang_array_specialization_t *_specialization_k = NULL;
            if (_specialization_count_k > 0U) {
                size_t _seen_k = 0U;
                for (size_t _spec = 0U; _spec < emitter->array_specialization_count_; ++_spec) {
                    if (emitter->array_specializations_[_spec].function_statement_ != _index) {
                        continue;
                    }
                    if (_seen_k == _iteration) {
                        _specialization_k = &emitter->array_specializations_[_spec];
                        break;
                    }
                    ++_seen_k;
                }
            }
            if (emitter->emitted_function_count_ >= ZLANG_EMITTED_SYMBOL_CAPACITY) {
                return false;
            }
            zlang_emitted_function_t *const _entry_k =
                &emitter->emitted_functions_[emitter->emitted_function_count_];
            if (_function_k->is_extern_) {
                if (!zlang_copy_name(_function_k->name_, _entry_k->generated_name_,
                                     sizeof(_entry_k->generated_name_))) {
                    return false;
                }
            } else if (!zlang_function_generated_name(program, emitter, _function_k,
                                                      _specialization_k, _entry_k->generated_name_,
                                                      sizeof(_entry_k->generated_name_))) {
                return false;
            }
            _entry_k->statement_ = _function_k;
            _entry_k->specialization_ = _specialization_k;
            ++emitter->emitted_function_count_;
        }
    }
    return true;
}

static void zlang_mark_function_state(zlang_emitter_t *emitter,
                                      const zlang_ast_statement_t *function) {
    for (size_t _index = 0U; _index < emitter->emitted_function_count_; ++_index) {
        if (emitter->emitted_functions_[_index].statement_ == function) {
            emitter->emitted_functions_[_index].needs_state_ = true;
        }
    }
}

static bool zlang_function_marked(const zlang_emitter_t *emitter, const char *name) {
    if (name == NULL) {
        return false;
    }
    for (size_t _index = 0U; _index < emitter->emitted_function_count_; ++_index) {
        const zlang_emitted_function_t *const _entry_k = &emitter->emitted_functions_[_index];
        if (_entry_k->needs_state_ && _entry_k->statement_ != NULL &&
            _entry_k->statement_->name_ != NULL && strcmp(_entry_k->statement_->name_, name) == 0) {
            return true;
        }
    }
    return false;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_expression_mentions_stateful_std(const zlang_program_t *program,
                                                   zlang_emitter_t *emitter, size_t index,
                                                   uint32_t depth) {
    if (index >= program->expression_count_ || depth > 16U) {
        return false;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ == zlang_expression_call_ &&
        zlang_find_import_statement(program, _expression_k->name_) != NULL) {
        const zlang_std_function_t *const _std_k = zlang_std_call(program, emitter, _expression_k);
        if (_std_k != NULL && _std_k->is_stateful_) {
            return true;
        }
    }
    if (zlang_expression_mentions_stateful_std(program, emitter, _expression_k->left_,
                                               depth + 1U) ||
        zlang_expression_mentions_stateful_std(program, emitter, _expression_k->right_,
                                               depth + 1U)) {
        return true;
    }
    for (size_t _argument = 0U; _argument < _expression_k->argument_count_; ++_argument) {
        if (zlang_expression_mentions_stateful_std(
                program, emitter, _expression_k->argument_indices_[_argument], depth + 1U)) {
            return true;
        }
    }
    return false;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_expression_mentions_marked_function(const zlang_program_t *program,
                                                      zlang_emitter_t *emitter, size_t index,
                                                      uint32_t depth) {
    if (index >= program->expression_count_ || depth > 16U) {
        return false;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ == zlang_expression_call_ &&
        zlang_function_marked(emitter, _expression_k->name_)) {
        return true;
    }
    if (zlang_expression_mentions_marked_function(program, emitter, _expression_k->left_,
                                                  depth + 1U) ||
        zlang_expression_mentions_marked_function(program, emitter, _expression_k->right_,
                                                  depth + 1U)) {
        return true;
    }
    for (size_t _argument = 0U; _argument < _expression_k->argument_count_; ++_argument) {
        if (zlang_expression_mentions_marked_function(
                program, emitter, _expression_k->argument_indices_[_argument], depth + 1U)) {
            return true;
        }
    }
    return false;
}

static bool zlang_statement_mentions_expression(const zlang_program_t *program,
                                                zlang_emitter_t *emitter,
                                                const zlang_ast_statement_t *statement,
                                                bool marked_function) {
    for (size_t _item = 0U; _item < statement->initializer_count_; ++_item) {
        const size_t _index_k = statement->initializer_start_ + _item;
        if (marked_function
                ? zlang_expression_mentions_marked_function(program, emitter, _index_k, 0U)
                : zlang_expression_mentions_stateful_std(program, emitter, _index_k, 0U)) {
            return true;
        }
    }
    const size_t _roots_k[2] = {statement->expression_, statement->update_expression_};
    for (size_t _root = 0U; _root < 2U; ++_root) {
        if (_roots_k[_root] >= program->expression_count_) {
            continue;
        }
        if (marked_function
                ? zlang_expression_mentions_marked_function(program, emitter, _roots_k[_root], 0U)
                : zlang_expression_mentions_stateful_std(program, emitter, _roots_k[_root], 0U)) {
            return true;
        }
    }
    return false;
}

static bool zlang_collect_stateful_usage(const zlang_program_t *program, zlang_emitter_t *emitter) {
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_function_ && _statement_k->is_main_) {
            emitter->has_explicit_main_ = true;
        }
        if (_statement_k->kind_ == zlang_statement_import_) {
            continue;
        }
        if (zlang_statement_mentions_expression(program, emitter, _statement_k, false)) {
            emitter->uses_stateful_std_ = true;
        }
    }
    bool _changed_k = false;
    do {
        _changed_k = false;
        for (size_t _function = 0U; _function < program->statement_count_; ++_function) {
            const zlang_ast_statement_t *const _function_k = &program->statements_[_function];
            if (_function_k->kind_ != zlang_statement_function_ ||
                zlang_is_nested(program, _function)) {
                continue;
            }
            bool _needs_state_k = false;
            const size_t _end_k = _function_k->body_start_ + _function_k->body_count_;
            for (size_t _statement = _function_k->body_start_;
                 _statement < _end_k && _statement < program->statement_count_; ++_statement) {
                if (zlang_statement_mentions_expression(program, emitter,
                                                        &program->statements_[_statement], false) ||
                    zlang_statement_mentions_expression(program, emitter,
                                                        &program->statements_[_statement], true)) {
                    _needs_state_k = true;
                    break;
                }
            }
            if (_needs_state_k && !zlang_function_marked(emitter, _function_k->name_)) {
                zlang_mark_function_state(emitter, _function_k);
                _changed_k = true;
            }
        }
    } while (_changed_k);
    return true;
}

static bool zlang_function_generated_name(const zlang_program_t *program,
                                          const zlang_emitter_t *emitter,
                                          const zlang_ast_statement_t *function,
                                          const zlang_array_specialization_t *specialization,
                                          char *output, size_t capacity) {
    char _function_name_k[ZLANG_EMITTED_NAME_CAPACITY];
    if (!zlang_generic_emitted_name(function->name_, _function_name_k, sizeof(_function_name_k)) ||
        !zlang_assemble_name("app_fn_", _function_name_k, "", output, capacity)) {
        return false;
    }
    if (specialization == NULL) {
        return true;
    }
    const char *const _c_type_k =
        zlang_generated_type(program, emitter, specialization->type_name_);
    char _suffix_k[ZLANG_EMITTED_NAME_CAPACITY];
    char _specialization_name_k[ZLANG_EMITTED_NAME_CAPACITY];
    if (_c_type_k == NULL || !zlang_array_type_suffix(_c_type_k, _suffix_k, sizeof(_suffix_k)) ||
        strlen(output) + 1U > sizeof(_specialization_name_k)) {
        return false;
    }
    memcpy(_specialization_name_k, output, strlen(output) + 1U);
    for (uint32_t _axis = 0U; _axis < specialization->rank_; ++_axis) {
        if (!zlang_append_name_suffix(_specialization_name_k, specialization->extents_[_axis],
                                      _specialization_name_k, sizeof(_specialization_name_k))) {
            return false;
        }
    }
    char _tail_k[ZLANG_EMITTED_NAME_CAPACITY];
    return zlang_assemble_name("_", _suffix_k, "", _tail_k, sizeof(_tail_k)) &&
           zlang_assemble_name(_specialization_name_k, _tail_k, "", output, capacity);
}

static const zlang_array_specialization_t *
zlang_find_call_specialization(const zlang_program_t *program, const zlang_emitter_t *emitter,
                               const zlang_ast_expression_t *call) {
    const size_t _function_index_k = zlang_find_function_index(program, call->name_);
    if (_function_index_k == ZLANG_NO_STATEMENT) {
        return NULL;
    }
    const zlang_array_specialization_t *const _first_k =
        zlang_find_specialization(emitter, _function_index_k);
    if (_first_k == NULL) {
        return NULL;
    }
    const zlang_ast_statement_t *_shape_k = NULL;
    for (size_t _argument = 0U; _argument < call->argument_count_; ++_argument) {
        const size_t _argument_k = call->argument_indices_[_argument];
        if (_argument_k >= program->expression_count_ ||
            program->expressions_[_argument_k].kind_ != zlang_expression_identifier_) {
            continue;
        }
        _shape_k = zlang_find_array_decl(program, program->expressions_[_argument_k].name_);
        if (_shape_k != NULL) {
            break;
        }
    }
    if (_shape_k == NULL) {
        for (size_t _statement = 0U; _statement < program->statement_count_; ++_statement) {
            if (program->statements_[_statement].kind_ == zlang_statement_array_declaration_ &&
                program->statements_[_statement].expression_ < program->expression_count_ &&
                &program->expressions_[program->statements_[_statement].expression_] == call) {
                _shape_k = &program->statements_[_statement];
                break;
            }
        }
    }
    if (_shape_k == NULL) {
        return _first_k;
    }
    for (size_t _spec = 0U; _spec < emitter->array_specialization_count_; ++_spec) {
        const zlang_array_specialization_t *const _entry_k =
            &emitter->array_specializations_[_spec];
        if (_entry_k->function_statement_ != _function_index_k ||
            _entry_k->rank_ != _shape_k->rank_ ||
            (_entry_k->type_name_ == NULL) != (_shape_k->type_name_ == NULL) ||
            (_entry_k->type_name_ != NULL &&
             strcmp(_entry_k->type_name_, _shape_k->type_name_) != 0)) {
            continue;
        }
        bool _same_k = true;
        for (uint32_t _axis = 0U; _axis < _shape_k->rank_; ++_axis) {
            if (_entry_k->extents_[_axis] != _shape_k->extents_[_axis]) {
                _same_k = false;
                break;
            }
        }
        if (_same_k) {
            return _entry_k;
        }
    }
    return _first_k;
}

static bool zlang_spec_extent_macro(const zlang_program_t *program, const zlang_emitter_t *emitter,
                                    const zlang_array_specialization_t *specialization,
                                    uint32_t axis, char *output, size_t capacity) {
    if (specialization == NULL || axis >= specialization->rank_) {
        return false;
    }
    const char *const _c_type_k =
        zlang_generated_type(program, emitter, specialization->type_name_);
    char _suffix_k[ZLANG_EMITTED_NAME_CAPACITY];
    char _upper_k[ZLANG_EMITTED_NAME_CAPACITY];
    char _rank_k[32];
    if (_c_type_k == NULL || !zlang_array_type_suffix(_c_type_k, _suffix_k, sizeof(_suffix_k)) ||
        !zlang_upper_name(_suffix_k, _upper_k, sizeof(_upper_k)) ||
        snprintf(_rank_k, sizeof(_rank_k), "_EXTENT_%lu", (unsigned long)axis) < 0) {
        return false;
    }
    return zlang_assemble_name("ZLANG_ARRAY_", _upper_k, _rank_k, output, capacity);
}

static bool zlang_has_sequence_type(const zlang_emitter_t *emitter, const char *type_name) {
    for (size_t _index = 0U; _index < emitter->emitted_sequence_count_; ++_index) {
        if (strcmp(emitter->emitted_sequences_[_index].type_name_, type_name) == 0) {
            return true;
        }
    }
    return false;
}

static bool zlang_emit_specialization_typedefs(const zlang_program_t *program,
                                               zlang_emitter_t *emitter) {
    char _seen[ZLANG_MAX_ARRAY_SPECIALIZATIONS][ZLANG_EMITTED_NAME_CAPACITY];
    size_t _seen_count_k = 0U;
    for (size_t _index = 0U; _index < emitter->array_specialization_count_; ++_index) {
        const zlang_array_specialization_t *const _entry_k =
            &emitter->array_specializations_[_index];
        char _type_name_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_canonical_array_name(program, emitter, _entry_k->type_name_, _entry_k->rank_,
                                        _entry_k->extents_, _type_name_k, sizeof(_type_name_k)) ||
            zlang_has_sequence_type(emitter, _type_name_k)) {
            continue;
        }
        bool _already_k = false;
        for (size_t _seen_index = 0U; _seen_index < _seen_count_k; ++_seen_index) {
            if (strcmp(_seen[_seen_index], _type_name_k) == 0) {
                _already_k = true;
                break;
            }
        }
        if (_already_k || _seen_count_k >= ZLANG_MAX_ARRAY_SPECIALIZATIONS) {
            continue;
        }
        zlang_copy_name(_type_name_k, _seen[_seen_count_k++], sizeof(_seen[0]));
        const char *const _element_k = zlang_generated_type(program, emitter, _entry_k->type_name_);
        uint32_t _capacity_k = 1U;
        for (uint32_t _axis = 0U; _axis < _entry_k->rank_; ++_axis) {
            _capacity_k *= _entry_k->extents_[_axis];
            char _macro_k[ZLANG_EMITTED_NAME_CAPACITY];
            if (!zlang_spec_extent_macro(program, emitter, _entry_k, _axis, _macro_k,
                                         sizeof(_macro_k)) ||
                !zlang_append_text(emitter, "#define ") || !zlang_append_text(emitter, _macro_k) ||
                !zlang_append_text(emitter, " ") ||
                !zlang_append_unsigned(emitter, _entry_k->extents_[_axis]) ||
                !zlang_append_text(emitter, "U\n")) {
                return false;
            }
        }
        if (_element_k == NULL || !zlang_append_text(emitter, "typedef struct ") ||
            !zlang_append_text(emitter, _type_name_k) || !zlang_append_text(emitter, " {\n    ") ||
            !zlang_append_text(emitter, _element_k) || !zlang_append_text(emitter, " data_[") ||
            !zlang_append_unsigned(emitter, _capacity_k) || !zlang_append_text(emitter, "];\n} ") ||
            !zlang_append_text(emitter, _type_name_k) || !zlang_append_text(emitter, ";\n\n")) {
            return false;
        }
    }
    return true;
}

static bool zlang_array_helper_name(const char *prefix, uint32_t rank, const char *suffix,
                                    char *output, size_t capacity) {
    char _rank_k[16];
    const int _written_k = snprintf(_rank_k, sizeof(_rank_k), "%u", (unsigned int)rank);
    if (_written_k < 0 || (size_t)_written_k >= sizeof(_rank_k)) {
        return false;
    }
    if (!zlang_assemble_name(prefix, _rank_k, suffix, output, capacity)) {
        return false;
    }
    return true;
}

static bool zlang_emit_array_helpers(const zlang_program_t *program, zlang_emitter_t *emitter) {
    (void)program;
    if (emitter->requested_array_helper_count_ == 0U) {
        return true;
    }
    if (!zlang_append_text(emitter, "/* generated array helpers */\n")) {
        return false;
    }
    for (uint32_t _rank = 2U; _rank <= ZLANG_ARRAY_MAX_RANK; ++_rank) {
        bool _needed_k = false;
        for (size_t _index = 0U; _index < emitter->requested_array_helper_count_; ++_index) {
            if (emitter->requested_array_helpers_[_index].rank_ == _rank) {
                _needed_k = true;
                break;
            }
        }
        if (!_needed_k) {
            continue;
        }
        char _offset_name_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_array_helper_name("zlang_gen_array_offset_", _rank, "", _offset_name_k,
                                     sizeof(_offset_name_k))) {
            return false;
        }
        if (!zlang_append_text(emitter, "static inline uint32_t ") ||
            !zlang_append_text(emitter, _offset_name_k) || !zlang_append_text(emitter, "(")) {
            return false;
        }
        for (uint32_t _axis = 0U; _axis < _rank; ++_axis) {
            if ((_axis > 0U && !zlang_append_text(emitter, ", ")) ||
                !zlang_append_text(emitter, "uint32_t extent_") ||
                !zlang_append_unsigned(emitter, _axis)) {
                return false;
            }
        }
        for (uint32_t _axis = 0U; _axis < _rank; ++_axis) {
            if (!zlang_append_text(emitter, ", uint32_t index_") ||
                !zlang_append_unsigned(emitter, _axis)) {
                return false;
            }
        }
        if (!zlang_append_text(emitter, ") {\n    if (")) {
            return false;
        }
        for (uint32_t _axis = 0U; _axis < _rank; ++_axis) {
            if ((_axis > 0U && !zlang_append_text(emitter, " || ")) ||
                !zlang_append_text(emitter, "index_") || !zlang_append_unsigned(emitter, _axis) ||
                !zlang_append_text(emitter, " >= extent_") ||
                !zlang_append_unsigned(emitter, _axis)) {
                return false;
            }
        }
        if (!zlang_append_text(emitter,
                               ") {\n        zlang_trap(ZLANG_TRAP_BOUNDS);\n    }\n    return ")) {
            return false;
        }
        for (uint32_t _axis = 0U; _axis < _rank; ++_axis) {
            if (_axis > 0U && !zlang_append_text(emitter, " + ")) {
                return false;
            }
            for (uint32_t _factor = _axis + 1U; _factor < _rank; ++_factor) {
                if (!zlang_append_text(emitter, "extent_") ||
                    !zlang_append_unsigned(emitter, _factor) ||
                    !zlang_append_text(emitter, " * ")) {
                    return false;
                }
            }
            if (!zlang_append_text(emitter, "index_") || !zlang_append_unsigned(emitter, _axis)) {
                return false;
            }
        }
        if (!zlang_append_text(emitter, ";\n}\n\n")) {
            return false;
        }
    }

    for (size_t _entry_index = 0U; _entry_index < emitter->requested_array_helper_count_;
         ++_entry_index) {
        const zlang_emitted_array_helper_t *const _helper_k =
            &emitter->requested_array_helpers_[_entry_index];
        const char *const _type_k = _helper_k->type_name_;
        char _suffix_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_array_type_suffix(_type_k, _suffix_k, sizeof(_suffix_k))) {
            return false;
        }
        char _suffix_with_underscore_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_assemble_name("_", _suffix_k, "", _suffix_with_underscore_k,
                                 sizeof(_suffix_with_underscore_k))) {
            return false;
        }
        char _get_name_k[ZLANG_EMITTED_NAME_CAPACITY];
        char _set_name_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_array_helper_name("zlang_gen_array_get_", _helper_k->rank_,
                                     _suffix_with_underscore_k, _get_name_k, sizeof(_get_name_k)) ||
            !zlang_array_helper_name("zlang_gen_array_set_", _helper_k->rank_,
                                     _suffix_with_underscore_k, _set_name_k, sizeof(_set_name_k))) {
            return false;
        }
        const char *const _name_k = _helper_k->is_set_ ? _set_name_k : _get_name_k;
        if (!zlang_append_text(emitter,
                               _helper_k->is_set_ ? "static inline void " : "static inline ") ||
            (!_helper_k->is_set_ && !zlang_append_text(emitter, _type_k)) ||
            (!_helper_k->is_set_ && !zlang_append_text(emitter, " ")) ||
            !zlang_append_text(emitter, _name_k) || !zlang_append_text(emitter, "(") ||
            (_helper_k->is_set_ && !zlang_append_text(emitter, _type_k)) ||
            (!_helper_k->is_set_ &&
             (!zlang_append_text(emitter, "const ") || !zlang_append_text(emitter, _type_k))) ||
            !zlang_append_text(emitter, " *data_")) {
            return false;
        }
        for (uint32_t _axis = 0U; _axis < _helper_k->rank_; ++_axis) {
            if (!zlang_append_text(emitter, ", uint32_t extent_") ||
                !zlang_append_unsigned(emitter, _axis)) {
                return false;
            }
        }
        for (uint32_t _axis = 0U; _axis < _helper_k->rank_; ++_axis) {
            if (!zlang_append_text(emitter, ", uint32_t index_") ||
                !zlang_append_unsigned(emitter, _axis)) {
                return false;
            }
        }
        if (_helper_k->is_set_ &&
            (!zlang_append_text(emitter, ", ") || !zlang_append_text(emitter, _type_k) ||
             !zlang_append_text(emitter, " value_"))) {
            return false;
        }
        if (!zlang_append_text(emitter, ") {\n    ")) {
            return false;
        }
        if (_helper_k->is_set_) {
            if (!zlang_append_text(emitter, "data_[zlang_gen_array_offset_")) {
                return false;
            }
        } else if (!zlang_append_text(emitter, "return data_[zlang_gen_array_offset_")) {
            return false;
        }
        if (!zlang_append_unsigned(emitter, _helper_k->rank_) || !zlang_append_text(emitter, "(")) {
            return false;
        }
        for (uint32_t _axis = 0U; _axis < _helper_k->rank_; ++_axis) {
            if ((_axis > 0U && !zlang_append_text(emitter, ", ")) ||
                !zlang_append_text(emitter, "extent_") || !zlang_append_unsigned(emitter, _axis)) {
                return false;
            }
        }
        for (uint32_t _axis = 0U; _axis < _helper_k->rank_; ++_axis) {
            if (!zlang_append_text(emitter, ", index_") || !zlang_append_unsigned(emitter, _axis)) {
                return false;
            }
        }
        if (!zlang_append_text(emitter, ")]")) {
            return false;
        }
        if (_helper_k->is_set_ && !zlang_append_text(emitter, " = value_")) {
            return false;
        }
        if (!zlang_append_text(emitter, ";\n}\n\n")) {
            return false;
        }
    }
    return true;
}

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
                                             "zlang_gen_literal_%lu_data", _literal_index_k);
        const int _view_written_k = snprintf(_entry_k->view_name_, sizeof(_entry_k->view_name_),
                                             "zlang_gen_literal_%lu", _literal_index_k);
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

static bool zlang_materialize_proven_read(const zlang_program_t *program, size_t index,
                                          zlang_emitter_t *emitter, uint32_t depth,
                                          bool needs_newline, char *name, size_t capacity) {
    if (index >= program->expression_count_) {
        return false;
    }
    const zlang_ast_expression_t *const _expression_k = &program->expressions_[index];
    if (_expression_k->kind_ != zlang_expression_index_ || !_expression_k->is_proven_) {
        return false;
    }
    const char *const _cached_k = zlang_find_cse(emitter, index);
    if (_cached_k != NULL) {
        return zlang_copy_name(_cached_k, name, capacity);
    }
    const char *const _type_k = zlang_expression_type(program, emitter, index);
    const char *const _c_type_k = zlang_generated_type(program, emitter, _type_k);
    const int _written_k =
        snprintf(name, capacity, "zlang_gen_read_%lu", (unsigned long)emitter->next_index_++);
    if (_c_type_k == NULL || _written_k < 0 || (size_t)_written_k >= capacity ||
        emitter->cse_count_ >= ZLANG_EMITTED_CSE_CAPACITY) {
        return false;
    }
    if ((needs_newline && !zlang_append_indent(emitter, depth)) ||
        !zlang_append_text(emitter, _c_type_k) || !zlang_append_text(emitter, " ") ||
        !zlang_append_text(emitter, name) || !zlang_append_text(emitter, " = ") ||
        !zlang_emit_expression(program, index, emitter) || !zlang_append_text(emitter, ";\n")) {
        return false;
    }
    emitter->cse_expression_indices_[emitter->cse_count_] = index;
    snprintf(emitter->cse_names_[emitter->cse_count_], sizeof(emitter->cse_names_[0]), "%s", name);
    ++emitter->cse_count_;
    return true;
}

static bool zlang_emit_index_assignment(const zlang_program_t *program,
                                        const zlang_ast_expression_t *expression,
                                        zlang_emitter_t *emitter, uint32_t depth) {
    const zlang_ast_expression_t *const _target_k = &program->expressions_[expression->left_];
    const char *const _generated_k = zlang_resolve_name(emitter, _target_k->name_);
    const zlang_emitted_symbol_t *const _symbol_k =
        zlang_find_active_symbol(emitter, _target_k->name_);
    char _index_name[32];
    const int _written_k = snprintf(_index_name, sizeof(_index_name), "zlang_gen_index_%lu",
                                    (unsigned long)emitter->next_index_++);
    if (_target_k->kind_ != zlang_expression_index_ || _generated_k == NULL || _symbol_k == NULL ||
        _written_k < 0 || (size_t)_written_k >= sizeof(_index_name)) {
        return false;
    }
    const zlang_ast_statement_t *const _sequence_k = zlang_find_sequence(emitter, _target_k->name_);
    const bool _string_element_k = _sequence_k != NULL && _sequence_k->has_element_capacity_ &&
                                   strcmp(_sequence_k->type_name_, "string") == 0;
    if (_symbol_k->view_element_[0] != '\0') {
        const char *const _element_type_k =
            zlang_generated_type(program, emitter, _symbol_k->view_element_);
        return _element_type_k != NULL && zlang_append_indent(emitter, depth) &&
               zlang_append_text(emitter, "(*(") && zlang_append_text(emitter, _element_type_k) &&
               zlang_append_text(emitter, " *)zlang_array_get(") &&
               zlang_append_text(emitter, _symbol_k->generated_name_) &&
               zlang_append_text(emitter, ".data_, ") &&
               zlang_emit_index_expression(program, _target_k->right_, emitter) &&
               zlang_append_text(emitter, ", ") &&
               zlang_append_text(emitter, _symbol_k->generated_name_) &&
               zlang_append_text(emitter, ".length_, ") &&
               zlang_append_text(emitter, _symbol_k->generated_name_) &&
               zlang_append_text(emitter, ".element_size_)) = ") &&
               zlang_emit_expression(program, expression->right_, emitter) &&
               zlang_append_text(emitter, ";\n");
    }
    if (_target_k->is_proven_ && !_string_element_k) {
        const size_t _saved_cse_k = emitter->cse_count_;
        const zlang_ast_expression_t *const _value_k = &program->expressions_[expression->right_];
        char _left_name[ZLANG_EMITTED_NAME_CAPACITY];
        char _right_name[ZLANG_EMITTED_NAME_CAPACITY];
        const bool _materialized_k =
            _value_k->kind_ != zlang_expression_add_ ||
            (zlang_materialize_proven_read(program, _value_k->left_, emitter, depth, false,
                                           _left_name, sizeof(_left_name)) &&
             zlang_materialize_proven_read(program, _value_k->right_, emitter, depth, true,
                                           _right_name, sizeof(_right_name)));
        if (!_materialized_k || !zlang_append_indent(emitter, depth) ||
            !zlang_append_text(emitter, _generated_k) || !zlang_append_text(emitter, ".data_[") ||
            !zlang_emit_index_expression(program, _target_k->right_, emitter) ||
            !zlang_append_text(emitter, "] = ")) {
            emitter->cse_count_ = _saved_cse_k;
            return false;
        }
        const bool _result_k =
            (_value_k->kind_ == zlang_expression_add_
                 ? zlang_append_text(emitter, "zlang_add_i32(") &&
                       zlang_append_text(emitter, _left_name) && zlang_append_text(emitter, ", ") &&
                       zlang_append_text(emitter, _right_name) && zlang_append_text(emitter, ")")
                 : zlang_emit_expression(program, expression->right_, emitter)) &&
            zlang_append_text(emitter, ";\n");
        emitter->cse_count_ = _saved_cse_k;
        return _result_k;
    }
    if (_target_k->argument_count_ > 1U) {
        uint32_t _rank_k = 0U;
        if (_symbol_k->is_array_) {
            _rank_k = _symbol_k->array_rank_;
        } else if (_sequence_k != NULL) {
            _rank_k = _sequence_k->rank_;
        }
        if (_rank_k != _target_k->argument_count_) {
            return false;
        }
        return zlang_emit_array_typed_call(program, emitter, _symbol_k, _target_k, true,
                                           expression->right_) &&
               zlang_append_text(emitter, ";\n");
    }
    char _element_capacity[ZLANG_EMITTED_NAME_CAPACITY];
    if (_string_element_k &&
        !zlang_sequence_element_capacity_text(program, emitter, _sequence_k, _element_capacity,
                                              sizeof(_element_capacity))) {
        return false;
    }

    if (!zlang_append_text(emitter, "uint32_t ") || !zlang_append_text(emitter, _index_name) ||
        !zlang_append_text(emitter, " = ") ||
        !zlang_emit_index_expression(program, _target_k->right_, emitter) ||
        !zlang_append_text(emitter, ";\n") || !zlang_append_indent(emitter, depth) ||
        !zlang_append_text(emitter, "if (") || !zlang_append_text(emitter, _index_name) ||
        !zlang_append_text(emitter, " < ") || !zlang_append_array_extent(emitter, _symbol_k) ||
        !zlang_append_text(emitter, ") {\n") || !zlang_append_indent(emitter, depth + 1U) ||
        !zlang_append_text(emitter, _generated_k) || !zlang_append_text(emitter, ".data_[") ||
        !zlang_append_text(emitter, _index_name) || !zlang_append_text(emitter, "] = ")) {
        return false;
    }
    if (_string_element_k) {
        char _slot[ZLANG_EMITTED_NAME_CAPACITY + 64];
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
    const zlang_emitted_symbol_t *const _base_symbol_k =
        _target_k->base_name_ != NULL ? zlang_find_active_symbol(emitter, _target_k->base_name_)
                                      : NULL;
    if (_target_k->kind_ == zlang_expression_field_ && _base_symbol_k != NULL &&
        !_base_symbol_k->is_sequence_ && _target_k->argument_count_ > 0U) {
        const zlang_ast_statement_t *const _record_k =
            zlang_find_struct_statement(program, _base_symbol_k->type_name_);
        const zlang_ast_statement_t *const _field_statement_k =
            _record_k != NULL ? zlang_find_field_statement(program, _record_k, _target_k->name_)
                              : NULL;
        if (_field_statement_k == NULL ||
            _field_statement_k->kind_ != zlang_statement_array_declaration_) {
            return false;
        }
        return zlang_emit_array_field_call(program, emitter, _target_k, _field_statement_k,
                                           _base_symbol_k, true, expression->right_) &&
               zlang_append_text(emitter, ";\n");
    }
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
// NOLINTNEXTLINE(misc-no-recursion)
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
    const bool _self_grouped_k = _kind_k == zlang_expression_not_ ||
                                 _kind_k == zlang_expression_and_ ||
                                 _kind_k == zlang_expression_or_;
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
        statement->kind_ == zlang_statement_do_while_ || statement->kind_ == zlang_statement_for_ ||
        statement->kind_ == zlang_statement_foreach_ ||
        statement->kind_ == zlang_statement_repeat_ ||
        statement->kind_ == zlang_statement_switch_ || statement->kind_ == zlang_statement_block_) {
        return true;
    }
    return statement->kind_ == zlang_statement_expression_ &&
           statement->expression_ < program->expression_count_ &&
           (program->expressions_[statement->expression_].kind_ ==
                zlang_expression_index_assignment_ ||
            program->expressions_[statement->expression_].kind_ ==
                zlang_expression_direct_index_assignment_);
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
        const int _written_k = snprintf(_name, sizeof(_name), "zlang_gen_value_%lu",
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
        if (_candidate == index &&
            _expression_k->kind_ == zlang_expression_direct_index_assignment_ &&
            _expression_k->name_ != NULL && name != NULL &&
            strcmp(_expression_k->name_, name) == 0) {
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
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_foreach_ &&
            _statement_k->iterable_name_ != NULL &&
            strcmp(_statement_k->iterable_name_, name) == 0) {
            return true;
        }
    }
    for (size_t _index = 0U; _index < program->expression_count_; ++_index) {
        const zlang_ast_expression_t *const _expression_k = &program->expressions_[_index];
        if (_expression_k->kind_ == zlang_expression_direct_index_assignment_ &&
            _expression_k->name_ != NULL && strcmp(_expression_k->name_, name) == 0) {
            return true;
        }
        const bool _is_field_k = _expression_k->kind_ == zlang_expression_field_ ||
                                 _expression_k->kind_ == zlang_expression_field_assignment_;
        const char *const _candidate_k =
            _is_field_k ? _expression_k->base_name_ : _expression_k->name_;
        if ((_expression_k->kind_ != zlang_expression_identifier_ &&
             _expression_k->kind_ != zlang_expression_index_ &&
             _expression_k->kind_ != zlang_expression_direct_index_assignment_ && !_is_field_k) ||
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
    if (!zlang_register_symbol(emitter, statement->name_, statement->type_name_, "app_local_",
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
static bool zlang_emit_struct_initializer(const zlang_program_t *program, zlang_emitter_t *emitter,
                                          const char *struct_name, const char *dynamic_name) {
    const zlang_ast_statement_t *const _structure_k =
        zlang_find_struct_statement(program, struct_name);
    if (_structure_k == NULL) {
        return false;
    }
    if (!zlang_append_text(emitter, "{")) {
        return false;
    }
    bool _first_k = true;
    if (_structure_k->base_name_ != NULL) {
        if (!zlang_emit_struct_initializer(program, emitter, _structure_k->base_name_,
                                           dynamic_name)) {
            return false;
        }
        _first_k = false;
    } else if (zlang_struct_has_descendant(program, dynamic_name)) {
        char _instance_k[ZLANG_EMITTED_NAME_CAPACITY];
        char _tag_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_vtable_instance_name(emitter, dynamic_name, _instance_k, sizeof(_instance_k)) ||
            !zlang_type_tag_name(emitter, dynamic_name, _tag_k, sizeof(_tag_k)) ||
            !zlang_append_text(emitter, "&") || !zlang_append_text(emitter, _instance_k) ||
            !zlang_append_text(emitter, ", ") || !zlang_append_text(emitter, _tag_k)) {
            return false;
        }
        _first_k = false;
    }
    const size_t _end_k = _structure_k->body_start_ + _structure_k->body_count_;
    for (size_t _field = _structure_k->body_start_;
         _field < _end_k && _field < program->statement_count_; ++_field) {
        const zlang_ast_statement_t *const _field_k = &program->statements_[_field];
        if (_field_k->kind_ == zlang_statement_function_ ||
            zlang_statement_in_method(program, _structure_k, _field)) {
            continue;
        }
        if (!zlang_append_text(emitter, _first_k ? " " : ", ") ||
            !zlang_append_text(emitter, _field_k->kind_ == zlang_statement_array_declaration_
                                            ? "{ ZLANG_ZERO }"
                                            : "ZLANG_ZERO")) {
            return false;
        }
        _first_k = false;
    }
    return zlang_append_text(emitter, "}");
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_statement(const zlang_program_t *program,
                                 const zlang_ast_statement_t *statement, zlang_emitter_t *emitter,
                                 uint32_t depth) {
    if (!zlang_emit_attached_comments(program, emitter, depth, statement->line_)) {
        return false;
    }
    const char *_written_name_k = NULL;
    bool _writes_k = false;
    if (statement->expression_ < program->expression_count_) {
        const zlang_ast_expression_t *const _expression_k =
            &program->expressions_[statement->expression_];
        _writes_k = _expression_k->kind_ == zlang_expression_index_assignment_ ||
                    _expression_k->kind_ == zlang_expression_direct_index_assignment_ ||
                    _expression_k->kind_ == zlang_expression_field_assignment_ ||
                    _expression_k->kind_ == zlang_expression_array_set_ ||
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
        if (!zlang_register_symbol(emitter, statement->name_, statement->type_name_, "app_local_",
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
        } else if (zlang_std_find_result(statement->type_name_) != NULL ||
                   zlang_is_user_result_type(program, statement->type_name_) ||
                   zlang_find_struct_statement(program, statement->type_name_) != NULL) {
            if (zlang_find_struct_statement(program, statement->type_name_) != NULL &&
                zlang_struct_is_polymorphic(program, statement->type_name_)) {
                if (!zlang_emit_struct_initializer(program, emitter, statement->type_name_,
                                                   statement->type_name_)) {
                    return false;
                }
            } else if (!zlang_append_text(emitter, "{ ZLANG_ZERO }")) {
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
    case zlang_statement_array_declaration_: {
        char _name[ZLANG_EMITTED_NAME_CAPACITY];
        const bool _string_elements_k =
            statement->has_element_capacity_ && strcmp(statement->type_name_, "string") == 0;
        if (emitter->active_specialization_ != NULL && statement->is_inferred_ &&
            !_string_elements_k) {
            const zlang_array_specialization_t *const _spec_k = emitter->active_specialization_;
            char _canonical_k[ZLANG_EMITTED_NAME_CAPACITY];
            if (!zlang_canonical_array_name(program, emitter, _spec_k->type_name_, _spec_k->rank_,
                                            _spec_k->extents_, _canonical_k,
                                            sizeof(_canonical_k)) ||
                !zlang_register_symbol(emitter, statement->name_, _spec_k->type_name_, "app_local_",
                                       _spec_k->rank_, true, false, true, false, _name,
                                       sizeof(_name))) {
                return false;
            }
            zlang_emitted_symbol_t *_array_symbol_k =
                zlang_find_mutable_emitted_symbol(emitter, statement->name_);
            if (_array_symbol_k != NULL) {
                _array_symbol_k->is_array_ = true;
                _array_symbol_k->array_rank_ = _spec_k->rank_;
                for (uint32_t _axis = 0U; _axis < _spec_k->rank_; ++_axis) {
                    _array_symbol_k->array_extents_[_axis] = _spec_k->extents_[_axis];
                }
            }
            uint32_t _capacity_k = 1U;
            for (uint32_t _axis = 0U; _axis < _spec_k->rank_; ++_axis) {
                _capacity_k *= _spec_k->extents_[_axis];
            }
            if ((statement->is_const_ && !zlang_append_text(emitter, "const ")) ||
                !zlang_append_text(emitter, _canonical_k) || !zlang_append_text(emitter, " ") ||
                !zlang_append_text(emitter, _name) || !zlang_append_text(emitter, " = { { ") ||
                !zlang_append_zero_list(emitter, _capacity_k, 0U) ||
                !zlang_append_text(emitter, " } };\n")) {
                return false;
            }
            return zlang_append_unused_guard(program, emitter, statement->name_, _name, depth);
        }
        if (statement->capacity_ == 0U ||
            !zlang_register_symbol(emitter, statement->name_, statement->type_name_, "app_local_",
                                   statement->capacity_, statement->has_explicit_capacity_,
                                   _string_elements_k, true, false, _name, sizeof(_name))) {
            return false;
        }
        const zlang_emitted_sequence_t *const _entry_k =
            zlang_find_sequence_entry(program, emitter, statement);
        if (_entry_k == NULL) {
            return false;
        }
        zlang_emitted_symbol_t *const _array_symbol_k =
            zlang_find_mutable_emitted_symbol(emitter, statement->name_);
        if (_array_symbol_k != NULL) {
            _array_symbol_k->is_array_ = true;
            _array_symbol_k->array_rank_ = statement->rank_;
            for (uint32_t _axis = 0U; _axis < statement->rank_; ++_axis) {
                _array_symbol_k->array_extents_[_axis] = statement->extents_[_axis];
            }
        }
        if (zlang_is_static(emitter, statement) && !zlang_append_text(emitter, "static ")) {
            return false;
        }
        if (statement->is_const_ && !zlang_append_text(emitter, "const ")) {
            return false;
        }
        if (!zlang_append_text(emitter, _entry_k->type_name_) || !zlang_append_text(emitter, " ") ||
            !zlang_append_text(emitter, _name) || !zlang_append_text(emitter, " = ")) {
            return false;
        }
        if (statement->expression_ != ZLANG_NO_EXPRESSION) {
            if (!zlang_emit_expression(program, statement->expression_, emitter) ||
                !zlang_append_text(emitter, ";\n")) {
                return false;
            }
            return zlang_append_unused_guard(program, emitter, statement->name_, _name, depth);
        }
        if (_string_elements_k) {
            if (!zlang_append_text(emitter, "{ ZLANG_ZERO };\n") ||
                !zlang_append_unused_guard(program, emitter, statement->name_, _name, depth)) {
                return false;
            }
            if (statement->initializer_count_ == 0U) {
                return true;
            }
        } else if (zlang_find_struct_statement(program, statement->type_name_) != NULL) {
            if (!zlang_append_text(emitter, "{ { ")) {
                return false;
            }
            for (uint32_t _item = 0U; _item < statement->capacity_; ++_item) {
                if ((_item > 0U && !zlang_append_text(emitter, ", ")) ||
                    !zlang_append_text(emitter, "{ ZLANG_ZERO }")) {
                    return false;
                }
            }
            if (!zlang_append_text(emitter, " } };\n")) {
                return false;
            }
            return zlang_append_unused_guard(program, emitter, statement->name_, _name, depth);
        } else {
            if (!zlang_append_text(emitter, "{ { ") ||
                !zlang_emit_sequence_values(program, statement, emitter) ||
                !zlang_append_text(emitter, " } };\n")) {
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
            char _slot[ZLANG_EMITTED_NAME_CAPACITY + 64];
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
        if (_expression_k != NULL &&
            _expression_k->kind_ == zlang_expression_direct_index_assignment_) {
            const char *const _name_k = zlang_resolve_name(emitter, _expression_k->name_);
            if (_name_k == NULL || _expression_k->argument_count_ != 1U ||
                !zlang_append_text(emitter, _name_k) || !zlang_append_text(emitter, ".data_[")) {
                return false;
            }
            if (!zlang_append_unsigned(emitter, (uint32_t)_expression_k->argument_indices_[0]) ||
                !zlang_append_text(emitter, "] = ") ||
                !zlang_emit_expression(program, _expression_k->right_, emitter) ||
                !zlang_append_text(emitter, ";\n")) {
                return false;
            }
            return true;
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
        if (statement->name_ != NULL) {
            char _label_k[ZLANG_EMITTED_NAME_CAPACITY];
            if (!zlang_lower_name(statement->name_, _label_k, sizeof(_label_k)) ||
                !zlang_append_text(emitter, "goto zlang_label_") ||
                !zlang_append_text(emitter, _label_k) || !zlang_append_text(emitter, "_break;\n")) {
                return false;
            }
            return true;
        }
        return zlang_append_text(emitter, "break;\n");
    case zlang_statement_continue_:
        if (statement->name_ != NULL) {
            char _label_k[ZLANG_EMITTED_NAME_CAPACITY];
            if (!zlang_lower_name(statement->name_, _label_k, sizeof(_label_k)) ||
                !zlang_append_text(emitter, "goto zlang_label_") ||
                !zlang_append_text(emitter, _label_k) ||
                !zlang_append_text(emitter, "_continue;\n")) {
                return false;
            }
            return true;
        }
        return zlang_append_text(emitter, "continue;\n");
    case zlang_statement_label_: {
        char _label_k[ZLANG_EMITTED_NAME_CAPACITY];
        bool _has_jump_k = false;
        bool _has_break_k = false;
        bool _has_continue_k = false;
        if (!zlang_lower_name(statement->name_, _label_k, sizeof(_label_k))) {
            return false;
        }
        for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
            const zlang_ast_statement_t *const _candidate_k = &program->statements_[_index];
            if (_candidate_k->name_ == NULL || statement->name_ == NULL ||
                strcmp(_candidate_k->name_, statement->name_) != 0) {
                continue;
            }
            if (_candidate_k->kind_ == zlang_statement_jump_) {
                _has_jump_k = true;
            } else if (_candidate_k->kind_ == zlang_statement_break_) {
                _has_break_k = true;
            } else if (_candidate_k->kind_ == zlang_statement_continue_) {
                _has_continue_k = true;
            }
        }
        if (_has_jump_k &&
            (!zlang_append_text(emitter, "zlang_label_") || !zlang_append_text(emitter, _label_k) ||
             !zlang_append_text(emitter, ": ;\n"))) {
            return false;
        }
        if (statement->body_count_ == 1U && statement->body_start_ < program->statement_count_) {
            if (_has_continue_k && (!zlang_append_text(emitter, "zlang_label_") ||
                                    !zlang_append_text(emitter, _label_k) ||
                                    !zlang_append_text(emitter, "_continue: ;\n"))) {
                return false;
            }
            if (!zlang_emit_statement(program, &program->statements_[statement->body_start_],
                                      emitter, depth)) {
                return false;
            }
            if (_has_break_k && (!zlang_append_indent(emitter, depth) ||
                                 !zlang_append_text(emitter, "zlang_label_") ||
                                 !zlang_append_text(emitter, _label_k) ||
                                 !zlang_append_text(emitter, "_break: ;\n"))) {
                return false;
            }
        }
        return true;
    }
    case zlang_statement_jump_: {
        char _label_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_lower_name(statement->name_, _label_k, sizeof(_label_k)) ||
            !zlang_append_text(emitter, "goto zlang_label_") ||
            !zlang_append_text(emitter, _label_k) || !zlang_append_text(emitter, ";\n")) {
            return false;
        }
        return true;
    }
    case zlang_statement_do_while_: {
        if (!zlang_append_text(emitter, "do {\n")) {
            return false;
        }
        const zlang_emitter_scope_t _loop_scope_k = zlang_save_scope(emitter);
        const bool _body_ok_k = zlang_emit_body(program, statement->body_start_,
                                                statement->body_count_, emitter, depth + 1U) &&
                                zlang_append_indent(emitter, depth) &&
                                zlang_append_text(emitter, "} while ") &&
                                zlang_emit_condition(program, statement->expression_, emitter) &&
                                zlang_append_text(emitter, ";\n");
        zlang_restore_scope(emitter, _loop_scope_k);
        return _body_ok_k;
    }
    case zlang_statement_switch_: {
        if (!zlang_append_text(emitter, "switch (") ||
            !zlang_emit_expression(program, statement->expression_, emitter) ||
            !zlang_append_text(emitter, ") {\n")) {
            return false;
        }
        const zlang_emitter_scope_t _switch_scope_k = zlang_save_scope(emitter);
        const size_t _end_k = statement->body_start_ + statement->body_count_;
        for (size_t _clause = statement->body_start_;
             _clause < _end_k && _clause < program->statement_count_; ++_clause) {
            const zlang_ast_statement_t *const _clause_k = &program->statements_[_clause];
            if (_clause_k->kind_ != zlang_statement_case_ &&
                _clause_k->kind_ != zlang_statement_default_) {
                continue;
            }
            if (!zlang_append_indent(emitter, depth + 1U)) {
                return false;
            }
            if (_clause_k->kind_ == zlang_statement_case_) {
                if (!zlang_append_text(emitter, "case ") ||
                    !zlang_emit_expression(program, _clause_k->expression_, emitter) ||
                    !zlang_append_text(emitter, ":\n")) {
                    return false;
                }
            } else if (!zlang_append_text(emitter, "default:\n")) {
                return false;
            }
            const zlang_emitter_scope_t _clause_scope_k = zlang_save_scope(emitter);
            const bool _body_ok_k = zlang_emit_body(program, _clause_k->body_start_,
                                                    _clause_k->body_count_, emitter, depth + 2U) &&
                                    zlang_append_indent(emitter, depth + 2U) &&
                                    zlang_append_text(emitter, "break;\n");
            zlang_restore_scope(emitter, _clause_scope_k);
            if (!_body_ok_k) {
                return false;
            }
        }
        zlang_restore_scope(emitter, _switch_scope_k);
        return zlang_append_indent(emitter, depth) && zlang_append_text(emitter, "}\n");
    }
    case zlang_statement_case_:
    case zlang_statement_default_:
        return true;
    case zlang_statement_return_:
        if (statement->expression_ == ZLANG_NO_EXPRESSION) {
            return zlang_append_text(emitter, "return;\n");
        }
        if (!zlang_append_text(emitter, "return ") ||
            !zlang_emit_expression_as(program, statement->expression_, emitter,
                                      emitter->current_return_type_, false)) {
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
            snprintf(_repeat_name, sizeof(_repeat_name), "zlang_gen_repeat_%lu",
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
        const bool _range_kind_k = statement->expression_ != ZLANG_NO_EXPRESSION;
        const zlang_emitted_symbol_t *const _iterable_symbol_k =
            _range_kind_k ? NULL : zlang_find_active_symbol(emitter, statement->iterable_name_);
        if (!_range_kind_k && _iterable_symbol_k == NULL) {
            return false;
        }
        if (_range_kind_k) {
            char _index_name[ZLANG_EMITTED_NAME_CAPACITY];
            const char *const _name_k =
                statement->index_name_ != NULL ? statement->index_name_ : statement->name_;
            if (_name_k == NULL ||
                !zlang_register_symbol(emitter, _name_k, "uint32", "app_local_", 0U, false, false,
                                       false, false, _index_name, sizeof(_index_name)) ||
                !zlang_append_indent(emitter, depth) ||
                !zlang_append_text(emitter, "for (uint32_t ") ||
                !zlang_append_text(emitter, _index_name) || !zlang_append_text(emitter, " = ") ||
                !zlang_emit_expression(program, program->expressions_[statement->expression_].left_,
                                       emitter) ||
                !zlang_append_text(emitter, "; ") || !zlang_append_text(emitter, _index_name) ||
                !zlang_append_text(emitter, " < ") ||
                !zlang_emit_expression(
                    program, program->expressions_[statement->expression_].right_, emitter) ||
                !zlang_append_text(emitter, "; ++") || !zlang_append_text(emitter, _index_name) ||
                !zlang_append_text(emitter, ") {\n")) {
                zlang_restore_scope(emitter, _foreach_scope_k);
                return false;
            }
            const bool _body_ok_k = zlang_emit_body(program, statement->body_start_,
                                                    statement->body_count_, emitter, depth + 1U) &&
                                    zlang_append_indent(emitter, depth) &&
                                    zlang_append_text(emitter, "}\n");
            zlang_restore_scope(emitter, _foreach_scope_k);
            return _body_ok_k;
        }
        if (statement->foreach_rank_ > 1U) {
            char _coordinate_names[ZLANG_ARRAY_MAX_RANK][ZLANG_EMITTED_NAME_CAPACITY];
            for (uint32_t _axis = 0U; _axis < statement->foreach_rank_; ++_axis) {
                if (!zlang_register_symbol(emitter, statement->foreach_names_[_axis], "uint32",
                                           "app_local_", 0U, false, false, false, false,
                                           _coordinate_names[_axis],
                                           sizeof(_coordinate_names[_axis]))) {
                    zlang_restore_scope(emitter, _foreach_scope_k);
                    return false;
                }
            }
            for (uint32_t _axis = 0U; _axis < statement->foreach_rank_; ++_axis) {
                if ((_axis > 0U && !zlang_append_indent(emitter, depth + _axis)) ||
                    !zlang_append_text(emitter, "for (uint32_t ") ||
                    !zlang_append_text(emitter, _coordinate_names[_axis]) ||
                    !zlang_append_text(emitter, " = ZLANG_ZERO; ") ||
                    !zlang_append_text(emitter, _coordinate_names[_axis]) ||
                    !zlang_append_text(emitter, " < ") ||
                    !zlang_append_array_extent_axis(emitter, _iterable_symbol_k, _axis) ||
                    !zlang_append_text(emitter, "; ++") ||
                    !zlang_append_text(emitter, _coordinate_names[_axis]) ||
                    !zlang_append_text(emitter, ") {\n")) {
                    zlang_restore_scope(emitter, _foreach_scope_k);
                    return false;
                }
            }
            const bool _body_ok_k =
                zlang_emit_body(program, statement->body_start_, statement->body_count_, emitter,
                                depth + statement->foreach_rank_);
            bool _result_k = _body_ok_k;
            for (uint32_t _axis = statement->foreach_rank_; _axis > 0U; --_axis) {
                _result_k = _result_k && zlang_append_indent(emitter, depth + _axis - 1U) &&
                            zlang_append_text(emitter, "}\n");
            }
            zlang_restore_scope(emitter, _foreach_scope_k);
            return _result_k;
        }
        char _loop_index[32];
        const int _loop_written_k =
            snprintf(_loop_index, sizeof(_loop_index), "zlang_gen_foreach_%lu",
                     (unsigned long)emitter->next_index_++);
        char _item_name[ZLANG_EMITTED_NAME_CAPACITY];
        char _user_index_name[ZLANG_EMITTED_NAME_CAPACITY];
        const char *const _iterable_k = zlang_resolve_name(emitter, statement->iterable_name_);
        if (_loop_written_k < 0 || (size_t)_loop_written_k >= sizeof(_loop_index) ||
            (statement->index_name_ != NULL &&
             !zlang_register_symbol(emitter, statement->index_name_, "uint32", "app_local_", 0U,
                                    false, false, false, false, _user_index_name,
                                    sizeof(_user_index_name))) ||
            !zlang_register_symbol(emitter, statement->name_, statement->type_name_, "app_local_",
                                   0U, false, false, false, false, _item_name,
                                   sizeof(_item_name))) {
            return false;
        }
        if (!zlang_append_text(emitter, "for (uint32_t ") ||
            !zlang_append_text(emitter, _loop_index) ||
            !zlang_append_text(emitter, " = ZLANG_ZERO; ") ||
            !zlang_append_text(emitter, _loop_index) || !zlang_append_text(emitter, " < ") ||
            !zlang_append_array_extent(emitter, _iterable_symbol_k) ||
            !zlang_append_text(emitter, "; ++") || !zlang_append_text(emitter, _loop_index) ||
            !zlang_append_text(emitter, ") {\n")) {
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
        if (statement->is_extern_) {
            return true;
        }
        const size_t _function_index_k = zlang_find_function_index(program, statement->name_);
        size_t _specialization_count_k = 0U;
        for (size_t _spec = 0U; _spec < emitter->array_specialization_count_; ++_spec) {
            if (emitter->array_specializations_[_spec].function_statement_ == _function_index_k) {
                ++_specialization_count_k;
            }
        }
        const size_t _iterations_k = _specialization_count_k == 0U ? 1U : _specialization_count_k;
        for (size_t _iteration = 0U; _iteration < _iterations_k; ++_iteration) {
            const zlang_array_specialization_t *_specialization_k = NULL;
            if (_specialization_count_k > 0U) {
                size_t _seen_k = 0U;
                for (size_t _spec = 0U; _spec < emitter->array_specialization_count_; ++_spec) {
                    if (emitter->array_specializations_[_spec].function_statement_ !=
                        _function_index_k) {
                        continue;
                    }
                    if (_seen_k == _iteration) {
                        _specialization_k = &emitter->array_specializations_[_spec];
                        break;
                    }
                    ++_seen_k;
                }
            }
            const zlang_array_specialization_t *const _previous_specialization_k =
                emitter->active_specialization_;
            emitter->active_specialization_ = _specialization_k;
            const zlang_emitted_function_t *_entry_k = NULL;
            for (size_t _registered = 0U; _registered < emitter->emitted_function_count_;
                 ++_registered) {
                const zlang_emitted_function_t *const _candidate_k =
                    &emitter->emitted_functions_[_registered];
                if (_candidate_k->statement_ == statement &&
                    _candidate_k->specialization_ == _specialization_k) {
                    _entry_k = _candidate_k;
                    break;
                }
            }
            const bool _needs_state_k = _entry_k != NULL && _entry_k->needs_state_;

            char _name[ZLANG_EMITTED_NAME_CAPACITY];
            if (!zlang_function_generated_name(program, emitter, statement, _specialization_k,
                                               _name, sizeof(_name))) {
                return false;
            }
            const zlang_emitter_scope_t _function_scope_k = zlang_save_scope(emitter);
            char _return_array_type_k[ZLANG_EMITTED_NAME_CAPACITY];
            const char *_return_type_k = NULL;
            if (statement->is_array_return_ && _specialization_k != NULL) {
                if (zlang_canonical_array_name(program, emitter, _specialization_k->type_name_,
                                               _specialization_k->rank_,
                                               _specialization_k->extents_, _return_array_type_k,
                                               sizeof(_return_array_type_k))) {
                    _return_type_k = _return_array_type_k;
                }
            } else if (statement->is_array_return_) {
                if (zlang_canonical_array_name(program, emitter, statement->type_name_,
                                               statement->return_rank_, statement->return_extents_,
                                               _return_array_type_k,
                                               sizeof(_return_array_type_k))) {
                    _return_type_k = _return_array_type_k;
                }
            } else {
                _return_type_k = zlang_generated_type(program, emitter, statement->type_name_);
            }
            const bool _has_parameters_k = statement->parameter_count_ > 0U || _needs_state_k;
            if (_return_type_k == NULL || !zlang_append_text(emitter, _return_type_k) ||
                !zlang_append_text(emitter, " ") || !zlang_append_text(emitter, _name) ||
                !zlang_append_text(emitter, "(") ||
                (!_has_parameters_k && !zlang_append_text(emitter, "void"))) {
                return false;
            }
            emitter->in_function_ = true;
            for (size_t _index = 0U; _index < statement->parameter_count_; ++_index) {
                const zlang_parameter_t *const _parameter_k =
                    &program->parameters_[statement->parameter_start_ + _index];
                const char *_parameter_type_name_k = _parameter_k->type_name_;
                uint32_t _parameter_rank_k = _parameter_k->rank_;
                const uint32_t *_parameter_extents_k = _parameter_k->extents_;
                if (_parameter_k->is_array_ && _specialization_k != NULL) {
                    _parameter_type_name_k = _specialization_k->type_name_;
                    _parameter_rank_k = _specialization_k->rank_;
                    _parameter_extents_k = _specialization_k->extents_;
                }
                char _parameter_name[ZLANG_EMITTED_NAME_CAPACITY];
                if (!zlang_register_symbol(
                        emitter, _parameter_k->name_, _parameter_type_name_k, "app_param_",
                        _parameter_k->is_array_ ? _parameter_rank_k : 0U, false, false, false,
                        _parameter_k->is_reference_, _parameter_name, sizeof(_parameter_name))) {
                    return false;
                }
                if (_parameter_k->is_array_) {
                    zlang_emitted_symbol_t *_symbol_k = NULL;
                    for (size_t _active = emitter->active_symbol_count_; _active > 0U; --_active) {
                        zlang_emitted_symbol_t *const _candidate_k =
                            &emitter->emitted_symbols_[_active - 1U];
                        if (strcmp(_candidate_k->source_name_, _parameter_k->name_) == 0) {
                            _symbol_k = _candidate_k;
                            break;
                        }
                    }
                    if (_symbol_k != NULL) {
                        _symbol_k->is_array_ = true;
                        _symbol_k->array_rank_ = _parameter_rank_k;
                        for (uint32_t _axis = 0U; _axis < _parameter_rank_k; ++_axis) {
                            _symbol_k->array_extents_[_axis] = _parameter_extents_k[_axis];
                        }
                    }
                }
                char _array_type_k[ZLANG_EMITTED_NAME_CAPACITY];
                const char *_parameter_type_k = NULL;
                if (_parameter_k->is_array_) {
                    if (zlang_canonical_array_name(program, emitter, _parameter_type_name_k,
                                                   _parameter_rank_k, _parameter_extents_k,
                                                   _array_type_k, sizeof(_array_type_k))) {
                        _parameter_type_k = _array_type_k;
                    }
                } else {
                    _parameter_type_k =
                        zlang_generated_type(program, emitter, _parameter_type_name_k);
                }
                if (_parameter_type_k == NULL ||
                    (_index > 0U && !zlang_append_text(emitter, ", ")) ||
                    (_parameter_k->is_const_ && !zlang_append_text(emitter, "const ")) ||
                    !zlang_append_text(emitter, _parameter_type_k) ||
                    (_parameter_k->is_reference_ && !zlang_append_text(emitter, " *")) ||
                    (!_parameter_k->is_reference_ && !zlang_append_text(emitter, " ")) ||
                    !zlang_append_text(emitter, _parameter_name)) {
                    return false;
                }
            }
            const bool _state_param_k =
                !_needs_state_k ||
                (zlang_append_text(emitter, statement->parameter_count_ == 0U ? "" : ", ") &&
                 zlang_append_text(emitter, "zlang_runtime_context_t *zlang_context"));
            const bool _result_k =
                _state_param_k && zlang_append_text(emitter, ") {\n") &&
                zlang_emit_unused_parameter_guards(program, statement, emitter, depth + 1U) &&
                zlang_emit_body(program, statement->body_start_, statement->body_count_, emitter,
                                depth + 1U) &&
                zlang_append_indent(emitter, depth) && zlang_append_text(emitter, "}\n\n");
            emitter->in_function_ = false;
            zlang_restore_scope(emitter, _function_scope_k);
            emitter->active_specialization_ = _previous_specialization_k;
            if (!_result_k) {
                return false;
            }
        }
        return true;
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
    case zlang_statement_interface_:
    case zlang_statement_trait_:
        return true;
    default:
        return false;
    }
}

static bool zlang_activate_macro(zlang_emitter_t *emitter, const char *macro) {
    if (macro == NULL) {
        return false;
    }
    for (size_t _index = 0U; _index < emitter->activated_macro_count_; ++_index) {
        if (strcmp(emitter->activated_macros_[_index], macro) == 0) {
            return true;
        }
    }
    if (emitter->activated_macro_count_ >=
            sizeof(emitter->activated_macros_) / sizeof(emitter->activated_macros_[0]) ||
        !zlang_copy_name(macro, emitter->activated_macros_[emitter->activated_macro_count_],
                         ZLANG_EMITTED_NAME_CAPACITY)) {
        return false;
    }
    ++emitter->activated_macro_count_;
    return true;
}

static bool zlang_activate_std_macros(const zlang_program_t *program, zlang_emitter_t *emitter) {
    emitter->emit_timing_ = emitter->force_timing_;
    emitter->emit_dump_ = emitter->force_dump_;
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->type_name_ != NULL && zlang_std_is_view_type(_statement_k->type_name_) &&
            !zlang_activate_macro(emitter, "ZLANG_STD_VIEW")) {
            return false;
        }
        if ((_statement_k->type_name_ != NULL &&
             (zlang_std_is_native_type(_statement_k->type_name_) &&
              strcmp(_statement_k->type_name_, "stream") != 0)) ||
            (_statement_k->name_ != NULL && zlang_std_is_status_constant(_statement_k->name_))) {
            if (!zlang_activate_macro(emitter, "ZLANG_STD_RESULT")) {
                return false;
            }
        }
    }
    for (size_t _index = 0U; _index < program->parameter_count_; ++_index) {
        const zlang_parameter_t *const _parameter_k = &program->parameters_[_index];
        if (_parameter_k->type_name_ != NULL &&
            zlang_std_is_native_type(_parameter_k->type_name_) &&
            !zlang_activate_macro(emitter, "ZLANG_STD_RESULT")) {
            return false;
        }
    }
    for (size_t _index = 0U; _index < program->expression_count_; ++_index) {
        const zlang_ast_expression_t *const _expression_k = &program->expressions_[_index];
        if (_expression_k->kind_ == zlang_expression_view_ &&
            !zlang_activate_macro(emitter, "ZLANG_STD_VIEW")) {
            return false;
        }
        if (_expression_k->name_ != NULL && zlang_std_is_status_constant(_expression_k->name_) &&
            !zlang_activate_macro(emitter, "ZLANG_STD_RESULT")) {
            return false;
        }
    }
    for (size_t _index = 0U; _index < program->generic_application_count_; ++_index) {
        if ((zlang_std_find_result(program->generic_applications_[_index].name_) != NULL ||
             zlang_is_user_result_type(program, program->generic_applications_[_index].name_)) &&
            !zlang_activate_macro(emitter, "ZLANG_STD_RESULT")) {
            return false;
        }
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ == zlang_statement_import_ &&
            !zlang_activate_macro(
                emitter,
                zlang_std_find(zlang_std_domain(_statement_k->type_name_), _statement_k->name_)
                    ->activation_macro_)) {
            return false;
        }
    }
    for (size_t _index = 0U; _index < program->expression_count_; ++_index) {
        const zlang_ast_expression_t *const _expression_k = &program->expressions_[_index];
        if (_expression_k->kind_ == zlang_expression_method_call_ && _expression_k->name_ != NULL) {
            if (strcmp(_expression_k->name_, "time") == 0) {
                emitter->emit_timing_ = true;
            } else if (strcmp(_expression_k->name_, "dump") == 0) {
                emitter->emit_dump_ = true;
            }
            continue;
        }
        if (_expression_k->kind_ != zlang_expression_call_ ||
            zlang_find_import_statement(program, _expression_k->name_) == NULL) {
            continue;
        }
        const zlang_std_function_t *const _function_k =
            zlang_std_call(program, emitter, _expression_k);
        if (_function_k == NULL || !zlang_activate_macro(emitter, _function_k->activation_macro_)) {
            return false;
        }
    }
    return true;
}

static bool zlang_emit_prologue(const zlang_program_t *program, zlang_emitter_t *emitter) {
    for (size_t _index = 0U; _index < emitter->activated_macro_count_; ++_index) {
        if ((_index > 0U && !zlang_append_text(emitter, "\n")) ||
            !zlang_append_text(emitter, "#define ") ||
            !zlang_append_text(emitter, emitter->activated_macros_[_index]) ||
            !zlang_append_text(emitter, "\n")) {
            return false;
        }
    }
    if (emitter->emit_timing_ && !zlang_append_text(emitter, "#define ZLANG_STD_RESULT_TIMING\n")) {
        return false;
    }
    if (emitter->emit_dump_ && !zlang_append_text(emitter, "#define ZLANG_STD_RESULT_DUMP\n")) {
        return false;
    }
    if ((emitter->activated_macro_count_ > 0U || emitter->emit_timing_ || emitter->emit_dump_) &&
        !zlang_append_text(emitter, "\n")) {
        return false;
    }
    for (size_t _index = 0U; _index < emitter->activated_macro_count_; ++_index) {
        if (strcmp(emitter->activated_macros_[_index], "ZLANG_STD_VIEW") == 0 &&
            !zlang_append_text(emitter, "#define ZLANG_STD_VIEW\n\n")) {
            return false;
        }
    }
    if (!zlang_append_text(emitter, "#include <stdbool.h>\n#include <stdint.h>\n")) {
        return false;
    }
    size_t _extern_count_k = 0U;
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ != zlang_statement_function_ || !_statement_k->is_extern_ ||
            _statement_k->header_name_ == NULL) {
            continue;
        }
        if (_extern_count_k == 0U && !zlang_append_text(emitter, "\n")) {
            return false;
        }
        ++_extern_count_k;
        if (!zlang_append_text(emitter, "#include <") ||
            !zlang_append_text(emitter, _statement_k->header_name_) ||
            !zlang_append_text(emitter, ">\n")) {
            return false;
        }
    }
    if (_extern_count_k == 0U && !zlang_append_text(emitter, "\n")) {
        return false;
    }
    return zlang_append_text(emitter, "#include <zlang/runtime/runtime.h>\n\n");
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
        if (_symbol_k == NULL || !_symbol_k->is_sequence_) {
            continue;
        }
        const char *const _type_k = zlang_generated_type(program, emitter, _symbol_k->type_name_);
        if (_type_k == NULL) {
            return false;
        }
        char _pointer_name[32];
        const int _written_k =
            snprintf(_pointer_name, sizeof(_pointer_name), "zlang_gen_field_ptr_%lu",
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
            !zlang_emit_index_expression(program, _field_k->right_, emitter) ||
            !zlang_append_text(emitter, ", ") || !zlang_append_array_extent(emitter, _symbol_k) ||
            !zlang_append_text(emitter, ", sizeof(") ||
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

static bool zlang_is_array_field_statement(const zlang_program_t *program,
                                           const zlang_ast_statement_t *statement) {
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _owner_k = &program->statements_[_index];
        if (_owner_k->kind_ != zlang_statement_struct_ || zlang_is_generic_template(_owner_k)) {
            continue;
        }
        const size_t _end_k = _owner_k->body_start_ + _owner_k->body_count_;
        for (size_t _field = _owner_k->body_start_;
             _field < _end_k && _field < program->statement_count_; ++_field) {
            if (&program->statements_[_field] == statement) {
                return true;
            }
        }
    }
    return false;
}

static bool zlang_emit_capacity_macros(zlang_emitter_t *emitter) {
    if (emitter->emitted_sequence_count_ == 0U && emitter->emitted_capacity_count_ == 0U) {
        return true;
    }
    if (!zlang_append_text(emitter, "/* generated capacities */\n")) {
        return false;
    }
    for (size_t _index = 0U; _index < emitter->emitted_sequence_count_; ++_index) {
        const zlang_emitted_sequence_t *const _entry_k = &emitter->emitted_sequences_[_index];
        if (!zlang_append_text(emitter, "#define ") ||
            !zlang_append_text(emitter, _entry_k->capacity_macro_) ||
            !zlang_append_text(emitter, " ") ||
            !zlang_append_unsigned(emitter, _entry_k->statement_->capacity_) ||
            !zlang_append_text(emitter, "U\n") || !zlang_append_text(emitter, "#define ") ||
            !zlang_append_text(emitter, _entry_k->length_macro_) ||
            !zlang_append_text(emitter, " ") ||
            !zlang_append_text(emitter, _entry_k->extent_macros_[0]) ||
            !zlang_append_text(emitter, "\n")) {
            return false;
        }
        for (uint32_t _rank = 0U; _rank < _entry_k->statement_->rank_; ++_rank) {
            if (!zlang_append_text(emitter, "#define ") ||
                !zlang_append_text(emitter, _entry_k->extent_macros_[_rank]) ||
                !zlang_append_text(emitter, " ") ||
                !zlang_append_unsigned(emitter, _entry_k->statement_->extents_[_rank]) ||
                !zlang_append_text(emitter, "U\n")) {
                return false;
            }
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
    for (size_t _index = 0U; _index < emitter->emitted_capacity_count_; ++_index) {
        const zlang_emitted_capacity_t *const _entry_k = &emitter->emitted_capacities_[_index];
        if (!zlang_append_text(emitter, "#define ") ||
            !zlang_append_text(emitter, _entry_k->macro_) || !zlang_append_text(emitter, " ") ||
            !zlang_append_unsigned(emitter, _entry_k->statement_->capacity_) ||
            !zlang_append_text(emitter, "U\n")) {
            return false;
        }
    }
    return zlang_append_text(emitter, "\n");
}

static bool zlang_emit_array_typedef(const zlang_program_t *program, zlang_emitter_t *emitter,
                                     const zlang_emitted_sequence_t *entry) {
    const zlang_ast_statement_t *const _statement_k = entry->statement_;
    if (!zlang_append_text(emitter, "typedef struct ") ||
        !zlang_append_text(emitter, entry->type_name_) || !zlang_append_text(emitter, " {\n")) {
        return false;
    }
    if (_statement_k->has_element_capacity_) {
        if (!zlang_append_text(emitter, "    char storage_[") ||
            !zlang_append_text(emitter, entry->capacity_macro_) ||
            !zlang_append_text(emitter, "][") ||
            !zlang_append_text(emitter, entry->element_macro_) ||
            !zlang_append_text(emitter, " + ZLANG_ONE];\n    zlang_string_t data_[") ||
            !zlang_append_text(emitter, entry->capacity_macro_) ||
            !zlang_append_text(emitter, "];\n")) {
            return false;
        }
    } else {
        const char *const _element_type_k =
            zlang_generated_type(program, emitter, _statement_k->type_name_);
        if (_element_type_k == NULL || !zlang_append_text(emitter, "    ") ||
            !zlang_append_text(emitter, _element_type_k) ||
            !zlang_append_text(emitter, " data_[") ||
            !zlang_append_text(emitter, entry->capacity_macro_) ||
            !zlang_append_text(emitter, "];\n")) {
            return false;
        }
    }
    return zlang_append_text(emitter, "} ") && zlang_append_text(emitter, entry->type_name_) &&
           zlang_append_text(emitter, ";\n\n");
}

static bool zlang_emit_array_typedefs(zlang_emitter_t *emitter, const zlang_program_t *program,
                                      bool field_arrays) {
    char _seen_types[ZLANG_EMITTED_SEQUENCE_CAPACITY][ZLANG_EMITTED_NAME_CAPACITY];
    size_t _seen_count_k = 0U;
    bool _emitted_header_k = false;
    for (size_t _index = 0U; _index < emitter->emitted_sequence_count_; ++_index) {
        const zlang_emitted_sequence_t *const _entry_k = &emitter->emitted_sequences_[_index];
        if (zlang_is_array_field_statement(program, _entry_k->statement_) != field_arrays) {
            continue;
        }
        bool _already_k = false;
        for (size_t _seen = 0U; _seen < _seen_count_k; ++_seen) {
            if (strcmp(_seen_types[_seen], _entry_k->type_name_) == 0) {
                _already_k = true;
                break;
            }
        }
        if (_already_k || _seen_count_k >= ZLANG_EMITTED_SEQUENCE_CAPACITY) {
            continue;
        }
        zlang_copy_name(_entry_k->type_name_, _seen_types[_seen_count_k++],
                        ZLANG_EMITTED_NAME_CAPACITY);
        if (!_emitted_header_k) {
            if (!zlang_append_text(emitter, "\n/* generated sequence types */\n")) {
                return false;
            }
            _emitted_header_k = true;
        }
        if (!zlang_emit_array_typedef(program, emitter, _entry_k)) {
            return false;
        }
    }
    return true;
}

static bool zlang_statement_in_method(const zlang_program_t *program,
                                      const zlang_ast_statement_t *structure, size_t index) {
    const size_t _end_k = structure->body_start_ + structure->body_count_;
    for (size_t _member = structure->body_start_;
         _member < _end_k && _member < program->statement_count_; ++_member) {
        const zlang_ast_statement_t *const _member_k = &program->statements_[_member];
        if (_member_k->kind_ != zlang_statement_function_ || _member_k->body_count_ == 0U) {
            continue;
        }
        const size_t _method_end_k = _member_k->body_start_ + _member_k->body_count_;
        if (index >= _member_k->body_start_ && index < _method_end_k) {
            return true;
        }
    }
    return false;
}

static bool zlang_vtable_type_name(const zlang_program_t *program, zlang_emitter_t *emitter,
                                   const char *struct_name, char *output, size_t capacity) {
    const zlang_ast_statement_t *const _root_k = zlang_struct_root_name(program, struct_name);
    char _lower_k[ZLANG_EMITTED_NAME_CAPACITY];
    char _base_k[ZLANG_EMITTED_NAME_CAPACITY];
    return _root_k != NULL && zlang_lower_name(_root_k->name_, _lower_k, sizeof(_lower_k)) &&
           zlang_assemble_name("zlang_vtable_", _lower_k, "_t", _base_k, sizeof(_base_k)) &&
           zlang_make_unique_name(emitter, _base_k, output, capacity);
}

static bool zlang_vtable_instance_name(zlang_emitter_t *emitter, const char *struct_name,
                                       char *output, size_t capacity) {
    char _lower_k[ZLANG_EMITTED_NAME_CAPACITY];
    char _base_k[ZLANG_EMITTED_NAME_CAPACITY];
    return zlang_lower_name(struct_name, _lower_k, sizeof(_lower_k)) &&
           zlang_assemble_name("zlang_vtable_", _lower_k, "", _base_k, sizeof(_base_k)) &&
           zlang_make_unique_name(emitter, _base_k, output, capacity);
}

static bool zlang_type_tag_name(zlang_emitter_t *emitter, const char *struct_name, char *output,
                                size_t capacity) {
    char _upper_k[ZLANG_EMITTED_NAME_CAPACITY];
    char _base_k[ZLANG_EMITTED_NAME_CAPACITY];
    return zlang_upper_name(struct_name, _upper_k, sizeof(_upper_k)) &&
           zlang_assemble_name("ZLANG_TYPE_", _upper_k, "", _base_k, sizeof(_base_k)) &&
           zlang_make_unique_name(emitter, _base_k, output, capacity);
}

static bool zlang_emit_type_tags(const zlang_program_t *program, zlang_emitter_t *emitter) {
    size_t _count_k = 0U;
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ != zlang_statement_struct_ ||
            zlang_is_generic_template(_statement_k) ||
            !zlang_struct_is_polymorphic(program, _statement_k->name_)) {
            continue;
        }
        char _macro_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (_count_k == 0U && !zlang_append_text(emitter, "/* generated type tags */\n")) {
            return false;
        }
        if (!zlang_type_tag_name(emitter, _statement_k->name_, _macro_k, sizeof(_macro_k)) ||
            !zlang_append_text(emitter, "#define ") || !zlang_append_text(emitter, _macro_k) ||
            !zlang_append_text(emitter, " ") ||
            !zlang_append_unsigned(emitter, (uint32_t)(_count_k + 1U)) ||
            !zlang_append_text(emitter, "U\n")) {
            return false;
        }
        ++_count_k;
    }
    return _count_k == 0U || zlang_append_text(emitter, "\n");
}

static bool zlang_emit_vtable_type_declarations(const zlang_program_t *program,
                                                zlang_emitter_t *emitter) {
    bool _header_k = false;
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (_statement_k->kind_ != zlang_statement_struct_ || _statement_k->base_name_ != NULL ||
            !zlang_struct_has_descendant(program, _statement_k->name_)) {
            continue;
        }
        char _type_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (!_header_k && !zlang_append_text(emitter, "/* generated vtable types */\n")) {
            return false;
        }
        _header_k = true;
        if (!zlang_vtable_type_name(program, emitter, _statement_k->name_, _type_k,
                                    sizeof(_type_k)) ||
            !zlang_append_text(emitter, "typedef struct ") ||
            !zlang_append_text(emitter, _type_k) || !zlang_append_text(emitter, " ") ||
            !zlang_append_text(emitter, _type_k) || !zlang_append_text(emitter, ";\n")) {
            return false;
        }
    }
    return !_header_k || zlang_append_text(emitter, "\n");
}

static bool zlang_emit_vtable_initializer(const zlang_program_t *program, zlang_emitter_t *emitter,
                                          const char *struct_name) {
    const zlang_ast_statement_t *const _root_k = zlang_struct_root_name(program, struct_name);
    char _instance_k[ZLANG_EMITTED_NAME_CAPACITY];
    char _tag_k[ZLANG_EMITTED_NAME_CAPACITY];
    if (_root_k == NULL ||
        !zlang_vtable_instance_name(emitter, struct_name, _instance_k, sizeof(_instance_k)) ||
        !zlang_type_tag_name(emitter, struct_name, _tag_k, sizeof(_tag_k))) {
        return false;
    }
    const uint32_t _depth_k = zlang_struct_depth(program, _root_k->name_, struct_name);
    return zlang_append_text(emitter, "    self") &&
           zlang_emit_base_chain(emitter, _depth_k, false) &&
           zlang_append_text(emitter, ".zlang_vtable_ = &") &&
           zlang_append_text(emitter, _instance_k) && zlang_append_text(emitter, ";\n    self") &&
           zlang_emit_base_chain(emitter, _depth_k, false) &&
           zlang_append_text(emitter, ".zlang_type_ = ") && zlang_append_text(emitter, _tag_k) &&
           zlang_append_text(emitter, ";\n");
}

static bool zlang_emit_vtable_definitions(const zlang_program_t *program,
                                          zlang_emitter_t *emitter) {
    bool _header_k = false;
    for (size_t _owner = 0U; _owner < program->statement_count_; ++_owner) {
        const zlang_ast_statement_t *const _structure_k = &program->statements_[_owner];
        if (_structure_k->kind_ != zlang_statement_struct_ || _structure_k->base_name_ != NULL ||
            !zlang_struct_has_descendant(program, _structure_k->name_)) {
            continue;
        }
        char _type_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (!_header_k && !zlang_append_text(emitter, "/* generated vtable definitions */\n\n")) {
            return false;
        }
        _header_k = true;
        if (!zlang_vtable_type_name(program, emitter, _structure_k->name_, _type_k,
                                    sizeof(_type_k)) ||
            !zlang_append_text(emitter, "struct ") || !zlang_append_text(emitter, _type_k) ||
            !zlang_append_text(emitter, " {\n")) {
            return false;
        }
        bool _slot_k = false;
        for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
            if (!zlang_is_vtable_slot(program, _structure_k->name_, _index)) {
                continue;
            }
            const zlang_ast_statement_t *const _method_k = &program->statements_[_index];
            const char *const _return_k =
                zlang_generated_type(program, emitter, _method_k->type_name_);
            char _slot_name_k[ZLANG_EMITTED_NAME_CAPACITY];
            if (_return_k == NULL ||
                !zlang_slot_name(_method_k->name_, _slot_name_k, sizeof(_slot_name_k)) ||
                !zlang_append_text(emitter, "    ") || !zlang_append_text(emitter, _return_k) ||
                !zlang_append_text(emitter, " (*") || !zlang_append_text(emitter, _slot_name_k) ||
                !zlang_append_text(emitter, ")(void *self")) {
                return false;
            }
            for (size_t _param = 0U; _param < _method_k->parameter_count_; ++_param) {
                const zlang_parameter_t *const _parameter_k =
                    &program->parameters_[_method_k->parameter_start_ + _param];
                const char *const _parameter_type_k =
                    zlang_generated_type(program, emitter, _parameter_k->type_name_);
                if (_parameter_type_k == NULL || !zlang_append_text(emitter, ", ") ||
                    !zlang_append_text(emitter, _parameter_type_k)) {
                    return false;
                }
            }
            if (!zlang_append_text(emitter, ");\n")) {
                return false;
            }
            _slot_k = true;
        }
        if (!_slot_k && !zlang_append_text(emitter, "    uint32_t zlang_reserved_;\n")) {
            return false;
        }
        if (!zlang_append_text(emitter, "};\n\n")) {
            return false;
        }
    }
    return true;
}

static bool zlang_emit_user_types(const zlang_program_t *program, zlang_emitter_t *emitter) {
    if (emitter->emitted_user_type_count_ == 0U) {
        return true;
    }
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
                const char *const _generated_k = zlang_generated_enum_item(emitter, _item_k->name_);
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
            const bool _polymorphic_k = _statement_k->base_name_ == NULL &&
                                        zlang_struct_has_descendant(program, _statement_k->name_);
            char _vtable_type_k[ZLANG_EMITTED_NAME_CAPACITY];
            const zlang_emitted_user_type_t *const _base_type_k =
                _statement_k->base_name_ != NULL
                    ? zlang_find_user_type(emitter, _statement_k->base_name_)
                    : NULL;
            if (_statement_k->base_name_ != NULL && _base_type_k == NULL) {
                return false;
            }
            if (_base_type_k != NULL && (!zlang_append_text(emitter, "    ") ||
                                         !zlang_append_text(emitter, _base_type_k->type_name_) ||
                                         !zlang_append_text(emitter, " zlang_base_;\n"))) {
                return false;
            }
            if (_polymorphic_k &&
                (!zlang_vtable_type_name(program, emitter, _statement_k->name_, _vtable_type_k,
                                         sizeof(_vtable_type_k)) ||
                 !zlang_append_text(emitter, "    const ") ||
                 !zlang_append_text(emitter, _vtable_type_k) ||
                 !zlang_append_text(emitter, " *zlang_vtable_;\n") ||
                 !zlang_append_text(emitter, "    uint32_t zlang_type_;\n"))) {
                return false;
            }
            const size_t _end_k = _statement_k->body_start_ + _statement_k->body_count_;
            for (size_t _field = _statement_k->body_start_;
                 _field < _end_k && _field < program->statement_count_; ++_field) {
                const zlang_ast_statement_t *const _field_k = &program->statements_[_field];
                if (_field_k->kind_ == zlang_statement_function_ ||
                    zlang_statement_in_method(program, _statement_k, _field)) {
                    continue;
                }
                char _field_array_type_k[ZLANG_EMITTED_NAME_CAPACITY];
                const char *_type_k = NULL;
                if (_field_k->kind_ == zlang_statement_array_declaration_) {
                    if (zlang_canonical_array_name(
                            program, emitter, _field_k->type_name_, _field_k->rank_,
                            _field_k->extents_, _field_array_type_k, sizeof(_field_array_type_k))) {
                        _type_k = _field_array_type_k;
                    }
                } else {
                    _type_k = zlang_generated_type(program, emitter, _field_k->type_name_);
                }
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
    return true;
}

static bool zlang_emit_user_result_types(const zlang_program_t *program, zlang_emitter_t *emitter) {
    if (emitter->emitted_user_result_count_ == 0U) {
        return true;
    }
    if (!zlang_append_text(emitter, "/* generated user result types */\n")) {
        return false;
    }
    for (size_t _index = 0U; _index < emitter->emitted_user_result_count_; ++_index) {
        const zlang_emitted_user_result_t *const _entry_k = &emitter->emitted_user_results_[_index];
        const zlang_emitted_user_type_t *const _payload_k =
            zlang_find_user_type(emitter, _entry_k->payload_name_);
        if (_payload_k == NULL || !zlang_append_text(emitter, "typedef struct ") ||
            !zlang_append_text(emitter, _entry_k->type_name_) ||
            !zlang_append_text(emitter, " {\n    zlang_std_status_t code_;\n    ") ||
            !zlang_append_text(emitter, _payload_k->type_name_) ||
            !zlang_append_text(emitter, " value_;\n    zlang_string_t what_;\n"
                                        "    ZLANG_STD_RESULT_TIME_FIELD\n"
                                        "    ZLANG_STD_RESULT_DUMP_FIELD\n} ") ||
            !zlang_append_text(emitter, _entry_k->type_name_) ||
            !zlang_append_text(emitter, ";\n\n")) {
            return false;
        }
    }
    (void)program;
    return true;
}

static bool zlang_emit_user_result_helpers(const zlang_program_t *program,
                                           zlang_emitter_t *emitter) {
    if (emitter->emitted_user_result_count_ == 0U || emitter->collecting_array_helpers_ ||
        (!emitter->emit_timing_ && !emitter->emit_dump_)) {
        return true;
    }
    bool _header_k = false;
    for (size_t _index = 0U; _index < emitter->emitted_user_result_count_; ++_index) {
        const zlang_emitted_user_result_t *const _entry_k = &emitter->emitted_user_results_[_index];
        const char *_owner_k = NULL;
        bool _is_trait_k = false;
        const zlang_ast_statement_t *const _dump_k =
            zlang_find_method(program, _entry_k->payload_name_, "dump", &_owner_k, &_is_trait_k);
        if (_dump_k == NULL || _owner_k == NULL) {
            return false;
        }
        char _dump_symbol_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (!zlang_method_generated_name(_owner_k, "dump", false, _dump_symbol_k,
                                         sizeof(_dump_symbol_k))) {
            return false;
        }
        if (!_header_k) {
            if (!zlang_append_text(emitter, "/* generated user result capabilities */\n\n")) {
                return false;
            }
            _header_k = true;
        }
        if (emitter->emit_timing_) {
            if (!zlang_append_text(emitter, "ZLANG_STATIC_INLINE uint64_t ") ||
                !zlang_append_text(emitter, _entry_k->type_name_) ||
                !zlang_append_text(emitter, "_time(") ||
                !zlang_append_text(emitter, _entry_k->type_name_) ||
                !zlang_append_text(emitter,
                                   " result) {\n"
                                   "    (void)result;\n"
                                   "    return (uint64_t)ZLANG_STD_RESULT_DUMP_TIME(&result);\n"
                                   "}\n")) {
                return false;
            }
        }
        if (emitter->emit_dump_) {
            if (!zlang_append_text(emitter, "ZLANG_STATIC_INLINE zlang_string_t ") ||
                !zlang_append_text(emitter, _entry_k->dump_name_) ||
                !zlang_append_text(emitter, "(") ||
                !zlang_append_text(emitter, _entry_k->type_name_) ||
                !zlang_append_text(emitter, " *result) {\n"
                                            "    if (result == NULL) {\n"
                                            "        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);\n"
                                            "    }\n"
                                            "#if defined(ZLANG_STD_RESULT_DUMP)\n"
                                            "    const zlang_string_t payload = ") ||
                !zlang_append_text(emitter, _dump_symbol_k) ||
                !zlang_append_text(
                    emitter,
                    "(&result->value_);\n"
                    "    return zlang_std_result_dump_compose(\n"
                    "        result->code_, payload, zlang_std_status_message(result->code_),\n"
                    "        ZLANG_STD_RESULT_DUMP_TIME(result),\n"
                    "        result->dump_storage_);\n"
                    "#else\n"
                    "    (void)result;\n"
                    "    return (zlang_string_t){\"\", 0U};\n"
                    "#endif\n"
                    "}\n\n")) {
                return false;
            }
        }
    }
    return true;
}

static bool zlang_emit_generated_declarations(const zlang_program_t *program,
                                              zlang_emitter_t *emitter) {
    return zlang_emit_capacity_macros(emitter) &&
           zlang_emit_specialization_typedefs(program, emitter) &&
           zlang_emit_type_tags(program, emitter) &&
           zlang_emit_vtable_type_declarations(program, emitter) &&
           zlang_emit_array_typedefs(emitter, program, true) &&
           zlang_emit_user_types(program, emitter) &&
           zlang_emit_user_result_types(program, emitter) &&
           zlang_emit_vtable_definitions(program, emitter) &&
           zlang_emit_array_typedefs(emitter, program, false) &&
           zlang_emit_literal_declarations(emitter, program);
}

static bool zlang_emit_function_prototype(const zlang_program_t *program, zlang_emitter_t *emitter,
                                          const zlang_emitted_function_t *entry) {
    const zlang_ast_statement_t *const _statement_k = entry->statement_;
    const zlang_array_specialization_t *const _specialization_k = entry->specialization_;
    if (_statement_k->is_extern_) {
        return true;
    }
    char _return_array_type_k[ZLANG_EMITTED_NAME_CAPACITY];
    const char *_return_type_k = NULL;
    if (_statement_k->is_array_return_ && _specialization_k != NULL) {
        if (zlang_canonical_array_name(program, emitter, _specialization_k->type_name_,
                                       _specialization_k->rank_, _specialization_k->extents_,
                                       _return_array_type_k, sizeof(_return_array_type_k))) {
            _return_type_k = _return_array_type_k;
        }
    } else if (_statement_k->is_array_return_) {
        if (zlang_canonical_array_name(program, emitter, _statement_k->type_name_,
                                       _statement_k->return_rank_, _statement_k->return_extents_,
                                       _return_array_type_k, sizeof(_return_array_type_k))) {
            _return_type_k = _return_array_type_k;
        }
    } else {
        _return_type_k = zlang_generated_type(program, emitter, _statement_k->type_name_);
    }
    if (_return_type_k == NULL || !zlang_append_text(emitter, _return_type_k) ||
        !zlang_append_text(emitter, " ") || !zlang_append_text(emitter, entry->generated_name_) ||
        !zlang_append_text(emitter, "(")) {
        return false;
    }
    if (_statement_k->parameter_count_ == 0U && !entry->needs_state_ &&
        !zlang_append_text(emitter, "void")) {
        return false;
    }
    for (size_t _index = 0U; _index < _statement_k->parameter_count_; ++_index) {
        const zlang_parameter_t *const _parameter_k =
            &program->parameters_[_statement_k->parameter_start_ + _index];
        const char *_parameter_type_name_k = _parameter_k->type_name_;
        uint32_t _parameter_rank_k = _parameter_k->rank_;
        const uint32_t *_parameter_extents_k = _parameter_k->extents_;
        if (_parameter_k->is_array_ && _specialization_k != NULL) {
            _parameter_type_name_k = _specialization_k->type_name_;
            _parameter_rank_k = _specialization_k->rank_;
            _parameter_extents_k = _specialization_k->extents_;
        }
        char _array_type_k[ZLANG_EMITTED_NAME_CAPACITY];
        const char *_parameter_type_k = NULL;
        if (_parameter_k->is_array_) {
            if (zlang_canonical_array_name(program, emitter, _parameter_type_name_k,
                                           _parameter_rank_k, _parameter_extents_k, _array_type_k,
                                           sizeof(_array_type_k))) {
                _parameter_type_k = _array_type_k;
            }
        } else {
            _parameter_type_k = zlang_generated_type(program, emitter, _parameter_type_name_k);
        }
        if (_parameter_type_k == NULL || (_index > 0U && !zlang_append_text(emitter, ", ")) ||
            (_parameter_k->is_const_ && !zlang_append_text(emitter, "const ")) ||
            !zlang_append_text(emitter, _parameter_type_k) ||
            (_parameter_k->is_reference_ && !zlang_append_text(emitter, " *")) ||
            (!_parameter_k->is_reference_ && !zlang_append_text(emitter, " ")) ||
            !zlang_append_text(emitter, "app_param_") ||
            !zlang_append_text(emitter, _parameter_k->name_)) {
            return false;
        }
    }
    if (entry->needs_state_ &&
        (!zlang_append_text(emitter, _statement_k->parameter_count_ == 0U ? "" : ", ") ||
         !zlang_append_text(emitter, "zlang_runtime_context_t *zlang_context"))) {
        return false;
    }
    return zlang_append_text(emitter, ");\n");
}

static bool zlang_emit_function_prototypes(const zlang_program_t *program,
                                           zlang_emitter_t *emitter) {
    if (emitter->emitted_function_count_ == 0U) {
        return true;
    }
    if (!zlang_append_text(emitter, "/* generated function prototypes */\n\n")) {
        return false;
    }
    for (size_t _index = 0U; _index < emitter->emitted_function_count_; ++_index) {
        if (!zlang_emit_function_prototype(program, emitter,
                                           &emitter->emitted_functions_[_index])) {
            return false;
        }
    }
    return zlang_append_text(emitter, "\n");
}

static bool zlang_emit_method_signature(const zlang_program_t *program, zlang_emitter_t *emitter,
                                        const zlang_ast_statement_t *method,
                                        const char *owner_struct_name, const char *generated_name) {
    const zlang_ast_statement_t *const _struct_k =
        zlang_find_struct_statement(program, owner_struct_name);
    const char *const _return_type_k = zlang_generated_type(
        program, emitter, method->is_constructor_ ? owner_struct_name : method->type_name_);
    const zlang_emitted_user_type_t *const _struct_type_k =
        _struct_k != NULL ? zlang_find_user_type(emitter, _struct_k->name_) : NULL;
    if (_return_type_k == NULL || _struct_type_k == NULL) {
        return false;
    }
    if (!zlang_append_text(emitter, _return_type_k) || !zlang_append_text(emitter, " ") ||
        !zlang_append_text(emitter, generated_name) || !zlang_append_text(emitter, "(")) {
        return false;
    }
    bool _first_k = true;
    if (!method->is_static_ && !method->is_constructor_) {
        if (!zlang_append_text(emitter, _struct_type_k->type_name_) ||
            !zlang_append_text(emitter, " *self")) {
            return false;
        }
        _first_k = false;
    }
    for (size_t _index = 0U; _index < method->parameter_count_; ++_index) {
        const zlang_parameter_t *const _parameter_k =
            &program->parameters_[method->parameter_start_ + _index];
        const char *const _parameter_type_k =
            zlang_generated_type(program, emitter, _parameter_k->type_name_);
        if (_parameter_type_k == NULL || (!_first_k && !zlang_append_text(emitter, ", ")) ||
            !zlang_append_text(emitter, _parameter_type_k) || !zlang_append_text(emitter, " ") ||
            !zlang_append_text(emitter, "app_param_") ||
            !zlang_append_text(emitter, _parameter_k->name_)) {
            return false;
        }
        _first_k = false;
    }
    if (_first_k && !zlang_append_text(emitter, "void")) {
        return false;
    }
    return zlang_append_text(emitter, ")");
}

static bool zlang_emit_vtable_thunk_definitions(const zlang_program_t *program,
                                                zlang_emitter_t *emitter) {
    for (size_t _owner = 0U; _owner < program->statement_count_; ++_owner) {
        const zlang_ast_statement_t *const _structure_k = &program->statements_[_owner];
        if (_structure_k->kind_ != zlang_statement_struct_ ||
            zlang_is_generic_template(_structure_k) ||
            !zlang_struct_is_polymorphic(program, _structure_k->name_)) {
            continue;
        }
        const zlang_ast_statement_t *const _root_k =
            zlang_struct_root_name(program, _structure_k->name_);
        if (_root_k == NULL) {
            return false;
        }
        for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
            if (!zlang_is_vtable_slot(program, _root_k->name_, _index)) {
                continue;
            }
            const zlang_ast_statement_t *const _slot_method_k = &program->statements_[_index];
            const char *_owner_k = NULL;
            bool _is_trait_k = false;
            const zlang_ast_statement_t *const _effective_k = zlang_find_method(
                program, _structure_k->name_, _slot_method_k->name_, &_owner_k, &_is_trait_k);
            if (_effective_k == NULL || _is_trait_k) {
                continue;
            }
            char _name_k[ZLANG_EMITTED_NAME_CAPACITY];
            const char *const _return_k =
                zlang_generated_type(program, emitter, _slot_method_k->type_name_);
            if (_return_k == NULL ||
                !zlang_vtable_wrapper_name(_structure_k->name_, _slot_method_k->name_, _name_k,
                                           sizeof(_name_k)) ||
                !zlang_append_text(emitter, "static ") || !zlang_append_text(emitter, _return_k) ||
                !zlang_append_text(emitter, " ") || !zlang_append_text(emitter, _name_k) ||
                !zlang_append_text(emitter, "(void *self")) {
                return false;
            }
            for (size_t _param = 0U; _param < _slot_method_k->parameter_count_; ++_param) {
                const zlang_parameter_t *const _parameter_k =
                    &program->parameters_[_slot_method_k->parameter_start_ + _param];
                const char *const _parameter_type_k =
                    zlang_generated_type(program, emitter, _parameter_k->type_name_);
                if (_parameter_type_k == NULL || !zlang_append_text(emitter, ", ") ||
                    !zlang_append_text(emitter, _parameter_type_k) ||
                    !zlang_append_text(emitter, " app_param_") ||
                    !zlang_append_text(emitter, _parameter_k->name_)) {
                    return false;
                }
            }
            if (!zlang_append_text(emitter, ") {\n    ") ||
                (strcmp(_return_k, "void") != 0 && !zlang_append_text(emitter, "return "))) {
                return false;
            }
            char _method_name_k[ZLANG_EMITTED_NAME_CAPACITY];
            const zlang_emitted_user_type_t *const _struct_type_k =
                zlang_find_user_type(emitter, _owner_k);
            if (_struct_type_k == NULL ||
                !zlang_method_generated_name(_owner_k, _slot_method_k->name_, false, _method_name_k,
                                             sizeof(_method_name_k)) ||
                !zlang_append_text(emitter, _method_name_k) || !zlang_append_text(emitter, "((") ||
                !zlang_append_text(emitter, _struct_type_k->type_name_) ||
                !zlang_append_text(emitter, " *)self")) {
                return false;
            }
            for (size_t _param = 0U; _param < _slot_method_k->parameter_count_; ++_param) {
                const zlang_parameter_t *const _parameter_k =
                    &program->parameters_[_slot_method_k->parameter_start_ + _param];
                if (!zlang_append_text(emitter, ", app_param_") ||
                    !zlang_append_text(emitter, _parameter_k->name_)) {
                    return false;
                }
            }
            if (!zlang_append_text(emitter, ");\n}\n\n")) {
                return false;
            }
        }
    }
    return true;
}

static bool zlang_emit_vtable_thunk_prototypes(const zlang_program_t *program,
                                               zlang_emitter_t *emitter) {
    for (size_t _owner = 0U; _owner < program->statement_count_; ++_owner) {
        const zlang_ast_statement_t *const _structure_k = &program->statements_[_owner];
        if (_structure_k->kind_ != zlang_statement_struct_ ||
            zlang_is_generic_template(_structure_k) ||
            !zlang_struct_is_polymorphic(program, _structure_k->name_)) {
            continue;
        }
        const zlang_ast_statement_t *const _root_k =
            zlang_struct_root_name(program, _structure_k->name_);
        if (_root_k == NULL) {
            return false;
        }
        for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
            if (!zlang_is_vtable_slot(program, _root_k->name_, _index)) {
                continue;
            }
            const zlang_ast_statement_t *const _method_k = &program->statements_[_index];
            const char *_owner_k = NULL;
            bool _is_trait_k = false;
            const zlang_ast_statement_t *const _effective_k = zlang_find_method(
                program, _structure_k->name_, _method_k->name_, &_owner_k, &_is_trait_k);
            if (_effective_k == NULL || _is_trait_k) {
                continue;
            }
            char _name_k[ZLANG_EMITTED_NAME_CAPACITY];
            const char *const _return_k =
                zlang_generated_type(program, emitter, _method_k->type_name_);
            if (_return_k == NULL ||
                !zlang_vtable_wrapper_name(_structure_k->name_, _method_k->name_, _name_k,
                                           sizeof(_name_k)) ||
                !zlang_append_text(emitter, "static ") || !zlang_append_text(emitter, _return_k) ||
                !zlang_append_text(emitter, " ") || !zlang_append_text(emitter, _name_k) ||
                !zlang_append_text(emitter, "(void *self")) {
                return false;
            }
            for (size_t _param = 0U; _param < _method_k->parameter_count_; ++_param) {
                const zlang_parameter_t *const _parameter_k =
                    &program->parameters_[_method_k->parameter_start_ + _param];
                const char *const _parameter_type_k =
                    zlang_generated_type(program, emitter, _parameter_k->type_name_);
                if (_parameter_type_k == NULL || !zlang_append_text(emitter, ", ") ||
                    !zlang_append_text(emitter, _parameter_type_k) ||
                    !zlang_append_text(emitter, " app_param_") ||
                    !zlang_append_text(emitter, _parameter_k->name_)) {
                    return false;
                }
            }
            if (!zlang_append_text(emitter, ");\n")) {
                return false;
            }
        }
    }
    return true;
}

static bool zlang_emit_vtable_instances(const zlang_program_t *program, zlang_emitter_t *emitter) {
    bool _header_k = false;
    for (size_t _owner = 0U; _owner < program->statement_count_; ++_owner) {
        const zlang_ast_statement_t *const _structure_k = &program->statements_[_owner];
        if (_structure_k->kind_ != zlang_statement_struct_ ||
            zlang_is_generic_template(_structure_k) ||
            !zlang_struct_is_polymorphic(program, _structure_k->name_)) {
            continue;
        }
        const zlang_ast_statement_t *const _root_k =
            zlang_struct_root_name(program, _structure_k->name_);
        if (_root_k == NULL) {
            return false;
        }
        char _type_k[ZLANG_EMITTED_NAME_CAPACITY];
        char _instance_k[ZLANG_EMITTED_NAME_CAPACITY];
        if (!_header_k && !zlang_append_text(emitter, "/* generated vtables */\n\n")) {
            return false;
        }
        _header_k = true;
        if (!zlang_vtable_type_name(program, emitter, _root_k->name_, _type_k, sizeof(_type_k)) ||
            !zlang_vtable_instance_name(emitter, _structure_k->name_, _instance_k,
                                        sizeof(_instance_k)) ||
            !zlang_append_text(emitter, "const ") || !zlang_append_text(emitter, _type_k) ||
            !zlang_append_text(emitter, " ") || !zlang_append_text(emitter, _instance_k) ||
            !zlang_append_text(emitter, " = {")) {
            return false;
        }
        bool _slot_k = false;
        for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
            if (!zlang_is_vtable_slot(program, _root_k->name_, _index)) {
                continue;
            }
            const zlang_ast_statement_t *const _slot_method_k = &program->statements_[_index];
            const char *_owner_k = NULL;
            bool _is_trait_k = false;
            const zlang_ast_statement_t *const _effective_k = zlang_find_method(
                program, _structure_k->name_, _slot_method_k->name_, &_owner_k, &_is_trait_k);
            if (!zlang_append_text(emitter, _slot_k ? ", " : " ")) {
                return false;
            }
            if (_effective_k == NULL || _is_trait_k) {
                if (!zlang_append_text(emitter, "0")) {
                    return false;
                }
            } else {
                char _thunk_k[ZLANG_EMITTED_NAME_CAPACITY];
                if (!zlang_vtable_wrapper_name(_structure_k->name_, _slot_method_k->name_, _thunk_k,
                                               sizeof(_thunk_k)) ||
                    !zlang_append_text(emitter, _thunk_k)) {
                    return false;
                }
            }
            _slot_k = true;
        }
        if (!_slot_k && !zlang_append_text(emitter, "0")) {
            return false;
        }
        if (!zlang_append_text(emitter, " };\n")) {
            return false;
        }
    }
    return !_header_k || zlang_append_text(emitter, "\n");
}

static bool zlang_emit_trait_method_prototypes(const zlang_program_t *program,
                                               zlang_emitter_t *emitter) {
    for (size_t _owner = 0U; _owner < program->statement_count_; ++_owner) {
        const zlang_ast_statement_t *const _structure_k = &program->statements_[_owner];
        if (_structure_k->kind_ != zlang_statement_struct_ ||
            zlang_is_generic_template(_structure_k)) {
            continue;
        }
        for (size_t _trait = 0U; _trait < _structure_k->uses_count_; ++_trait) {
            const zlang_ast_statement_t *const _trait_k =
                zlang_find_trait_statement(program, _structure_k->uses_names_[_trait]);
            if (_trait_k == NULL) {
                continue;
            }
            bool _requirements_met_k = true;
            for (size_t _requirement = 0U; _requirement < _trait_k->requires_count_;
                 ++_requirement) {
                const char *const _interface_name_k = _trait_k->requires_names_[_requirement];
                if (zlang_find_interface(program, _interface_name_k) != NULL &&
                    !zlang_statement_implements_interface(program, _structure_k,
                                                          _interface_name_k)) {
                    _requirements_met_k = false;
                }
            }
            if (!_requirements_met_k) {
                continue;
            }
            const size_t _end_k = _trait_k->body_start_ + _trait_k->body_count_;
            for (size_t _member = _trait_k->body_start_;
                 _member < _end_k && _member < program->statement_count_; ++_member) {
                const zlang_ast_statement_t *const _method_k = &program->statements_[_member];
                if (_method_k->kind_ != zlang_statement_function_ ||
                    zlang_statement_in_method(program, _trait_k, _member) ||
                    _method_k->body_count_ == 0U ||
                    _method_k->body_start_ >= program->statement_count_) {
                    continue;
                }
                char _name_k[ZLANG_EMITTED_NAME_CAPACITY];
                if (!zlang_method_generated_name(_structure_k->name_, _method_k->name_, false,
                                                 _name_k, sizeof(_name_k)) ||
                    !zlang_emit_method_signature(program, emitter, _method_k, _structure_k->name_,
                                                 _name_k) ||
                    !zlang_append_text(emitter, ";\n")) {
                    return false;
                }
            }
        }
    }
    return true;
}

static bool zlang_emit_method_prototypes(const zlang_program_t *program, zlang_emitter_t *emitter) {
    bool _header_k = false;
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _owner_k = &program->statements_[_index];
        if (_owner_k->kind_ != zlang_statement_struct_ || zlang_is_generic_template(_owner_k)) {
            continue;
        }
        const size_t _owner_end_k = _owner_k->body_start_ + _owner_k->body_count_;
        for (size_t _member = _owner_k->body_start_;
             _member < _owner_end_k && _member < program->statement_count_; ++_member) {
            const zlang_ast_statement_t *const _method_k = &program->statements_[_member];
            if (_method_k->kind_ != zlang_statement_function_ || _method_k->owner_name_ == NULL ||
                zlang_statement_in_method(program, _owner_k, _member)) {
                continue;
            }
            if (!_header_k) {
                if (!zlang_append_text(emitter, "/* generated method prototypes */\n\n")) {
                    return false;
                }
                _header_k = true;
            }
            char _name_k[ZLANG_EMITTED_NAME_CAPACITY];
            if (!zlang_method_generated_name(_owner_k->name_, _method_k->name_,
                                             _method_k->is_constructor_, _name_k,
                                             sizeof(_name_k)) ||
                !zlang_emit_method_signature(program, emitter, _method_k, _owner_k->name_,
                                             _name_k) ||
                !zlang_append_text(emitter, ";\n")) {
                return false;
            }
        }
    }
    return !_header_k || zlang_append_text(emitter, "\n");
}

static bool zlang_emit_trait_method_definitions(const zlang_program_t *program,
                                                zlang_emitter_t *emitter) {
    for (size_t _owner = 0U; _owner < program->statement_count_; ++_owner) {
        const zlang_ast_statement_t *const _structure_k = &program->statements_[_owner];
        if (_structure_k->kind_ != zlang_statement_struct_ ||
            zlang_is_generic_template(_structure_k)) {
            continue;
        }
        for (size_t _trait = 0U; _trait < _structure_k->uses_count_; ++_trait) {
            const zlang_ast_statement_t *const _trait_k =
                zlang_find_trait_statement(program, _structure_k->uses_names_[_trait]);
            if (_trait_k == NULL) {
                continue;
            }
            bool _requirements_met_k = true;
            for (size_t _requirement = 0U; _requirement < _trait_k->requires_count_;
                 ++_requirement) {
                const char *const _interface_name_k = _trait_k->requires_names_[_requirement];
                if (zlang_find_interface(program, _interface_name_k) != NULL &&
                    !zlang_statement_implements_interface(program, _structure_k,
                                                          _interface_name_k)) {
                    _requirements_met_k = false;
                }
            }
            if (!_requirements_met_k) {
                continue;
            }
            const size_t _end_k = _trait_k->body_start_ + _trait_k->body_count_;
            for (size_t _member = _trait_k->body_start_;
                 _member < _end_k && _member < program->statement_count_; ++_member) {
                const zlang_ast_statement_t *const _method_k = &program->statements_[_member];
                if (_method_k->kind_ != zlang_statement_function_ ||
                    zlang_statement_in_method(program, _trait_k, _member) ||
                    _method_k->body_count_ == 0U ||
                    _method_k->body_start_ >= program->statement_count_) {
                    continue;
                }
                char _name_k[ZLANG_EMITTED_NAME_CAPACITY];
                if (!zlang_method_generated_name(_structure_k->name_, _method_k->name_, false,
                                                 _name_k, sizeof(_name_k)) ||
                    !zlang_emit_method_signature(program, emitter, _method_k, _structure_k->name_,
                                                 _name_k) ||
                    !zlang_append_text(emitter, " {\n")) {
                    return false;
                }
                const zlang_ast_statement_t *const _previous_struct_k = emitter->current_struct_;
                const char *const _previous_self_k = emitter->current_self_name_;
                const bool _previous_pointer_k = emitter->current_self_is_pointer_;
                const char *const _previous_return_k = emitter->current_return_type_;
                emitter->current_struct_ = _structure_k;
                emitter->current_self_name_ = "self";
                emitter->current_self_is_pointer_ = !_method_k->is_static_;
                emitter->current_return_type_ = _method_k->type_name_;
                const zlang_emitter_scope_t _scope_k = zlang_save_scope(emitter);
                bool _result_k = _method_k->is_static_ ||
                                 zlang_register_self_symbol(emitter, _structure_k->name_,
                                                            emitter->current_self_is_pointer_);
                for (size_t _parameter = 0U; _result_k && _parameter < _method_k->parameter_count_;
                     ++_parameter) {
                    const zlang_parameter_t *const _parameter_k =
                        &program->parameters_[_method_k->parameter_start_ + _parameter];
                    char _generated_k[ZLANG_EMITTED_NAME_CAPACITY];
                    _result_k = zlang_register_method_parameter(emitter, _parameter_k, _generated_k,
                                                                sizeof(_generated_k));
                }
                if (_result_k) {
                    _result_k = zlang_emit_body(program, _method_k->body_start_,
                                                _method_k->body_count_, emitter, 1U);
                }
                zlang_restore_scope(emitter, _scope_k);
                emitter->current_struct_ = _previous_struct_k;
                emitter->current_self_name_ = _previous_self_k;
                emitter->current_self_is_pointer_ = _previous_pointer_k;
                emitter->current_return_type_ = _previous_return_k;
                if (!_result_k || !zlang_append_text(emitter, "}\n\n")) {
                    return false;
                }
            }
        }
    }
    return true;
}

static bool zlang_emit_method_definitions(const zlang_program_t *program,
                                          zlang_emitter_t *emitter) {
    bool _header_k = false;
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _owner_k = &program->statements_[_index];
        if (_owner_k->kind_ != zlang_statement_struct_ || zlang_is_generic_template(_owner_k)) {
            continue;
        }
        const size_t _owner_end_k = _owner_k->body_start_ + _owner_k->body_count_;
        for (size_t _member = _owner_k->body_start_;
             _member < _owner_end_k && _member < program->statement_count_; ++_member) {
            const zlang_ast_statement_t *const _method_k = &program->statements_[_member];
            if (_method_k->kind_ != zlang_statement_function_ || _method_k->owner_name_ == NULL ||
                zlang_statement_in_method(program, _owner_k, _member)) {
                continue;
            }
            if (!_header_k) {
                if (!zlang_append_text(emitter, "/* generated methods */\n\n")) {
                    return false;
                }
                _header_k = true;
            }
            char _name_k[ZLANG_EMITTED_NAME_CAPACITY];
            if (!zlang_method_generated_name(_method_k->owner_name_, _method_k->name_,
                                             _method_k->is_constructor_, _name_k,
                                             sizeof(_name_k)) ||
                !zlang_emit_method_signature(program, emitter, _method_k, _method_k->owner_name_,
                                             _name_k) ||
                !zlang_append_text(emitter, " {\n")) {
                return false;
            }
            const zlang_ast_statement_t *const _previous_struct_k = emitter->current_struct_;
            const char *const _previous_self_k = emitter->current_self_name_;
            const bool _previous_pointer_k = emitter->current_self_is_pointer_;
            emitter->current_struct_ = zlang_find_struct_statement(program, _method_k->owner_name_);
            emitter->current_self_name_ = "self";
            emitter->current_self_is_pointer_ =
                !_method_k->is_static_ && !_method_k->is_constructor_;
            const zlang_emitter_scope_t _scope_k = zlang_save_scope(emitter);
            bool _result_k = _method_k->is_static_ ||
                             zlang_register_self_symbol(emitter,
                                                        emitter->current_struct_ != NULL
                                                            ? emitter->current_struct_->name_
                                                            : _method_k->owner_name_,
                                                        emitter->current_self_is_pointer_);
            for (size_t _parameter = 0U; _result_k && _parameter < _method_k->parameter_count_;
                 ++_parameter) {
                const zlang_parameter_t *const _parameter_k =
                    &program->parameters_[_method_k->parameter_start_ + _parameter];
                char _generated_k[ZLANG_EMITTED_NAME_CAPACITY];
                _result_k = zlang_register_method_parameter(emitter, _parameter_k, _generated_k,
                                                            sizeof(_generated_k));
            }
            if (_result_k && _method_k->is_constructor_) {
                const zlang_ast_statement_t *const _struct_k = emitter->current_struct_;
                const zlang_emitted_user_type_t *const _struct_type_k =
                    _struct_k != NULL ? zlang_find_user_type(emitter, _struct_k->name_) : NULL;
                _result_k = _struct_type_k != NULL && zlang_append_text(emitter, "    ") &&
                            zlang_append_text(emitter, _struct_type_k->type_name_) &&
                            zlang_append_text(emitter, " self = { ZLANG_ZERO };\n");
                if (_result_k && _struct_k != NULL &&
                    zlang_struct_is_polymorphic(program, _struct_k->name_)) {
                    _result_k = zlang_emit_vtable_initializer(program, emitter, _struct_k->name_);
                }
            }
            if (_result_k) {
                _result_k = zlang_emit_body(program, _method_k->body_start_, _method_k->body_count_,
                                            emitter, 1U);
            }
            if (_result_k && _method_k->is_constructor_) {
                _result_k = zlang_append_text(emitter, "    return self;\n");
            }
            zlang_restore_scope(emitter, _scope_k);
            emitter->current_struct_ = _previous_struct_k;
            emitter->current_self_name_ = _previous_self_k;
            emitter->current_self_is_pointer_ = _previous_pointer_k;
            if (!_result_k || !zlang_append_text(emitter, "}\n\n")) {
                return false;
            }
        }
    }
    return true;
}

static bool zlang_is_type_definition(const zlang_ast_statement_t *statement) {
    return statement->kind_ == zlang_statement_struct_ || statement->kind_ == zlang_statement_enum_;
}

static bool zlang_is_generic_template(const zlang_ast_statement_t *statement) {
    return statement != NULL && statement->is_generic_;
}

static bool zlang_is_file_scope(const zlang_emitter_t *emitter,
                                const zlang_ast_statement_t *statement) {
    return statement->kind_ == zlang_statement_function_ ||
           ((statement->kind_ == zlang_statement_declaration_ ||
             statement->kind_ == zlang_statement_array_declaration_) &&
            zlang_is_static(emitter, statement));
}

static bool zlang_emit_program_pass(const zlang_program_t *program, zlang_emitter_t *emitter) {
    emitter->program_ = program;
    if (emitter->plan_ == NULL || !zlang_activate_std_macros(program, emitter) ||
        !zlang_emit_prologue(program, emitter) || !zlang_emit_common_constants(emitter) ||
        !zlang_collect_user_types(emitter, program) ||
        !zlang_collect_user_result_types(emitter, program) ||
        !zlang_collect_array_specializations(program, emitter) ||
        !zlang_register_function_names(program, emitter) ||
        !zlang_collect_stateful_usage(program, emitter) ||
        !zlang_collect_sequences(emitter, program) || !zlang_collect_capacities(emitter, program) ||
        !zlang_collect_literals(emitter, program) ||
        (!emitter->collecting_array_helpers_ && !zlang_emit_array_helpers(program, emitter)) ||
        !zlang_emit_generated_declarations(program, emitter) ||
        !zlang_emit_method_prototypes(program, emitter) ||
        !zlang_emit_trait_method_prototypes(program, emitter) ||
        !zlang_emit_user_result_helpers(program, emitter) ||
        !zlang_emit_vtable_thunk_prototypes(program, emitter) ||
        !zlang_emit_vtable_instances(program, emitter) ||
        !zlang_emit_function_prototypes(program, emitter)) {
        return false;
    }

    if (!zlang_emit_method_definitions(program, emitter)) {
        return false;
    }
    if (!zlang_emit_trait_method_definitions(program, emitter)) {
        return false;
    }
    if (!zlang_emit_vtable_thunk_definitions(program, emitter)) {
        return false;
    }

    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (zlang_is_nested(program, _index) || zlang_is_type_definition(_statement_k) ||
            zlang_is_generic_template(_statement_k) ||
            _statement_k->kind_ == zlang_statement_import_) {
            continue;
        }
        if (zlang_is_file_scope(emitter, _statement_k)) {
            if (!zlang_emit_statement(program, _statement_k, emitter, 0U)) {
                return false;
            }
        }
    }

    if (emitter->has_explicit_main_) {
        const zlang_emitted_function_t *_main_k = NULL;
        for (size_t _function = 0U; _function < emitter->emitted_function_count_; ++_function) {
            const zlang_emitted_function_t *const _entry_k =
                &emitter->emitted_functions_[_function];
            if (_entry_k->statement_ != NULL && _entry_k->statement_->is_main_) {
                _main_k = _entry_k;
                break;
            }
        }
        if (_main_k == NULL) {
            return false;
        }
        const bool _has_host_arguments_k =
            _main_k->statement_ != NULL && _main_k->statement_->parameter_count_ == 2U;
        if (!_has_host_arguments_k && !zlang_append_text(emitter, "int main(void) {\n")) {
            return false;
        }
        if (_has_host_arguments_k &&
            !zlang_append_text(emitter,
                               "int main(int argc, char **argv) {\n"
                               "    zlang_arguments_t _zlang_arguments = { argc, argv };\n")) {
            return false;
        }
        if (_main_k->needs_state_ &&
            !zlang_append_text(emitter,
                               "    zlang_runtime_context_t _zlang_context = { 0x6D2B79F5U };\n")) {
            return false;
        }
        if (!zlang_append_text(emitter, "    return (int)") ||
            !zlang_append_text(emitter, _main_k->generated_name_)) {
            return false;
        }
        if (_has_host_arguments_k && !zlang_append_text(emitter, "(argc, &_zlang_arguments")) {
            return false;
        }
        if (!_has_host_arguments_k && !zlang_append_text(emitter, "(")) {
            return false;
        }
        if (_main_k->needs_state_ &&
            !zlang_append_text(emitter,
                               _has_host_arguments_k ? ", &_zlang_context" : "&_zlang_context")) {
            return false;
        }
        return zlang_append_text(emitter, ");\n}\n");
    }

    if (!zlang_append_text(emitter, "int main(void) {\n")) {
        return false;
    }
    if (emitter->uses_stateful_std_ &&
        (!zlang_append_text(emitter,
                            "    zlang_runtime_context_t _zlang_context = { 0x6D2B79F5U };\n\n"))) {
        return false;
    }
    bool _has_previous_k = false;
    bool _previous_grouped_k = false;
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (zlang_is_nested(program, _index) || zlang_is_type_definition(_statement_k) ||
            _statement_k->kind_ == zlang_statement_import_ ||
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

bool zlang_emit_program(const zlang_program_t *program, zlang_emitter_t *emitter) {
    if (program == NULL || emitter == NULL || emitter->plan_ == NULL) {
        return false;
    }
    char *const _buffer_k = emitter->buffer_;
    const size_t _capacity_k = emitter->capacity_;
    zlang_diagnostics_t *const _diagnostics_k = emitter->diagnostics_;
    const zlang_memory_plan_t *const _plan_k = emitter->plan_;
    const bool _emit_comments_k = emitter->emit_comments_;
    const bool _emit_timing_k = emitter->emit_timing_;
    const bool _emit_dump_k = emitter->emit_dump_;
    const bool _force_timing_k = emitter->force_timing_;
    const bool _force_dump_k = emitter->force_dump_;
    zlang_emitter_init(emitter, _buffer_k, _capacity_k, _diagnostics_k, _plan_k);
    emitter->emit_comments_ = _emit_comments_k;
    emitter->emit_timing_ = _emit_timing_k;
    emitter->emit_dump_ = _emit_dump_k;
    emitter->force_timing_ = _force_timing_k;
    emitter->force_dump_ = _force_dump_k;
    emitter->collecting_array_helpers_ = true;
    if (!zlang_emit_program_pass(program, emitter)) {
        return false;
    }
    zlang_emitted_array_helper_t _helpers_k[ZLANG_EMITTED_ARRAY_HELPER_CAPACITY];
    const size_t _helper_count_k = emitter->requested_array_helper_count_;
    memcpy(_helpers_k, emitter->requested_array_helpers_, sizeof(_helpers_k));
    zlang_emitter_init(emitter, _buffer_k, _capacity_k, _diagnostics_k, _plan_k);
    emitter->emit_comments_ = _emit_comments_k;
    emitter->emit_timing_ = _emit_timing_k;
    emitter->emit_dump_ = _emit_dump_k;
    emitter->force_timing_ = _force_timing_k;
    emitter->force_dump_ = _force_dump_k;
    emitter->collecting_array_helpers_ = false;
    emitter->requested_array_helper_count_ = _helper_count_k;
    memcpy(emitter->requested_array_helpers_, _helpers_k, sizeof(_helpers_k));
    return zlang_emit_program_pass(program, emitter);
}
