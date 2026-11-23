/** @file
 * @brief C99 emitter for the Zlang 0.1 abstract syntax tree.
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

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_expression(const zlang_program_t *program, size_t index,
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
        return zlang_append_text(emitter, _expression_k->name_);
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
        return zlang_append_text(emitter, "(") &&
               zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, " == ") &&
               zlang_emit_expression(program, _expression_k->right_, emitter) &&
               zlang_append_text(emitter, ")");
    case zlang_expression_assignment_:
        return zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, " = ") &&
               zlang_emit_expression(program, _expression_k->right_, emitter);
    case zlang_expression_print_call_:
        return zlang_append_text(emitter, "zlang_print(") &&
               zlang_emit_expression(program, _expression_k->left_, emitter) &&
               zlang_append_text(emitter, ")");
    case zlang_expression_append_call_: {
        const zlang_ast_statement_t *const _sequence_k =
            zlang_find_sequence(emitter, _expression_k->name_);
        if (_sequence_k == NULL) {
            return false;
        }
        if (!zlang_append_text(emitter, "zlang_array_append(") ||
            !zlang_append_text(emitter, _expression_k->name_) ||
            !zlang_append_text(emitter, ".data_, &") ||
            !zlang_append_text(emitter, _expression_k->name_) ||
            !zlang_append_text(emitter, ".length_, ") ||
            !zlang_append_unsigned(emitter, _sequence_k->capacity_) ||
            !zlang_append_text(emitter, "U, sizeof(") ||
            !zlang_append_text(emitter, _expression_k->name_) ||
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
        const char *const _name_k = _left_k->name_;
        const zlang_ast_statement_t *const _sequence_k = zlang_find_sequence(emitter, _name_k);
        if (_sequence_k == NULL) {
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
    default:
        return false;
    }
}

static bool zlang_append_zero_list(zlang_emitter_t *emitter, uint32_t capacity) {
    for (uint32_t _index = 0U; _index < capacity; ++_index) {
        if (_index > 0U && !zlang_append_text(emitter, ", ")) {
            return false;
        }
        if (!zlang_append_text(emitter, "0")) {
            return false;
        }
    }
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_statement(const zlang_program_t *program,
                                 const zlang_ast_statement_t *statement, zlang_emitter_t *emitter,
                                 uint32_t depth);

/* Block statements are bounded by the caller-owned statement array. */
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

