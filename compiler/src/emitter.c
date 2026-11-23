#include "zlang/compiler/emitter.h"

#include <stdio.h>
#include <string.h>

void zlang_emitter_init(zlang_emitter_t *emitter, char *buffer, size_t capacity,
                        zlang_diagnostics_t *diagnostics) {
    emitter->buffer_ = buffer;
    emitter->capacity_ = capacity;
    emitter->length_ = 0U;
    emitter->diagnostics_ = diagnostics;
    emitter->next_index_ = 0U;
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
    if (_written_k < 0 || (size_t)_written_k >= sizeof(_digits)) {
        return false;
    }
    return zlang_append_text(emitter, _digits);
}

static bool zlang_append_unsigned(zlang_emitter_t *emitter, uint32_t value) {
    char _digits[16];
    const int _written_k = snprintf(_digits, sizeof(_digits), "%lu", (unsigned long)value);
    if (_written_k < 0 || (size_t)_written_k >= sizeof(_digits)) {
        return false;
    }
    return zlang_append_text(emitter, _digits);
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

static bool zlang_emit_expression(const zlang_ast_expression_t *expression,
                                  zlang_emitter_t *emitter) {
    switch (expression->kind_) {
    case zlang_ast_expression_integer_literal_:
        if (expression->is_unsigned_) {
            if (!zlang_append_unsigned(emitter, (uint32_t)expression->integer_value_)) {
                return false;
            }
            return zlang_append_text(emitter, "U");
        }
        return zlang_append_integer(emitter, expression->integer_value_);
    case zlang_ast_expression_string_literal_:
        if (!zlang_append_text(emitter, "(zlang_string_t){ \"")) {
            return false;
        }
        if (!zlang_append_text(emitter, expression->string_.data_)) {
            return false;
        }
        if (!zlang_append_text(emitter, "\", ")) {
            return false;
        }
        if (!zlang_append_unsigned(emitter, expression->string_.length_)) {
            return false;
        }
        return zlang_append_text(emitter, "U }");
    case zlang_ast_expression_identifier_:
        return zlang_append_text(emitter, expression->name_);
    default:
        return false;
    }
}

static bool zlang_append_sequence_type(zlang_emitter_t *emitter,
                                       const zlang_ast_statement_t *statement) {
    if (!zlang_append_text(emitter, "struct zlang_sequence_")) {
        return false;
    }
    if (!zlang_append_text(emitter, zlang_type_suffix(statement->type_name_))) {
        return false;
    }
    if (!zlang_append_text(emitter, "_")) {
        return false;
    }
    if (!zlang_append_unsigned(emitter, statement->capacity_)) {
        return false;
    }
    return zlang_append_text(emitter, "_t");
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

static bool zlang_emit_declaration(const zlang_ast_statement_t *statement, zlang_emitter_t *emitter,
                                   uint32_t depth) {
    if (!zlang_append_indent(emitter, depth)) {
        return false;
    }
    if (!zlang_append_text(emitter, zlang_c_type(statement->type_name_))) {
        return false;
    }
    if (!zlang_append_text(emitter, " ")) {
        return false;
    }
    if (!zlang_append_text(emitter, statement->name_)) {
        return false;
    }
    if (statement->initializer_ != NULL) {
        if (!zlang_append_text(emitter, " = ")) {
            return false;
        }
        if (!zlang_emit_expression(statement->initializer_, emitter)) {
            return false;
        }
    }
    return zlang_append_text(emitter, ";\n");
}

static bool zlang_emit_sequence_declaration(const zlang_ast_statement_t *statement,
                                            zlang_emitter_t *emitter, uint32_t depth) {
    /* Semantic analysis rejects ZLANG3001 before emission. */
    if (statement->capacity_ == 0U) {
        return false;
    }

    if (!zlang_append_indent(emitter, depth)) {
        return false;
    }
    if (!zlang_append_sequence_type(emitter, statement)) {
        return false;
    }
    if (!zlang_append_text(emitter, " {\n")) {
        return false;
    }
    if (!zlang_append_indent(emitter, depth + 1U)) {
        return false;
    }
    if (!zlang_append_text(emitter, zlang_c_type(statement->type_name_))) {
        return false;
    }
    if (!zlang_append_text(emitter, " data_[")) {
        return false;
    }
    if (!zlang_append_unsigned(emitter, statement->capacity_)) {
        return false;
    }
    if (!zlang_append_text(emitter, "];\n")) {
        return false;
    }
    if (!zlang_append_indent(emitter, depth + 1U)) {
        return false;
    }
    if (!zlang_append_text(emitter, "uint32_t length_;\n")) {
        return false;
    }
    if (!zlang_append_indent(emitter, depth)) {
        return false;
    }
    if (!zlang_append_text(emitter, "};\n")) {
        return false;
    }
    if (!zlang_append_indent(emitter, depth)) {
        return false;
    }
    if (!zlang_append_sequence_type(emitter, statement)) {
        return false;
    }
    if (!zlang_append_text(emitter, " ")) {
        return false;
    }
    if (!zlang_append_text(emitter, statement->name_)) {
        return false;
    }
    if (!zlang_append_text(emitter, " = { { ")) {
        return false;
    }
    if (!zlang_append_zero_list(emitter, statement->capacity_)) {
        return false;
    }
    return zlang_append_text(emitter, " }, 0U };\n");
}

static bool zlang_emit_print(const zlang_ast_statement_t *statement, zlang_emitter_t *emitter,
                             uint32_t depth) {
    if (!zlang_append_indent(emitter, depth)) {
        return false;
    }
    if (!zlang_append_text(emitter, "zlang_print(")) {
        return false;
    }
    if (!zlang_emit_expression(statement->initializer_, emitter)) {
        return false;
    }
    return zlang_append_text(emitter, ");\n");
}

static bool zlang_emit_foreach(const zlang_ast_statement_t *statement, zlang_emitter_t *emitter,
                               uint32_t depth) {
    if (!zlang_append_indent(emitter, depth)) {
        return false;
    }
    if (!zlang_append_text(emitter, "for (uint32_t index = 0U; index < ")) {
        return false;
    }
    if (!zlang_append_text(emitter, statement->iterable_name_)) {
        return false;
    }
    if (!zlang_append_text(emitter, ".length_; ++index) {\n")) {
        return false;
    }
    if (!zlang_append_indent(emitter, depth + 1U)) {
        return false;
    }
    if (!zlang_append_text(emitter, zlang_c_type(statement->type_name_))) {
        return false;
    }
    if (!zlang_append_text(emitter, " ")) {
        return false;
    }
    if (!zlang_append_text(emitter, statement->name_)) {
        return false;
    }
    if (!zlang_append_text(emitter, " = ")) {
        return false;
    }
    if (!zlang_append_text(emitter, statement->iterable_name_)) {
        return false;
    }
    if (!zlang_append_text(emitter, ".data_[index];\n")) {
        return false;
    }
    if (!zlang_append_indent(emitter, depth + 1U)) {
        return false;
    }
    if (!zlang_append_text(emitter, "(void)")) {
        return false;
    }
    if (!zlang_append_text(emitter, statement->name_)) {
        return false;
    }
    if (!zlang_append_text(emitter, ";\n")) {
        return false;
    }

    for (size_t _index = 0U; _index < statement->body_count_; ++_index) {
        const zlang_ast_statement_t *const _body_k = &statement->body_[_index];
        switch (_body_k->kind_) {
        case zlang_ast_statement_print_:
            if (!zlang_emit_print(_body_k, emitter, depth + 2U)) {
                return false;
            }
            break;
        /* Nested foreach is outside the first emitter slice. Keeping it out
           also keeps the emitter non-recursive. */
        default:
            return false;
        }
    }

    if (!zlang_append_indent(emitter, depth)) {
        return false;
    }
    return zlang_append_text(emitter, "}\n");
}

static bool zlang_emit_prologue(zlang_emitter_t *emitter) {
    return zlang_append_text(emitter, "#include <stdbool.h>\n#include <stdint.h>\n\n"
                                      "#include <zlang/runtime/runtime.h>\n\n");
}

static bool zlang_is_declaration(const zlang_ast_statement_t *statement) {
    return statement->kind_ == zlang_ast_statement_declaration_ ||
           statement->kind_ == zlang_ast_statement_sequence_declaration_;
}

static bool zlang_emit_file_scope(const zlang_ast_statement_t *statement,
                                  zlang_emitter_t *emitter) {
    switch (statement->kind_) {
    case zlang_ast_statement_declaration_:
        return zlang_emit_declaration(statement, emitter, 0U);
    case zlang_ast_statement_sequence_declaration_:
        return zlang_emit_sequence_declaration(statement, emitter, 0U);
    default:
        return false;
    }
}

static bool zlang_emit_main_scope(const zlang_ast_statement_t *statement,
                                  zlang_emitter_t *emitter) {
    switch (statement->kind_) {
    case zlang_ast_statement_print_:
        return zlang_emit_print(statement, emitter, 1U);
    case zlang_ast_statement_foreach_:
        return zlang_emit_foreach(statement, emitter, 1U);
    default:
        return false;
    }
}

bool zlang_emit_program(const zlang_program_t *program, zlang_emitter_t *emitter) {
    if (!zlang_emit_prologue(emitter)) {
        return false;
    }

    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (zlang_is_declaration(_statement_k) && !zlang_emit_file_scope(_statement_k, emitter)) {
            return false;
        }
    }

    if (!zlang_append_text(emitter, "int main(void) {\n")) {
        return false;
    }

    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        if (!zlang_is_declaration(_statement_k) && !zlang_emit_main_scope(_statement_k, emitter)) {
            return false;
        }
    }

    return zlang_append_text(emitter, "    return 0;\n}\n");
}
