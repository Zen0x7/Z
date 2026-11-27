/** @file
 * @brief Module graph resolution and single-file assembly.
 */

#include "modules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void zlang_modules_diag(zlang_diagnostics_t *diagnostics, uint32_t code,
                               const char *message) {
    (void)zlang_diagnostics_add(diagnostics, (uint16_t)code, 1U, 1U, message);
}

static bool zlang_modules_copy(char *output, size_t capacity, const char *value) {
    if (value == NULL || strlen(value) + 1U > capacity) {
        return false;
    }
    const size_t _length_k = strlen(value);
    memcpy(output, value, _length_k + 1U);
    return true;
}

static char *zlang_modules_trim(char *text) {
    while (*text == ' ' || *text == '\t' || *text == '\r') {
        ++text;
    }
    char *_end_k = text + strlen(text);
    while (_end_k > text && (_end_k[-1] == ' ' || _end_k[-1] == '\t' || _end_k[-1] == '\r')) {
        --_end_k;
    }
    *_end_k = '\0';
    return text;
}

static bool zlang_modules_read_file(const char *path, char *buffer, size_t capacity) {
    FILE *const _stream_k = fopen(path, "rb");
    if (_stream_k == NULL) {
        return false;
    }
    const size_t _read_k = fread(buffer, 1U, capacity - 1U, _stream_k);
    const bool _failed_k = ferror(_stream_k) != 0;
    (void)fclose(_stream_k);
    if (_failed_k) {
        return false;
    }
    buffer[_read_k] = '\0';
    return true;
}

static bool zlang_modules_dirname(const char *path, char *output, size_t capacity) {
    const char *_slash_k = strrchr(path, '/');
    const char *_backslash_k = strrchr(path, '\\');
    const char *_last_k = _slash_k > _backslash_k ? _slash_k : _backslash_k;
    if (_last_k == NULL) {
        return zlang_modules_copy(output, capacity, ".");
    }
    if (_last_k == path) {
        return zlang_modules_copy(output, capacity, "/");
    }
    const size_t _length_k = (size_t)(_last_k - path);
    if (_length_k + 1U > capacity) {
        return false;
    }
    memcpy(output, path, _length_k);
    output[_length_k] = '\0';
    return true;
}

static bool zlang_modules_join(const char *base, const char *part, char *output, size_t capacity) {
    const size_t _base_k = strlen(base);
    const size_t _part_k = strlen(part);
    const bool _separator_k =
        _base_k > 0U && base[_base_k - 1U] != '/' && base[_base_k - 1U] != '\\';
    if (_base_k + (_separator_k ? 1U : 0U) + _part_k + 1U > capacity) {
        return false;
    }
    memcpy(output, base, _base_k);
    size_t _length_k = _base_k;
    if (_separator_k) {
        output[_length_k++] = '/';
    }
    memcpy(output + _length_k, part, _part_k);
    output[_length_k + _part_k] = '\0';
    return true;
}

static bool zlang_modules_normalize(const char *path, char *output, size_t capacity) {
    char _parts_k[64][ZLANG_MODULE_PATH_CAPACITY];
    size_t _count_k = 0U;
    char _buffer_k[ZLANG_MODULE_PATH_CAPACITY];
    if (!zlang_modules_copy(_buffer_k, sizeof(_buffer_k), path)) {
        return false;
    }
    const bool _absolute_k = _buffer_k[0] == '/' || _buffer_k[0] == '\\';
    char *_cursor_k = _buffer_k;
    while (*_cursor_k != '\0') {
        char *_separator_k = strchr(_cursor_k, '/');
        if (_separator_k == NULL) {
            _separator_k = strchr(_cursor_k, '\\');
        }
        if (_separator_k != NULL) {
            *_separator_k = '\0';
        }
        const char *const _part_k = zlang_modules_trim(_cursor_k);
        if (_part_k[0] != '\0' && strcmp(_part_k, ".") != 0) {
            if (strcmp(_part_k, "..") == 0) {
                if (_count_k == 0U) {
                    return false;
                }
                --_count_k;
            } else {
                if (_count_k >= 64U ||
                    !zlang_modules_copy(_parts_k[_count_k], ZLANG_MODULE_PATH_CAPACITY, _part_k)) {
                    return false;
                }
                ++_count_k;
            }
        }
        if (_separator_k == NULL) {
            break;
        }
        _cursor_k = _separator_k + 1;
    }
    output[0] = '\0';
    size_t _length_k = 0U;
    if (_absolute_k) {
        if (capacity < 2U) {
            return false;
        }
        output[_length_k++] = '/';
    }
    for (size_t _index = 0U; _index < _count_k; ++_index) {
        const size_t _part_k = strlen(_parts_k[_index]);
        if (_length_k > 0U && output[_length_k - 1U] != '/') {
            output[_length_k++] = '/';
        }
        if (_length_k + _part_k + 1U > capacity) {
            return false;
        }
        memcpy(output + _length_k, _parts_k[_index], _part_k + 1U);
        _length_k += _part_k;
    }
    return true;
}

static bool zlang_modules_within_root(const zlang_modules_t *modules, const char *path);

static bool zlang_modules_find_native(zlang_modules_t *modules, const char *alias,
                                      zlang_native_t **output) {
    if (modules == NULL || alias == NULL) {
        return false;
    }
    for (size_t _index = 0U; _index < modules->native_count_; ++_index) {
        if (strcmp(modules->natives_[_index].alias_, alias) == 0) {
            if (output != NULL) {
                *output = &modules->natives_[_index];
            }
            return true;
        }
    }
    return false;
}

static bool zlang_modules_unquote(const char *value, char *output, size_t capacity) {
    if (value == NULL || output == NULL || capacity == 0U) {
        return false;
    }
    const size_t _length_k = strlen(value);
    if (_length_k >= 2U && value[0] == '\"' && value[_length_k - 1U] == '\"') {
        if (_length_k - 2U + 1U > capacity) {
            return false;
        }
        memcpy(output, value + 1U, _length_k - 2U);
        output[_length_k - 2U] = '\0';
        return true;
    }
    return zlang_modules_copy(output, capacity, value);
}