/* Control flow nests through the caller-owned statement array. */
// NOLINTNEXTLINE(misc-no-recursion)
static bool zlang_emit_statement(const zlang_program_t *program,
                                 const zlang_ast_statement_t *statement, zlang_emitter_t *emitter,
                                 uint32_t depth) { // NOLINT(misc-no-recursion)
    if (!zlang_append_indent(emitter, depth)) {
        return false;
    }
    switch (statement->kind_) {
    case zlang_statement_declaration_:
        if (zlang_is_static(emitter, statement) && !zlang_append_text(emitter, "static ")) {
            return false;
        }
        if (!zlang_append_text(emitter, zlang_c_type(statement->type_name_)) ||
            !zlang_append_text(emitter, " ") || !zlang_append_text(emitter, statement->name_) ||
            !zlang_append_text(emitter, " = ")) {
            return false;
        }
        if (statement->expression_ != ZLANG_NO_EXPRESSION) {
            if (!zlang_emit_expression(program, statement->expression_, emitter)) {
                return false;
            }
        } else if (!zlang_append_text(emitter, zlang_default_initializer(statement->type_name_))) {
            return false;
        }
        if (!zlang_append_text(emitter, ";\n") || !zlang_append_indent(emitter, depth) ||
            !zlang_append_text(emitter, "(void)") ||
            !zlang_append_text(emitter, statement->name_) || !zlang_append_text(emitter, ";\n")) {
            return false;
        }
        return true;
    case zlang_statement_sequence_declaration_: {
        char _name[64];
        if (statement->capacity_ == 0U || !zlang_type_name(statement, _name, sizeof(_name))) {
            return false;
        }
        const bool _needs_type_k = !zlang_type_was_emitted(emitter, _name);
        if (_needs_type_k &&
            (!zlang_mark_type_emitted(emitter, _name) || !zlang_append_text(emitter, "struct ") ||
             !zlang_append_text(emitter, _name) || !zlang_append_text(emitter, " {\n    ") ||
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
        if (!zlang_append_text(emitter, "struct ") || !zlang_append_text(emitter, _name) ||
            !zlang_append_text(emitter, " ") || !zlang_append_text(emitter, statement->name_) ||
            !zlang_append_text(emitter, " = { { ")) {
            return false;
        }
        if (!zlang_append_zero_list(emitter, statement->capacity_)) {
            return false;
        }
        return zlang_append_text(emitter, " }, 0U };\n");
    }
    case zlang_statement_expression_:
        if (!zlang_emit_expression(program, statement->expression_, emitter)) {
            return false;
        }
        return zlang_append_text(emitter, ";\n");
    case zlang_statement_return_:
        if (statement->expression_ == ZLANG_NO_EXPRESSION) {
            return zlang_append_text(emitter, "return;\n");
        }
        if (!zlang_append_text(emitter, "return ") ||
            !zlang_emit_expression(program, statement->expression_, emitter)) {
            return false;
        }
        return zlang_append_text(emitter, ";\n");
    case zlang_statement_if_:
        if (!zlang_append_text(emitter, "if (") ||
            !zlang_emit_expression(program, statement->expression_, emitter) ||
            !zlang_append_text(emitter, ") {\n") ||
            !zlang_emit_body(program, statement->body_start_, statement->body_count_, emitter,
                             depth + 1U) ||
            !zlang_append_indent(emitter, depth) || !zlang_append_text(emitter, "}")) {
            return false;
        }
        if (statement->else_count_ == 0U) {
            return zlang_append_text(emitter, "\n");
        }
        if (!zlang_append_text(emitter, " else {\n") ||
            !zlang_emit_body(program, statement->else_start_, statement->else_count_, emitter,
                             depth + 1U) ||
            !zlang_append_indent(emitter, depth) || !zlang_append_text(emitter, "}\n")) {
            return false;
        }
        return true;
    case zlang_statement_while_:
        return zlang_append_text(emitter, "while (") &&
               zlang_emit_expression(program, statement->expression_, emitter) &&
               zlang_append_text(emitter, ") {\n") &&
               zlang_emit_body(program, statement->body_start_, statement->body_count_, emitter,
                               depth + 1U) &&
               zlang_append_indent(emitter, depth) && zlang_append_text(emitter, "}\n");
    case zlang_statement_repeat_:
        if (!zlang_append_text(emitter, "for (uint32_t _repeat = 0U; _repeat < ") ||
            !zlang_emit_expression(program, statement->expression_, emitter) ||
            !zlang_append_text(emitter, "; ++_repeat) {\n") ||
            !zlang_emit_body(program, statement->body_start_, statement->body_count_, emitter,
                             depth + 1U) ||
            !zlang_append_indent(emitter, depth) || !zlang_append_text(emitter, "}\n")) {
            return false;
        }
        return true;
    case zlang_statement_foreach_: {
        if (!zlang_append_text(emitter, "for (uint32_t index = 0U; index < ") ||
            !zlang_append_text(emitter, statement->iterable_name_) ||
            !zlang_append_text(emitter, ".length_; ++index) {\n") ||
            !zlang_append_indent(emitter, depth + 1U) ||
            !zlang_append_text(emitter, zlang_c_type(statement->type_name_)) ||
            !zlang_append_text(emitter, " ") || !zlang_append_text(emitter, statement->name_) ||
            !zlang_append_text(emitter, " = ") ||
            !zlang_append_text(emitter, statement->iterable_name_) ||
            !zlang_append_text(emitter, ".data_[index];\n") ||
            !zlang_append_indent(emitter, depth + 1U) || !zlang_append_text(emitter, "(void)") ||
            !zlang_append_text(emitter, statement->name_) || !zlang_append_text(emitter, ";\n") ||
            !zlang_emit_body(program, statement->body_start_, statement->body_count_, emitter,
                             depth + 1U) ||
            !zlang_append_indent(emitter, depth) || !zlang_append_text(emitter, "}\n")) {
            return false;
        }
        return true;
    }
    case zlang_statement_function_: {
        if (!zlang_append_text(emitter, zlang_c_type(statement->type_name_)) ||
            !zlang_append_text(emitter, " ") || !zlang_append_text(emitter, statement->name_) ||
            !zlang_append_text(emitter, "(")) {
            return false;
        }
        for (size_t _index = 0U; _index < statement->parameter_count_; ++_index) {
            const zlang_parameter_t *const _parameter_k =
                &program->parameters_[statement->parameter_start_ + _index];
            if (_index > 0U && !zlang_append_text(emitter, ", ")) {
                return false;
            }
            if (!zlang_append_text(emitter, zlang_c_type(_parameter_k->type_name_)) ||
                !zlang_append_text(emitter, " ") ||
                !zlang_append_text(emitter, _parameter_k->name_)) {
                return false;
            }
        }
        if (!zlang_append_text(emitter, ") {\n") ||
            !zlang_emit_body(program, statement->body_start_, statement->body_count_, emitter,
                             depth + 1U) ||
            !zlang_append_indent(emitter, depth) || !zlang_append_text(emitter, "}\n\n")) {
            return false;
        }
        return true;
    }
    case zlang_statement_block_:
        if (!zlang_append_text(emitter, "{\n") ||
            !zlang_emit_body(program, statement->body_start_, statement->body_count_, emitter,
                             depth + 1U) ||
            !zlang_append_indent(emitter, depth) || !zlang_append_text(emitter, "}\n")) {
            return false;
        }
        return true;
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
