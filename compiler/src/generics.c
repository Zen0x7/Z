/** @file
 * @brief Shared helpers for compile-time generic type applications.
 */

#include "zlang/compiler/generics.h"

#include <stdio.h>
#include <string.h>

const zlang_generic_application_t *zlang_generic_find_application(const zlang_program_t *program,
                                                                  const char *name) {
    if (program == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->generic_application_count_; ++_index) {
        const zlang_generic_application_t *const _entry_k = &program->generic_applications_[_index];
        if (_entry_k->name_ != NULL && strcmp(_entry_k->name_, name) == 0) {
            return _entry_k;
        }
    }
    return NULL;
}

static bool zlang_generic_append_text(char *output, size_t capacity, size_t *length,
                                      const char *text) {
    const size_t _text_length_k = strlen(text);
    if (*length > capacity || _text_length_k > capacity - *length) {
        return false;
    }
    memcpy(output + *length, text, _text_length_k + 1U);
    *length += _text_length_k;
    return true;
}

bool zlang_generic_parse_name(const char *name, zlang_generic_name_t *parsed) {
    if (name == NULL || parsed == NULL) {
        return false;
    }
    memset(parsed, 0, sizeof(*parsed));
    const char *const _open_k = strchr(name, '<');
    if (_open_k == NULL || name[0] == '\0') {
        return false;
    }
    const size_t _base_length_k = (size_t)(_open_k - name);
    if (_base_length_k >= sizeof(parsed->name_)) {
        return false;
    }
    memcpy(parsed->name_, name, _base_length_k);
    parsed->name_[_base_length_k] = '\0';
    parsed->base_name_ = parsed->name_;
    size_t _offset_k = _base_length_k + 1U;
    const char *_cursor_k = _open_k + 1U;
    while (*_cursor_k != '\0' && *_cursor_k != '>' &&
           parsed->argument_count_ < ZLANG_GENERIC_PARAMETER_CAPACITY) {
        const char *const _start_k = _cursor_k;
        while (*_cursor_k != '\0' && *_cursor_k != ',' && *_cursor_k != '>') {
            ++_cursor_k;
        }
        const size_t _argument_length_k = (size_t)(_cursor_k - _start_k);
        if (_argument_length_k == 0U || _offset_k + _argument_length_k >= sizeof(parsed->name_)) {
            return false;
        }
        memcpy(parsed->name_ + _offset_k, _start_k, _argument_length_k);
        parsed->name_[_offset_k + _argument_length_k] = '\0';
        parsed->arguments_[parsed->argument_count_++] = parsed->name_ + _offset_k;
        _offset_k += _argument_length_k + 1U;
        if (*_cursor_k == ',') {
            ++_cursor_k;
        }
    }
    return *_cursor_k == '>' && parsed->argument_count_ > 0U;
}

bool zlang_generic_build_name(const char *base_name, const char *const *arguments,
                              size_t argument_count, char *output, size_t capacity) {
    if (base_name == NULL || arguments == NULL || output == NULL || capacity == 0U ||
        argument_count == 0U || argument_count > ZLANG_GENERIC_PARAMETER_CAPACITY) {
        return false;
    }
    size_t _length_k = 0U;
    output[0] = '\0';
    if (!zlang_generic_append_text(output, capacity, &_length_k, base_name) ||
        !zlang_generic_append_text(output, capacity, &_length_k, "<")) {
        return false;
    }
    for (size_t _index = 0U; _index < argument_count; ++_index) {
        if (arguments[_index] == NULL) {
            return false;
        }
        if (_index > 0U && !zlang_generic_append_text(output, capacity, &_length_k, ",")) {
            return false;
        }
        if (!zlang_generic_append_text(output, capacity, &_length_k, arguments[_index])) {
            return false;
        }
    }
    return zlang_generic_append_text(output, capacity, &_length_k, ">");
}

const zlang_ast_statement_t *zlang_generic_find_declaration(const zlang_program_t *program,
                                                            const char *base_name,
                                                            bool is_function) {
    if (program == NULL || base_name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < program->statement_count_; ++_index) {
        const zlang_ast_statement_t *const _statement_k = &program->statements_[_index];
        const bool _matches_k = is_function ? _statement_k->kind_ == zlang_statement_function_
                                            : _statement_k->kind_ == zlang_statement_struct_;
        if (_matches_k && _statement_k->is_generic_ && _statement_k->name_ != NULL &&
            strcmp(_statement_k->name_, base_name) == 0) {
            return _statement_k;
        }
    }
    return NULL;
}

bool zlang_generic_parameter_index(const zlang_ast_statement_t *declaration, const char *name,
                                   size_t *index) {
    if (declaration == NULL || name == NULL || !declaration->is_generic_) {
        return false;
    }
    for (size_t _parameter = 0U; _parameter < declaration->generic_parameter_count_; ++_parameter) {
        const char *const _parameter_k = declaration->generic_parameters_[_parameter];
        if (_parameter_k != NULL && strcmp(_parameter_k, name) == 0) {
            if (index != NULL) {
                *index = _parameter;
            }
            return true;
        }
    }
    return false;
}

bool zlang_generic_substitute(const char *type_name, const zlang_ast_statement_t *declaration,
                              const zlang_generic_application_t *application, char *output,
                              size_t capacity) {
    size_t _index_k = 0U;
    if (type_name == NULL || application == NULL || output == NULL || capacity == 0U ||
        !zlang_generic_parameter_index(declaration, type_name, &_index_k) ||
        _index_k >= application->argument_count_) {
        return false;
    }
    const char *const _argument_k = application->arguments_[_index_k];
    const size_t _length_k = strlen(_argument_k);
    return _length_k < capacity && memcpy(output, _argument_k, _length_k + 1U) != NULL;
}

bool zlang_generic_resolve_type(const char *type_name, const zlang_ast_statement_t *declaration,
                                const zlang_generic_application_t *application, char *output,
                                size_t capacity) {
    if (type_name == NULL || output == NULL || capacity == 0U) {
        return false;
    }
    if (application != NULL && declaration != NULL &&
        zlang_generic_substitute(type_name, declaration, application, output, capacity)) {
        return true;
    }
    const size_t _length_k = strlen(type_name);
    return _length_k < capacity && memcpy(output, type_name, _length_k + 1U) != NULL;
}