static bool zlang_modules_resolve_native_path(const zlang_modules_t *modules, const char *path,
                                              char *output, size_t capacity) {
    if (modules == NULL || path == NULL || path[0] == '\0' || output == NULL || capacity == 0U) {
        return false;
    }
    char _path_k[ZLANG_MODULE_PATH_CAPACITY];
    if (!zlang_modules_unquote(path, _path_k, sizeof(_path_k))) {
        return false;
    }
    char _full_k[ZLANG_MODULE_PATH_CAPACITY];
    if (!zlang_modules_join(modules->root_, _path_k, _full_k, sizeof(_full_k))) {
        return false;
    }
    return zlang_modules_normalize(_full_k, output, capacity) &&
           zlang_modules_within_root(modules, output);
}

static bool zlang_modules_find_config(zlang_modules_t *modules, const char *source_path) {
    char _directory_k[ZLANG_MODULE_PATH_CAPACITY];
    if (!zlang_modules_dirname(source_path, _directory_k, sizeof(_directory_k))) {
        return false;
    }
    char _candidate_k[ZLANG_MODULE_PATH_CAPACITY];
    char _config_k[ZLANG_MODULE_SOURCE_CAPACITY];
    for (;;) {
        if (!zlang_modules_join(_directory_k, "z.yaml", _candidate_k, sizeof(_candidate_k))) {
            return false;
        }
        if (zlang_modules_read_file(_candidate_k, _config_k, sizeof(_config_k))) {
            break;
        }
        if (strcmp(_directory_k, ".") == 0 || strcmp(_directory_k, "/") == 0) {
            return zlang_modules_copy(modules->root_, sizeof(modules->root_), _directory_k);
        }
        if (!zlang_modules_dirname(_directory_k, _candidate_k, sizeof(_candidate_k))) {
            return false;
        }
        memcpy(_directory_k, _candidate_k, strlen(_candidate_k) + 1U);
    }
    if (!zlang_modules_copy(modules->root_, sizeof(modules->root_), _directory_k)) {
        return false;
    }
    char *_line_k = strtok(_config_k, "\n");
    bool _in_aliases_k = false;
    bool _in_native_k = false;
    zlang_native_t *_native_k = NULL;
    char(*_native_paths_k)[ZLANG_MODULE_PATH_CAPACITY] = NULL;
    size_t *_native_path_count_k = NULL;
    while (_line_k != NULL) {
        const bool _indented_k = _line_k[0] == ' ' || _line_k[0] == '\t';
        char *_trimmed_k = zlang_modules_trim(_line_k);
        if (*_trimmed_k == '#') {
            _line_k = strtok(NULL, "\n");
            continue;
        }
        if (strcmp(_trimmed_k, "aliases:") == 0) {
            _in_aliases_k = true;
            _in_native_k = false;
            _native_k = NULL;
            _line_k = strtok(NULL, "\n");
            continue;
        }
        if (strcmp(_trimmed_k, "native:") == 0) {
            _in_native_k = true;
            _in_aliases_k = false;
            _native_k = NULL;
            _line_k = strtok(NULL, "\n");
            continue;
        }
        if (!_indented_k) {
            _in_aliases_k = false;
            _in_native_k = false;
            _native_k = NULL;
        }
        if (_in_aliases_k) {
            char *_separator_k = strchr(_trimmed_k, ':');
            if (_separator_k != NULL && modules->alias_count_ < ZLANG_ALIAS_MAX) {
                *_separator_k = '\0';
                const char *const _name_k = zlang_modules_trim(_trimmed_k);
                const char *const _path_k = zlang_modules_trim(_separator_k + 1);
                if (!zlang_modules_copy(modules->alias_names_[modules->alias_count_],
                                        ZLANG_MODULE_NAME_CAPACITY, _name_k) ||
                    !zlang_modules_copy(modules->alias_paths_[modules->alias_count_],
                                        ZLANG_MODULE_PATH_CAPACITY, _path_k)) {
                    return false;
                }
                ++modules->alias_count_;
            }
            _line_k = strtok(NULL, "\n");
            continue;
        }
        if (!_in_native_k) {
            _line_k = strtok(NULL, "\n");
            continue;
        }
        if (_trimmed_k[0] == '-' && _native_path_count_k != NULL &&
            *_native_path_count_k < ZLANG_NATIVE_PATH_MAX) {
            const char *const _value_k = zlang_modules_trim(_trimmed_k + 1);
            if (!zlang_modules_copy(_native_paths_k[*_native_path_count_k],
                                    ZLANG_MODULE_PATH_CAPACITY, _value_k)) {
                return false;
            }
            ++*_native_path_count_k;
            _line_k = strtok(NULL, "\n");
            continue;
        }
        char *_separator_k = strchr(_trimmed_k, ':');
        if (_separator_k == NULL) {
            _line_k = strtok(NULL, "\n");
            continue;
        }
        *_separator_k = '\0';
        const char *const _key_k = zlang_modules_trim(_trimmed_k);
        const char *const _value_k = zlang_modules_trim(_separator_k + 1);
        if (*_value_k == '\0') {
            if (strcmp(_key_k, "include_dirs") == 0 && _native_k != NULL) {
                _native_paths_k = _native_k->include_dirs_;
                _native_path_count_k = &_native_k->include_dir_count_;
            } else if (strcmp(_key_k, "sources") == 0 && _native_k != NULL) {
                _native_paths_k = _native_k->sources_;
                _native_path_count_k = &_native_k->source_count_;
            } else if (_native_k == NULL && modules->native_count_ < ZLANG_NATIVE_MAX) {
                zlang_native_t *const _entry_k = &modules->natives_[modules->native_count_++];
                memset(_entry_k, 0, sizeof(*_entry_k));
                if (!zlang_modules_copy(_entry_k->alias_, ZLANG_MODULE_NAME_CAPACITY, _key_k)) {
                    return false;
                }
                _native_k = _entry_k;
                _native_paths_k = NULL;
                _native_path_count_k = NULL;
            }
            _line_k = strtok(NULL, "\n");
            continue;
        }
        _native_paths_k = NULL;
        _native_path_count_k = NULL;
        if (strcmp(_key_k, "header") == 0 && _native_k != NULL) {
            if (!zlang_modules_copy(_native_k->header_, ZLANG_MODULE_PATH_CAPACITY, _value_k)) {
                return false;
            }
        } else if (_native_k == NULL && modules->native_count_ < ZLANG_NATIVE_MAX) {
            zlang_native_t *const _entry_k = &modules->natives_[modules->native_count_++];
            memset(_entry_k, 0, sizeof(*_entry_k));
            if (!zlang_modules_copy(_entry_k->alias_, ZLANG_MODULE_NAME_CAPACITY, _key_k)) {
                return false;
            }
            _native_k = _entry_k;
        }
        _line_k = strtok(NULL, "\n");
    }
    return true;
}

static bool zlang_modules_alias(const zlang_modules_t *modules, const char *name, char *output,
                                size_t capacity) {
    for (size_t _index = 0U; _index < modules->alias_count_; ++_index) {
        if (strcmp(modules->alias_names_[_index], name) == 0) {
            return zlang_modules_copy(output, capacity, modules->alias_paths_[_index]);
        }
    }
    return false;
}

static bool zlang_modules_within_root(const zlang_modules_t *modules, const char *path) {
    const size_t _root_k = strlen(modules->root_);
    if (_root_k == 0U || strcmp(modules->root_, ".") == 0) {
        return path[0] != '/' && path[0] != '\\';
    }
    if (strcmp(modules->root_, "/") == 0) {
        return path[0] == '/';
    }
    return strncmp(path, modules->root_, _root_k) == 0 &&
           (path[_root_k] == '/' || path[_root_k] == '\\' || path[_root_k] == '\0');
}

static bool zlang_modules_resolve(const zlang_modules_t *modules, const char *base_path,
                                  const char *specifier, char *output, size_t capacity) {
    if (specifier == NULL) {
        return false;
    }
    char _base_dir_k[ZLANG_MODULE_PATH_CAPACITY];
    if (!zlang_modules_dirname(base_path, _base_dir_k, sizeof(_base_dir_k))) {
        return false;
    }
    char _path_k[ZLANG_MODULE_PATH_CAPACITY];
    if (strncmp(specifier, "@std/", 5U) == 0) {
        return false;
    }
    if (specifier[0] == '@') {
        const char *const _slash_k = strchr(specifier, '/');
        if (_slash_k == NULL) {
            return false;
        }
        char _alias_k[ZLANG_MODULE_NAME_CAPACITY];
        const size_t _alias_length_k = (size_t)(_slash_k - specifier - 1);
        if (_alias_length_k + 1U > sizeof(_alias_k)) {
            return false;
        }
        memcpy(_alias_k, specifier + 1U, _alias_length_k);
        _alias_k[_alias_length_k] = '\0';
        char _alias_path_k[ZLANG_MODULE_PATH_CAPACITY];
        if (!zlang_modules_alias(modules, _alias_k, _alias_path_k, sizeof(_alias_path_k))) {
            return false;
        }
        char _full_k[ZLANG_MODULE_PATH_CAPACITY];
        if (!zlang_modules_join(modules->root_, _alias_path_k, _full_k, sizeof(_full_k)) ||
            !zlang_modules_join(_full_k, _slash_k + 1, _path_k, sizeof(_path_k))) {
            return false;
        }
    } else {
        if (!zlang_modules_join(_base_dir_k, specifier, _path_k, sizeof(_path_k))) {
            return false;
        }
    }
    char _normalized_k[ZLANG_MODULE_PATH_CAPACITY];
    if (!zlang_modules_normalize(_path_k, _normalized_k, sizeof(_normalized_k))) {
        return false;
    }
    if (!zlang_modules_within_root(modules, _normalized_k)) {
        return false;
    }
    const size_t _length_k = strlen(_normalized_k);
    const bool _has_extension_k = _length_k > 2U && _normalized_k[_length_k - 2U] == '.' &&
                                  (_normalized_k[_length_k - 1U] == 'z');
    if (_has_extension_k) {
        return zlang_modules_copy(output, capacity, _normalized_k);
    }
    if (_length_k + 3U > capacity) {
        return false;
    }
    memcpy(output, _normalized_k, _length_k);
    output[_length_k] = '.';
    output[_length_k + 1U] = 'z';
    output[_length_k + 2U] = '\0';
    return true;
}

static bool zlang_modules_identifier_kind(zlang_token_kind_t kind) {
    return kind == zlang_token_integer_ || kind == zlang_token_unsigned_ ||
           kind == zlang_token_boolean_ || kind == zlang_token_string_ ||
           kind == zlang_token_array_ || kind == zlang_token_const_ ||
           kind == zlang_token_function_ || kind == zlang_token_struct_ ||
           kind == zlang_token_enum_;
}

static bool zlang_modules_copy_token_name(const zlang_module_t *module, const zlang_token_t *token,
                                          char *output, size_t capacity) {
    if (token == NULL || token->length_ + 1U > capacity ||
        token->offset_ + token->length_ > module->source_length_) {
        return false;
    }
    memcpy(output, module->source_ + token->offset_, token->length_);
    output[token->length_] = '\0';
    return true;
}

static size_t zlang_modules_find_declaration_index(const zlang_module_t *module, const char *name);

static bool zlang_modules_add_declaration(zlang_module_t *module, const char *name) {
    if (name == NULL || module->declaration_count_ >= ZLANG_MODULE_DECLARATION_CAPACITY) {
        return false;
    }
    for (size_t _index = 0U; _index < module->declaration_count_; ++_index) {
        if (strcmp(module->declaration_names_[_index], name) == 0) {
            return true;
        }
    }
    if (!zlang_modules_copy(module->declaration_names_[module->declaration_count_],
                            ZLANG_MODULE_NAME_CAPACITY, name)) {
        return false;
    }
    ++module->declaration_count_;
    return true;
}

static bool zlang_modules_scan(zlang_module_t *module, zlang_diagnostics_t *diagnostics) {
    size_t _depth_k = 0U;
    for (size_t _index = 0U; _index < module->token_count_; ++_index) {
        const zlang_token_t *const _token_k = &module->tokens_[_index];
        if (_token_k->kind_ == zlang_token_left_brace_) {
            ++_depth_k;
            continue;
        }
        if (_token_k->kind_ == zlang_token_right_brace_) {
            if (_depth_k > 0U) {
                --_depth_k;
            }
            continue;
        }
        if (_depth_k != 0U) {
            continue;
        }
        if (_token_k->kind_ == zlang_token_import_) {
            char _names_k[ZLANG_MODULE_DECLARATION_CAPACITY][ZLANG_MODULE_NAME_CAPACITY];
            size_t _name_count_k = 0U;
            size_t _cursor_k = _index + 1U;
            if (_cursor_k >= module->token_count_ ||
                module->tokens_[_cursor_k].kind_ != zlang_token_left_brace_) {
                return false;
            }
            ++_cursor_k;
            while (_cursor_k < module->token_count_ &&
                   module->tokens_[_cursor_k].kind_ != zlang_token_right_brace_) {
                if (module->tokens_[_cursor_k].kind_ == zlang_token_identifier_ &&
                    _name_count_k < ZLANG_MODULE_DECLARATION_CAPACITY) {
                    const zlang_token_t *const _name_k = &module->tokens_[_cursor_k];
                    if (_name_k->length_ + 1U > ZLANG_MODULE_NAME_CAPACITY ||
                        _name_k->offset_ + _name_k->length_ > module->source_length_) {
                        return false;
                    }
                    memcpy(_names_k[_name_count_k], module->source_ + _name_k->offset_,
                           _name_k->length_);
                    _names_k[_name_count_k][_name_k->length_] = '\0';
                    ++_name_count_k;
                }
                ++_cursor_k;
            }
            if (_cursor_k + 2U >= module->token_count_ ||
                module->tokens_[_cursor_k].kind_ != zlang_token_right_brace_ ||
                module->tokens_[_cursor_k + 1U].kind_ != zlang_token_from_ ||
                module->tokens_[_cursor_k + 2U].kind_ != zlang_token_string_literal_) {
                return false;
            }
            const zlang_token_t *const _domain_k = &module->tokens_[_cursor_k + 2U];
            for (size_t _name = 0U; _name < _name_count_k; ++_name) {
                if (module->import_count_ >= ZLANG_MODULE_DECLARATION_CAPACITY ||
                    _domain_k->value_length_ + 1U > ZLANG_MODULE_NAME_CAPACITY) {
                    return false;
                }
                memcpy(module->import_names_[module->import_count_], _names_k[_name],
                       strlen(_names_k[_name]) + 1U);
                if (_domain_k->offset_ + _domain_k->value_length_ > module->source_length_) {
                    return false;
                }
                memcpy(module->import_domains_[module->import_count_],
                       module->source_ + _domain_k->offset_ + 1U, _domain_k->value_length_);
                module->import_domains_[module->import_count_][_domain_k->value_length_] = '\0';
                ++module->import_count_;
            }
            continue;
        }
        if (_token_k->kind_ == zlang_token_function_ || _token_k->kind_ == zlang_token_struct_ ||
            _token_k->kind_ == zlang_token_enum_) {
            const bool _enum_k = _token_k->kind_ == zlang_token_enum_;
            if (_index + 1U < module->token_count_ &&
                module->tokens_[_index + 1U].kind_ == zlang_token_identifier_ &&
                !zlang_modules_add_declaration(
                    module, (zlang_modules_copy_token_name(module, &module->tokens_[_index + 1U],
                                                           module->decl_name_buffer_,
                                                           ZLANG_MODULE_NAME_CAPACITY)
                                 ? module->decl_name_buffer_
                                 : NULL))) {
                return false;
            }
            const size_t _decl_index_k =
                zlang_modules_find_declaration_index(module, module->decl_name_buffer_);
            if (_decl_index_k < ZLANG_MODULE_DECLARATION_CAPACITY) {
                module->declaration_is_enum_[_decl_index_k] = _enum_k;
            }
            size_t _cursor_k = _index + 1U;
            while (_cursor_k < module->token_count_ &&
                   module->tokens_[_cursor_k].kind_ != zlang_token_left_brace_) {
                ++_cursor_k;
            }
            if (_enum_k && _cursor_k < module->token_count_) {
                ++_cursor_k;
                while (_cursor_k < module->token_count_ &&
                       module->tokens_[_cursor_k].kind_ != zlang_token_right_brace_) {
                    if (module->tokens_[_cursor_k].kind_ == zlang_token_identifier_ &&
                        !zlang_modules_add_declaration(
                            module, (zlang_modules_copy_token_name(
                                         module, &module->tokens_[_cursor_k],
                                         module->decl_name_buffer_, ZLANG_MODULE_NAME_CAPACITY)
                                         ? module->decl_name_buffer_
                                         : NULL))) {
                        return false;
                    }
                    ++_cursor_k;
                }
            }
            if (_cursor_k < module->token_count_) {
                ++_depth_k;
            }
            _index = _cursor_k;
            continue;
        }
        const bool _user_type_declaration_k =
            _token_k->kind_ == zlang_token_identifier_ && _index + 1U < module->token_count_ &&
            module->tokens_[_index + 1U].kind_ == zlang_token_identifier_;
        if (zlang_modules_identifier_kind(_token_k->kind_) || _user_type_declaration_k) {
            size_t _cursor_k = _index;
            if (module->tokens_[_cursor_k].kind_ == zlang_token_const_) {
                ++_cursor_k;
            }
            if (module->tokens_[_cursor_k].kind_ == zlang_token_array_) {
                ++_cursor_k;
                while (_cursor_k < module->token_count_ &&
                       module->tokens_[_cursor_k].kind_ == zlang_token_left_bracket_) {
                    ++_cursor_k;
                    if (_cursor_k < module->token_count_ &&
                        module->tokens_[_cursor_k].kind_ == zlang_token_integer_literal_) {
                        ++_cursor_k;
                    }
                    if (_cursor_k < module->token_count_ &&
                        module->tokens_[_cursor_k].kind_ == zlang_token_right_bracket_) {
                        ++_cursor_k;
                    }
                }
            }
            if (_cursor_k + 1U < module->token_count_ &&
                module->tokens_[_cursor_k + 1U].kind_ == zlang_token_identifier_ &&
                !zlang_modules_add_declaration(
                    module, (zlang_modules_copy_token_name(module, &module->tokens_[_cursor_k + 1U],
                                                           module->decl_name_buffer_,
                                                           ZLANG_MODULE_NAME_CAPACITY)
                                 ? module->decl_name_buffer_
                                 : NULL))) {
                return false;
            }
        }
    }
    (void)diagnostics;
    return true;
}

static bool zlang_modules_canonical(const zlang_module_t *module, const char *name, char *output,
                                    size_t capacity) {
    if (strcmp(module->id_, "root") == 0) {
        return zlang_modules_copy(output, capacity, name);
    }
    const int _written_k = snprintf(output, capacity, "m_%s_%s", module->id_, name);
    return _written_k >= 0 && (size_t)_written_k < capacity;
}

static bool zlang_modules_find_declaration(zlang_modules_t *modules, const char *path,
                                           const char *name, char *output, size_t capacity);

static const zlang_module_t *zlang_modules_find_module(const zlang_modules_t *modules,
                                                       const char *path) {
    if (modules == NULL || path == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < modules->module_count_; ++_index) {
        if (strcmp(modules->modules_[_index].path_, path) == 0) {
            return &modules->modules_[_index];
        }
    }
    return NULL;
}

static size_t zlang_modules_find_declaration_index(const zlang_module_t *module, const char *name) {
    if (module == NULL || name == NULL) {
        return ZLANG_MODULE_DECLARATION_CAPACITY;
    }
    for (size_t _index = 0U; _index < module->declaration_count_; ++_index) {
        if (strcmp(module->declaration_names_[_index], name) == 0) {
            return _index;
        }
    }
    return ZLANG_MODULE_DECLARATION_CAPACITY;
}

static bool zlang_modules_bind(zlang_module_t *module, const char *source_name,
                               const char *canonical_name) {
    if (module->declaration_count_ >= ZLANG_MODULE_DECLARATION_CAPACITY) {
        return false;
    }
    for (size_t _index = 0U; _index < module->declaration_count_; ++_index) {
        if (strcmp(module->declaration_names_[_index], source_name) == 0) {
            return strcmp(module->canonical_names_[_index], canonical_name) == 0;
        }
    }
    if (!zlang_modules_copy(module->declaration_names_[module->declaration_count_],
                            ZLANG_MODULE_NAME_CAPACITY, source_name) ||
        !zlang_modules_copy(module->canonical_names_[module->declaration_count_],
                            ZLANG_MODULE_NAME_CAPACITY, canonical_name)) {
        return false;
    }
    ++module->declaration_count_;
    return true;
}

static bool zlang_modules_bind_enum_items(zlang_modules_t *modules, zlang_module_t *importer,
                                          const char *dependency_path, const char *enum_name,
                                          bool is_enum) {
    if (!is_enum) {
        return true;
    }
    zlang_module_t *_dependency_k = NULL;
    for (size_t _index = 0U; _index < modules->module_count_; ++_index) {
        if (strcmp(modules->modules_[_index].path_, dependency_path) == 0) {
            _dependency_k = &modules->modules_[_index];
            break;
        }
    }
    if (_dependency_k == NULL) {
        return false;
    }
    bool _found_enum_k = false;
    size_t _cursor_k = 0U;
    while (_cursor_k < _dependency_k->token_count_) {
        const zlang_token_t *const _token_k = &_dependency_k->tokens_[_cursor_k];
        if (_token_k->kind_ != zlang_token_enum_ || _cursor_k + 1U >= _dependency_k->token_count_) {
            ++_cursor_k;
            continue;
        }
        char _name_k[ZLANG_MODULE_NAME_CAPACITY];
        if (!zlang_modules_copy_token_name(_dependency_k, &_dependency_k->tokens_[_cursor_k + 1U],
                                           _name_k, sizeof(_name_k))) {
            return false;
        }
        if (strcmp(_name_k, enum_name) != 0) {
            ++_cursor_k;
            continue;
        }
        _found_enum_k = true;
        ++_cursor_k;
        while (_cursor_k < _dependency_k->token_count_ &&
               _dependency_k->tokens_[_cursor_k].kind_ != zlang_token_left_brace_) {
            ++_cursor_k;
        }
        if (_cursor_k >= _dependency_k->token_count_) {
            return false;
        }
        ++_cursor_k;
        while (_cursor_k < _dependency_k->token_count_ &&
               _dependency_k->tokens_[_cursor_k].kind_ != zlang_token_right_brace_) {
            if (_dependency_k->tokens_[_cursor_k].kind_ == zlang_token_identifier_) {
                char _item_name_k[ZLANG_MODULE_NAME_CAPACITY];
                char _item_canonical_k[ZLANG_MODULE_NAME_CAPACITY];
                if (!zlang_modules_copy_token_name(_dependency_k,
                                                   &_dependency_k->tokens_[_cursor_k], _item_name_k,
                                                   sizeof(_item_name_k)) ||
                    !zlang_modules_find_declaration(modules, dependency_path, _item_name_k,
                                                    _item_canonical_k, sizeof(_item_canonical_k)) ||
                    !zlang_modules_bind(importer, _item_name_k, _item_canonical_k)) {
                    return false;
                }
            }
            ++_cursor_k;
        }
        break;
    }
    return _found_enum_k;
}

static bool zlang_modules_find_declaration(zlang_modules_t *modules, const char *path,
                                           const char *name, char *output, size_t capacity) {
    for (size_t _index = 0U; _index < modules->module_count_; ++_index) {
        const zlang_module_t *const _module_k = &modules->modules_[_index];
        if (strcmp(_module_k->path_, path) != 0) {
            continue;
        }
        for (size_t _decl = 0U; _decl < _module_k->declaration_count_; ++_decl) {
            if (strcmp(_module_k->declaration_names_[_decl], name) == 0) {
                return zlang_modules_copy(output, capacity, _module_k->canonical_names_[_decl]);
            }
        }
    }
    return false;
}

static bool zlang_modules_load(zlang_modules_t *modules, const char *path, bool is_root,
                               zlang_diagnostics_t *diagnostics);

static bool zlang_modules_transform(zlang_modules_t *modules, zlang_module_t *module,
                                    zlang_diagnostics_t *diagnostics) {
    size_t _length_k = 0U;
    size_t _cursor_k = 0U;
    for (size_t _index = 0U; _index < module->token_count_; ++_index) {
        const zlang_token_t *const _token_k = &module->tokens_[_index];
        if (_token_k->kind_ == zlang_token_import_) {
            size_t _end_k = _index + 1U;
            while (_end_k < module->token_count_ &&
                   module->tokens_[_end_k].kind_ != zlang_token_semicolon_) {
                ++_end_k;
            }
            if (_end_k >= module->token_count_) {
                return false;
            }
            const size_t _skip_end_k = module->tokens_[_end_k].offset_ + 1U;
            if (_token_k->offset_ < _cursor_k || _skip_end_k > module->source_length_ ||
                _skip_end_k - _token_k->offset_ + _length_k >= ZLANG_MODULE_SOURCE_CAPACITY) {
                return false;
            }
            _cursor_k = _skip_end_k;
            _index = _end_k;
            continue;
        }
        if (_token_k->offset_ < _cursor_k) {
            continue;
        }
        const size_t _gap_k = _token_k->offset_ - _cursor_k;
        if (_gap_k + _length_k >= ZLANG_MODULE_SOURCE_CAPACITY) {
            return false;
        }
        memcpy(module->assembled_ + _length_k, module->source_ + _cursor_k, _gap_k);
        _length_k += _gap_k;
        const char *_replacement_k = NULL;
        if (_token_k->kind_ == zlang_token_native_specifier_) {
            char _alias_k[ZLANG_MODULE_NAME_CAPACITY];
            const char *const _alias_start_k = _token_k->text_ + 8U;
            const size_t _alias_length_k = strlen(_alias_start_k);
            if (_alias_length_k == 0U || _alias_length_k + 1U > sizeof(_alias_k)) {
                zlang_modules_diag(diagnostics, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER,
                                   "invalid @native alias");
                return false;
            }
            memcpy(_alias_k, _alias_start_k, _alias_length_k + 1U);
            zlang_native_t *_native_k = NULL;
            if (!zlang_modules_find_native(modules, _alias_k, &_native_k)) {
                zlang_modules_diag(diagnostics, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER,
                                   "native alias is not declared in z.yaml");
                return false;
            }
            char _header_k[ZLANG_MODULE_PATH_CAPACITY];
            if (!zlang_modules_resolve_native_path(modules, _native_k->header_, _header_k,
                                                   sizeof(_header_k)) ||
                _header_k[0] == '\0') {
                zlang_modules_diag(diagnostics, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER,
                                   "native header is not declared or is outside the project");
                return false;
            }
            _native_k->used_ = true;
            const char *_header_text_k = _header_k;
            if (_header_text_k[0] == '.' && _header_text_k[1] == '/') {
                _header_text_k += 2U;
            }
            const size_t _header_text_length_k = strlen(_header_text_k);
            const size_t _available_k = ZLANG_MODULE_SOURCE_CAPACITY - _length_k;
            if (_header_text_length_k + 3U > _available_k) {
                return false;
            }
            module->assembled_[_length_k++] = '\"';
            memcpy(module->assembled_ + _length_k, _header_text_k, _header_text_length_k);
            _length_k += _header_text_length_k;
            module->assembled_[_length_k++] = '\"';
            if (_length_k >= ZLANG_MODULE_SOURCE_CAPACITY) {
                return false;
            }
            module->assembled_[_length_k++] = ';';
            _cursor_k = _token_k->offset_ + _token_k->length_;
            if (_index + 1U < module->token_count_ &&
                module->tokens_[_index + 1U].kind_ == zlang_token_semicolon_) {
                _cursor_k = module->tokens_[_index + 1U].offset_ + 1U;
                ++_index;
            }
            continue;
        }
        if (_token_k->kind_ == zlang_token_identifier_) {
            for (size_t _decl = 0U; _decl < module->declaration_count_; ++_decl) {
                if (strcmp(module->declaration_names_[_decl], _token_k->text_) == 0) {
                    _replacement_k = module->canonical_names_[_decl];
                    break;
                }
            }
            const bool _after_dot_k =
                _index > 0U && module->tokens_[_index - 1U].kind_ == zlang_token_dot_;
            if (_replacement_k != NULL && !_after_dot_k) {
                const size_t _replacement_length_k = strlen(_replacement_k);
                if (_replacement_length_k + _length_k >= ZLANG_MODULE_SOURCE_CAPACITY) {
                    return false;
                }
                memcpy(module->assembled_ + _length_k, _replacement_k, _replacement_length_k);
                _length_k += _replacement_length_k;
            } else {
                if (_token_k->length_ + _length_k >= ZLANG_MODULE_SOURCE_CAPACITY) {
                    return false;
                }
                memcpy(module->assembled_ + _length_k, module->source_ + _token_k->offset_,
                       _token_k->length_);
                _length_k += _token_k->length_;
            }
        } else {
            const size_t _token_length_k =
                _index + 1U < module->token_count_
                    ? module->tokens_[_index + 1U].offset_ - _token_k->offset_
                    : module->source_length_ - _token_k->offset_;
            if (_token_length_k + _length_k >= ZLANG_MODULE_SOURCE_CAPACITY) {
                return false;
            }
            memcpy(module->assembled_ + _length_k, module->source_ + _token_k->offset_,
                   _token_length_k);
            _length_k += _token_length_k;
            _cursor_k = _token_k->offset_ + _token_length_k;
            continue;
        }
        _cursor_k = _token_k->offset_ + _token_k->length_;
    }
    if (_cursor_k < module->source_length_) {
        const size_t _tail_k = module->source_length_ - _cursor_k;
        if (_tail_k + _length_k >= ZLANG_MODULE_SOURCE_CAPACITY) {
            return false;
        }
        memcpy(module->assembled_ + _length_k, module->source_ + _cursor_k, _tail_k);
        _length_k += _tail_k;
    }
    module->assembled_[_length_k] = '\0';
    (void)diagnostics;
    return true;
}

// NOLINTNEXTLINE(misc-no-recursion): the import graph is a bounded DFS.
static bool zlang_modules_load(zlang_modules_t *modules, const char *path, bool is_root,
                               zlang_diagnostics_t *diagnostics) {
    char _normalized_k[ZLANG_MODULE_PATH_CAPACITY];
    if (!zlang_modules_normalize(path, _normalized_k, sizeof(_normalized_k))) {
        return false;
    }
    for (size_t _index = 0U; _index < modules->module_count_; ++_index) {
        const zlang_module_t *const _module_k = &modules->modules_[_index];
        if (strcmp(_module_k->path_, _normalized_k) == 0) {
            if (_module_k->visiting_) {
                zlang_modules_diag(diagnostics, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT,
                                   "cyclic module import");
                return false;
            }
            return true;
        }
    }
    if (modules->module_count_ >= ZLANG_MODULE_MAX) {
        zlang_modules_diag(diagnostics, ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT,
                           "module capacity exceeded");
        return false;
    }
    zlang_module_t *const _module_k = &modules->modules_[modules->module_count_++];
    memset(_module_k, 0, sizeof(*_module_k));
    _module_k->visiting_ = true;
    if (!zlang_modules_copy(_module_k->path_, sizeof(_module_k->path_), _normalized_k) ||
        !zlang_modules_read_file(_normalized_k, _module_k->source_, sizeof(_module_k->source_))) {
        return false;
    }
    _module_k->source_length_ = strlen(_module_k->source_);
    if (is_root) {
        if (!zlang_modules_copy(_module_k->id_, sizeof(_module_k->id_), "root")) {
            return false;
        }
    } else {
        const char *_relative_k = _normalized_k;
        const size_t _root_length_k = strlen(modules->root_);
        if (strcmp(modules->root_, "/") == 0 && _normalized_k[0] == '/') {
            _relative_k = _normalized_k + 1U;
        } else if (strncmp(_normalized_k, modules->root_, _root_length_k) == 0 &&
                   _normalized_k[_root_length_k] == '/') {
            _relative_k = _normalized_k + _root_length_k + 1U;
        }
        size_t _id_length_k = 0U;
        for (const char *_cursor_k = _relative_k; *_cursor_k != '\0'; ++_cursor_k) {
            char _character_k = *_cursor_k;
            if (*_cursor_k == '/' || *_cursor_k == '\\' || *_cursor_k == '.') {
                _character_k = '_';
            }
            if (_id_length_k + 1U >= sizeof(_module_k->id_)) {
                return false;
            }
            _module_k->id_[_id_length_k++] = _character_k;
        }
        _module_k->id_[_id_length_k] = '\0';
    }
    zlang_comment_t _comments_k[ZLANG_MODULE_COMMENT_CAPACITY];
    zlang_lexer_t _lexer;
    zlang_lexer_init(&_lexer, _module_k->source_, _module_k->scratch_, sizeof(_module_k->scratch_),
                     _comments_k, sizeof(_comments_k) / sizeof(_comments_k[0]));
    if (!zlang_lex(&_lexer, _module_k->tokens_,
                   sizeof(_module_k->tokens_) / sizeof(_module_k->tokens_[0]),
                   &_module_k->token_count_, diagnostics)) {
        zlang_modules_diag(diagnostics, ZLANG_DIAGNOSTIC_UNEXPECTED_TOKEN,
                           "module source has invalid tokens");
        return false;
    }
    if (!zlang_modules_scan(_module_k, diagnostics)) {
        return false;
    }
    for (size_t _decl = 0U; _decl < _module_k->declaration_count_; ++_decl) {
        if (!zlang_modules_canonical(_module_k, _module_k->declaration_names_[_decl],
                                     _module_k->canonical_names_[_decl],
                                     ZLANG_MODULE_NAME_CAPACITY)) {
            return false;
        }
    }
    for (size_t _import = 0U; _import < _module_k->import_count_; ++_import) {
        const char *const _domain_k = _module_k->import_domains_[_import];
        const bool _std_domain_k = strncmp(_domain_k, "@std/", 5U) == 0;
        if (_std_domain_k) {
            bool _exists_k = false;
            for (size_t _std = 0U; _std < modules->std_import_count_; ++_std) {
                if (strcmp(modules->std_import_names_[_std], _module_k->import_names_[_import]) ==
                        0 &&
                    strcmp(modules->std_domains_[_std], _domain_k) == 0) {
                    _exists_k = true;
                    break;
                }
            }
            if (!_exists_k && modules->std_import_count_ < ZLANG_MODULE_DECLARATION_CAPACITY) {
                zlang_modules_copy(modules->std_import_names_[modules->std_import_count_],
                                   ZLANG_MODULE_NAME_CAPACITY, _module_k->import_names_[_import]);
                zlang_modules_copy(modules->std_domains_[modules->std_import_count_],
                                   ZLANG_MODULE_NAME_CAPACITY, _domain_k);
                ++modules->std_import_count_;
            }
            continue;
        }
        char _dependency_k[ZLANG_MODULE_PATH_CAPACITY];
        if (!zlang_modules_resolve(modules, _module_k->path_, _domain_k, _dependency_k,
                                   sizeof(_dependency_k)) ||
            !zlang_modules_load(modules, _dependency_k, false, diagnostics)) {
            zlang_modules_diag(diagnostics, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER,
                               "cannot resolve module import");
            return false;
        }
        char _canonical_k[ZLANG_MODULE_NAME_CAPACITY];
        if (!zlang_modules_find_declaration(modules, _dependency_k,
                                            _module_k->import_names_[_import], _canonical_k,
                                            sizeof(_canonical_k))) {
            zlang_modules_diag(diagnostics, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER,
                               "imported name is not declared by module");
            return false;
        }
        const size_t _existing_k =
            zlang_modules_find_declaration_index(_module_k, _module_k->import_names_[_import]);
        if (_existing_k < ZLANG_MODULE_DECLARATION_CAPACITY &&
            strcmp(_module_k->canonical_names_[_existing_k], _canonical_k) != 0) {
            zlang_modules_diag(diagnostics, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER,
                               "imported name is ambiguous");
            return false;
        }
        if (!zlang_modules_bind(_module_k, _module_k->import_names_[_import], _canonical_k)) {
            return false;
        }
        const zlang_module_t *const _dependency_module_k =
            zlang_modules_find_module(modules, _dependency_k);
        const size_t _import_decl_k = zlang_modules_find_declaration_index(
            _dependency_module_k, _module_k->import_names_[_import]);
        const bool _import_is_enum_k = _import_decl_k < ZLANG_MODULE_DECLARATION_CAPACITY &&
                                       _dependency_module_k->declaration_is_enum_[_import_decl_k];
        const size_t _import_binding_k =
            zlang_modules_find_declaration_index(_module_k, _module_k->import_names_[_import]);
        if (_import_binding_k < ZLANG_MODULE_DECLARATION_CAPACITY) {
            _module_k->declaration_is_enum_[_import_binding_k] = _import_is_enum_k;
        }
        if (!zlang_modules_bind_enum_items(modules, _module_k, _dependency_k,
                                           _module_k->import_names_[_import], _import_is_enum_k)) {
            return false;
        }
    }
    if (!zlang_modules_transform(modules, _module_k, diagnostics)) {
        return false;
    }
    _module_k->visiting_ = false;
    return true;
}

void zlang_modules_init(zlang_modules_t *modules) { memset(modules, 0, sizeof(*modules)); }

size_t zlang_modules_native_count(const zlang_modules_t *modules) {
    return modules != NULL ? modules->native_count_ : 0U;
}

const zlang_native_t *zlang_modules_native_at(const zlang_modules_t *modules, size_t index) {
    return modules != NULL && index < modules->native_count_ ? &modules->natives_[index] : NULL;
}

bool zlang_modules_native_include_dir(const zlang_modules_t *modules, const zlang_native_t *native,
                                      size_t index, char *output, size_t capacity) {
    return native != NULL && index < native->include_dir_count_ &&
           zlang_modules_resolve_native_path(modules, native->include_dirs_[index], output,
                                             capacity);
}

bool zlang_modules_native_source(const zlang_modules_t *modules, const zlang_native_t *native,
                                 size_t index, char *output, size_t capacity) {
    return native != NULL && index < native->source_count_ &&
           zlang_modules_resolve_native_path(modules, native->sources_[index], output, capacity);
}

bool zlang_modules_assemble(zlang_modules_t *modules, const char *source_path,
                            zlang_diagnostics_t *diagnostics, char *output, size_t capacity) {
    if (!zlang_modules_find_config(modules, source_path)) {
        return false;
    }
    if (!zlang_modules_load(modules, source_path, true, diagnostics)) {
        return false;
    }
    output[0] = '\0';
    size_t _length_k = 0U;
    for (size_t _std = 0U; _std < modules->std_import_count_; ++_std) {
        const int _written_k =
            snprintf(output + _length_k, capacity - _length_k, "import { %s } from \"%s\";\n",
                     modules->std_import_names_[_std], modules->std_domains_[_std]);
        if (_written_k < 0 || (size_t)_written_k >= capacity - _length_k) {
            return false;
        }
        _length_k += (size_t)_written_k;
    }
    if (_length_k > 0U && _length_k + 1U >= capacity) {
        return false;
    }
    output[_length_k++] = '\n';
    for (size_t _index = 1U; _index < modules->module_count_; ++_index) {
        const zlang_module_t *const _module_k = &modules->modules_[_index];
        const size_t _source_k = strlen(_module_k->assembled_);
        if (_length_k + _source_k + 1U >= capacity) {
            return false;
        }
        memcpy(output + _length_k, _module_k->assembled_, _source_k);
        _length_k += _source_k;
        output[_length_k++] = '\n';
    }
    if (modules->module_count_ > 0U) {
        const zlang_module_t *const _root_k = &modules->modules_[0];
        const size_t _source_k = strlen(_root_k->assembled_);
        if (_length_k + _source_k + 1U >= capacity) {
            return false;
        }
        memcpy(output + _length_k, _root_k->assembled_, _source_k);
        _length_k += _source_k;
        output[_length_k++] = '\n';
    }
    output[_length_k] = '\0';
    return true;
}
