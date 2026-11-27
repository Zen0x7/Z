/** @file
 * @brief Command line driver that emits C or builds a native binary.
 */

#include "zlang/compiler/diagnostics.h"
#include "zlang/compiler/emitter.h"
#include "zlang/compiler/optimizer.h"
#include "zlang/compiler/parser.h"
#include "zlang/compiler/planner.h"
#include "zlang/compiler/semantics.h"

#include "modules.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#endif

#define ZLANG_CLI_SOURCE_CAPACITY 65536U
#define ZLANG_CLI_OUTPUT_CAPACITY 262144U
#define ZLANG_CLI_PATH_CAPACITY 4096U
#define ZLANG_CLI_COMMAND_CAPACITY 8192U
#define ZLANG_CLI_TOKEN_CAPACITY 1024U
#define ZLANG_CLI_COMMENT_CAPACITY 128U
#define ZLANG_CLI_EXPRESSION_CAPACITY 256U
#define ZLANG_CLI_STATEMENT_CAPACITY 256U
#define ZLANG_CLI_PARAMETER_CAPACITY 128U
#define ZLANG_CLI_SYMBOL_CAPACITY 256U
#define ZLANG_CLI_DIAGNOSTIC_CAPACITY 64U
#define ZLANG_CLI_PLAN_CAPACITY 256U

#ifndef ZLANG_RUNTIME_INCLUDE_DIR
#define ZLANG_RUNTIME_INCLUDE_DIR "include"
#endif

#if defined(_WIN32)
#define ZLANG_EXECUTABLE_SUFFIX ".exe"
#else
#define ZLANG_EXECUTABLE_SUFFIX ""
#endif

static void zlang_cli_usage(void) {
    (void)fprintf(stderr, "usage: zc <source.z> [--emit-c] [--disable-comments] [--tto] [--timing] "
                          "[--dump]\n");
}

static size_t zlang_cli_stem_length(const char *path) {
    const char *const _slash_k = strrchr(path, '/');
    const char *const _backslash_k = strrchr(path, '\\');
    const char *_base_k = path;
    if (_slash_k != NULL && _base_k < _slash_k) {
        _base_k = _slash_k;
    }
    if (_backslash_k != NULL && _base_k < _backslash_k) {
        _base_k = _backslash_k;
    }
    const char *const _dot_k = strrchr(path, '.');
    if (_dot_k != NULL && _dot_k > _base_k) {
        return (size_t)(_dot_k - path);
    }
    return strlen(path);
}

static bool zlang_cli_append(char *buffer, size_t capacity, size_t *length, const char *text) {
    const size_t _text_k = strlen(text);
    if (*length + _text_k >= capacity) {
        return false;
    }
    memcpy(buffer + *length, text, _text_k);
    *length += _text_k;
    buffer[*length] = '\0';
    return true;
}

static bool zlang_cli_make_output_path(const char *source_path, const char *suffix, char *buffer,
                                       size_t capacity) {
    const size_t _stem_k = zlang_cli_stem_length(source_path);
    const size_t _suffix_k = strlen(suffix);
    if (_stem_k + _suffix_k >= capacity) {
        return false;
    }
    memcpy(buffer, source_path, _stem_k);
    memcpy(buffer + _stem_k, suffix, _suffix_k);
    buffer[_stem_k + _suffix_k] = '\0';
    return true;
}

static bool zlang_cli_write_output(const char *path, const char *content) {
    FILE *const _stream_k = fopen(path, "wb");
    if (_stream_k == NULL) {
        (void)fprintf(stderr, "error: cannot open '%s' for writing\n", path);
        return false;
    }
    const bool _written_k = fputs(content, _stream_k) != EOF;
    const bool _closed_k = fclose(_stream_k) == 0;
    if (!_written_k || !_closed_k) {
        (void)fprintf(stderr, "error: cannot write '%s'\n", path);
        return false;
    }
    return true;
}

static bool zlang_cli_report_diagnostics(const zlang_diagnostics_t *diagnostics,
                                         const char *source_path) {
    bool _reported_k = false;
    for (size_t _index = 0U; _index < diagnostics->count_; ++_index) {
        const zlang_diagnostic_t *const _item_k = &diagnostics->items_[_index];
        (void)fprintf(stderr, "%s:%u:%u: %s[ZLANG%04u]: %s\n", source_path, _item_k->line_,
                      _item_k->column_, _item_k->is_warning_ ? "warning" : "error",
                      (unsigned int)_item_k->code_,
                      _item_k->message_ != NULL ? _item_k->message_ : "diagnostic");
        if (!_item_k->is_warning_) {
            _reported_k = true;
        }
    }
    return _reported_k;
}

static bool zlang_cli_equals_ignore_case(const char *left, const char *right) {
    for (;; ++left, ++right) {
        unsigned char _left_k = (unsigned char)*left;
        unsigned char _right_k = (unsigned char)*right;
        if (_left_k >= (unsigned char)'A' && _left_k <= (unsigned char)'Z') {
            _left_k = (unsigned char)(_left_k - (unsigned char)'A' + (unsigned char)'a');
        }
        if (_right_k >= (unsigned char)'A' && _right_k <= (unsigned char)'Z') {
            _right_k = (unsigned char)(_right_k - (unsigned char)'A' + (unsigned char)'a');
        }
        if (_left_k != _right_k) {
            return false;
        }
        if (_left_k == '\0') {
            return true;
        }
    }
}

static const char *zlang_cli_base_name(const char *path) {
    const char *_base_k = path;
    for (const char *_cursor_k = path; *_cursor_k != '\0'; ++_cursor_k) {
        if (*_cursor_k == '/' || *_cursor_k == '\\') {
            _base_k = _cursor_k + 1;
        }
    }
    return _base_k;
}

static bool zlang_cli_compiler_uses_msvc_style(const char *compiler) {
    const char *const _style_k = getenv("ZLANG_CC_STYLE");
    if (_style_k != NULL && _style_k[0] != '\0') {
        return zlang_cli_equals_ignore_case(_style_k, "MSVC");
    }
    const char *const _base_k = zlang_cli_base_name(compiler);
    return zlang_cli_equals_ignore_case(_base_k, "cl") ||
           zlang_cli_equals_ignore_case(_base_k, "cl.exe") ||
           zlang_cli_equals_ignore_case(_base_k, "clang-cl") ||
           zlang_cli_equals_ignore_case(_base_k, "clang-cl.exe");
}

static bool zlang_cli_quote(const char *path, char *buffer, size_t capacity) {
    const size_t _length_k = strlen(path);
    if (strchr(path, '"') != NULL || _length_k + 3U > capacity) {
        return false;
    }
    buffer[0] = '"';
    memcpy(buffer + 1U, path, _length_k);
    buffer[_length_k + 1U] = '"';
    buffer[_length_k + 2U] = '\0';
    return true;
}

#if defined(_WIN32)
static int zlang_cli_run_compiler(char *command) {
    STARTUPINFOA _startup_info;
    PROCESS_INFORMATION _process_info;
    memset(&_startup_info, 0, sizeof(_startup_info));
    memset(&_process_info, 0, sizeof(_process_info));
    _startup_info.cb = (DWORD)sizeof(_startup_info);
    if (!CreateProcessA(NULL, command, NULL, NULL, FALSE, 0U, NULL, NULL, &_startup_info,
                        &_process_info)) {
        (void)fprintf(stderr, "error: cannot start C compiler (%lu)\n",
                      (unsigned long)GetLastError());
        return -1;
    }
    if (WaitForSingleObject(_process_info.hProcess, INFINITE) != WAIT_OBJECT_0) {
        (void)CloseHandle(_process_info.hThread);
        (void)CloseHandle(_process_info.hProcess);
        (void)fprintf(stderr, "error: cannot wait for C compiler (%lu)\n",
                      (unsigned long)GetLastError());
        return -1;
    }
    DWORD _exit_code_k = 0U;
    const bool _exit_read_k = GetExitCodeProcess(_process_info.hProcess, &_exit_code_k) != FALSE;
    (void)CloseHandle(_process_info.hThread);
    (void)CloseHandle(_process_info.hProcess);
    if (!_exit_read_k) {
        (void)fprintf(stderr, "error: cannot read C compiler status (%lu)\n",
                      (unsigned long)GetLastError());
        return -1;
    }
    return _exit_code_k == 0U ? 0 : 1;
}
#else
// cppcheck-suppress constParameterPointer
static int zlang_cli_run_compiler(char *command) { return system(command); }
#endif

static bool zlang_cli_add_unique(char seen[16][ZLANG_MODULE_PATH_CAPACITY], size_t *count,
                                 const char *value) {
    if (value == NULL || *count >= 16U || strlen(value) + 1U > ZLANG_MODULE_PATH_CAPACITY) {
        return false;
    }
    for (size_t _index = 0U; _index < *count; ++_index) {
        if (strcmp(seen[_index], value) == 0) {
            return true;
        }
    }
    memcpy(seen[*count], value, strlen(value) + 1U);
    ++*count;
    return true;
}

static bool zlang_cli_add_native_flags(char *command, size_t capacity, size_t *length,
                                       const zlang_modules_t *modules, bool msvc) {
    char _path_k[ZLANG_CLI_PATH_CAPACITY];
    char _quoted_k[ZLANG_CLI_PATH_CAPACITY + 2U];
    char _seen_includes[16][ZLANG_MODULE_PATH_CAPACITY];
    char _seen_sources[16][ZLANG_MODULE_PATH_CAPACITY];
    char _seen_libraries[16][ZLANG_MODULE_PATH_CAPACITY];
    char _seen_flags[16][ZLANG_MODULE_PATH_CAPACITY];
    size_t _include_count_k = 0U;
    size_t _source_count_k = 0U;
    size_t _library_count_k = 0U;
    size_t _flag_count_k = 0U;
    memset(_seen_includes, 0, sizeof(_seen_includes));
    memset(_seen_sources, 0, sizeof(_seen_sources));
    memset(_seen_libraries, 0, sizeof(_seen_libraries));
    memset(_seen_flags, 0, sizeof(_seen_flags));
    for (size_t _native = 0U; _native < zlang_modules_native_count(modules); ++_native) {
        const zlang_native_t *const _entry_k = zlang_modules_native_at(modules, _native);
        if (_entry_k == NULL || !_entry_k->used_) {
            continue;
        }
        for (size_t _include = 0U; _include < _entry_k->include_dir_count_; ++_include) {
            if (!zlang_modules_native_include_dir(modules, _entry_k, _include, _path_k,
                                                  sizeof(_path_k)) ||
                !zlang_cli_add_unique(_seen_includes, &_include_count_k, _path_k) ||
                !zlang_cli_quote(_path_k, _quoted_k, sizeof(_quoted_k)) ||
                !zlang_cli_append(command, capacity, length, msvc ? " /I " : " -I ") ||
                !zlang_cli_append(command, capacity, length, _quoted_k)) {
                return false;
            }
        }
        for (size_t _source = 0U; _source < _entry_k->source_count_; ++_source) {
            if (!zlang_modules_native_source(modules, _entry_k, _source, _path_k,
                                             sizeof(_path_k)) ||
                !zlang_cli_add_unique(_seen_sources, &_source_count_k, _path_k) ||
                !zlang_cli_quote(_path_k, _quoted_k, sizeof(_quoted_k)) ||
                !zlang_cli_append(command, capacity, length, " ") ||
                !zlang_cli_append(command, capacity, length, _quoted_k)) {
                return false;
            }
        }
        for (size_t _library = 0U; _library < _entry_k->library_count_; ++_library) {
            if (!zlang_modules_native_library(_entry_k, _library, _path_k, sizeof(_path_k)) ||
                !zlang_cli_add_unique(_seen_libraries, &_library_count_k, _path_k) ||
                !zlang_cli_append(command, capacity, length, msvc ? " " : " -l") ||
                !zlang_cli_append(command, capacity, length, _path_k)) {
                return false;
            }
        }
        for (size_t _flag = 0U; _flag < _entry_k->link_flag_count_; ++_flag) {
            if (!zlang_modules_native_link_flag(_entry_k, _flag, _path_k, sizeof(_path_k)) ||
                !zlang_cli_add_unique(_seen_flags, &_flag_count_k, _path_k) ||
                !zlang_cli_append(command, capacity, length, " ") ||
                !zlang_cli_append(command, capacity, length, _path_k)) {
                return false;
            }
        }
    }
    return true;
}

static bool zlang_cli_compile_c(const char *c_path, const char *executable_path,
                                const zlang_modules_t *modules) {
    const char *_compiler_k = getenv("ZLANG_CC");
    if (_compiler_k == NULL || _compiler_k[0] == '\0') {
#if defined(_WIN32)
        _compiler_k = "cl";
#else
        _compiler_k = "cc";
#endif
    }
    char _compiler_arg[ZLANG_CLI_PATH_CAPACITY];
    char _c_arg[ZLANG_CLI_PATH_CAPACITY];
    char _executable_arg[ZLANG_CLI_PATH_CAPACITY];
    char _command[ZLANG_CLI_COMMAND_CAPACITY];
    if (!zlang_cli_quote(_compiler_k, _compiler_arg, sizeof(_compiler_arg)) ||
        !zlang_cli_quote(c_path, _c_arg, sizeof(_c_arg)) ||
        !zlang_cli_quote(executable_path, _executable_arg, sizeof(_executable_arg))) {
        (void)fprintf(stderr, "error: a path contains an unsupported character\n");
        return false;
    }
    size_t _command_length_k = 0U;
    _command[0] = '\0';
    const bool _msvc_k = zlang_cli_compiler_uses_msvc_style(_compiler_k);
    bool _built_k = zlang_cli_append(_command, sizeof(_command), &_command_length_k, _compiler_arg);
    if (_msvc_k) {
        _built_k =
            _built_k &&
            zlang_cli_append(_command, sizeof(_command), &_command_length_k, " /nologo /I \"") &&
            zlang_cli_append(_command, sizeof(_command), &_command_length_k,
                             ZLANG_RUNTIME_INCLUDE_DIR) &&
            zlang_cli_append(_command, sizeof(_command), &_command_length_k, "\" /Fe") &&
            zlang_cli_append(_command, sizeof(_command), &_command_length_k, _executable_arg) &&
            zlang_cli_append(_command, sizeof(_command), &_command_length_k, " ") &&
            zlang_cli_append(_command, sizeof(_command), &_command_length_k, _c_arg);
    } else {
        _built_k =
            _built_k &&
            zlang_cli_append(_command, sizeof(_command), &_command_length_k,
                             " -std=c99 -O2 -I \"") &&
            zlang_cli_append(_command, sizeof(_command), &_command_length_k,
                             ZLANG_RUNTIME_INCLUDE_DIR) &&
            zlang_cli_append(_command, sizeof(_command), &_command_length_k, "\" ") &&
            zlang_cli_append(_command, sizeof(_command), &_command_length_k, _c_arg) &&
            zlang_cli_append(_command, sizeof(_command), &_command_length_k, " -o ") &&
            zlang_cli_append(_command, sizeof(_command), &_command_length_k, _executable_arg);
    }
    if (!_built_k || !zlang_cli_add_native_flags(_command, sizeof(_command), &_command_length_k,
                                                 modules, _msvc_k)) {
        (void)fprintf(stderr, "error: compile command is too long\n");
        return false;
    }
    const int _status_k = zlang_cli_run_compiler(_command);
    if (_status_k != 0) {
        (void)fprintf(stderr, "error: C compiler failed with status %d\n", _status_k);
        return false;
    }
    return true;
}

int main(int argc, char **argv) {
    static char _source[ZLANG_CLI_SOURCE_CAPACITY];
    static char _scratch[ZLANG_CLI_SOURCE_CAPACITY];
    static char _output[ZLANG_CLI_OUTPUT_CAPACITY];
    static char _c_path[ZLANG_CLI_PATH_CAPACITY];
    static char _executable_path[ZLANG_CLI_PATH_CAPACITY];
    static zlang_token_t _tokens[ZLANG_CLI_TOKEN_CAPACITY];
    static zlang_comment_t _comments[ZLANG_CLI_COMMENT_CAPACITY];
    static zlang_ast_expression_t _expressions[ZLANG_CLI_EXPRESSION_CAPACITY];
    static zlang_ast_statement_t _statements[ZLANG_CLI_STATEMENT_CAPACITY];
    static zlang_parameter_t _parameters[ZLANG_CLI_PARAMETER_CAPACITY];
    static zlang_symbol_t _symbols[ZLANG_CLI_SYMBOL_CAPACITY];
    static zlang_memory_plan_entry_t _plan_entries[ZLANG_CLI_PLAN_CAPACITY];
    static zlang_diagnostic_t _diagnostics[ZLANG_CLI_DIAGNOSTIC_CAPACITY];
    static zlang_optimizer_context_t _optimizer;
    static zlang_modules_t _modules;

    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        zlang_cli_usage();
        return argc < 2 ? 1 : 0;
    }
    bool _emit_only_k = false;
    bool _disable_comments_k = false;
    bool _tto_k = false;
    bool _timing_k = false;
    bool _dump_k = false;
    for (int _argument = 2; _argument < argc; ++_argument) {
        if (strcmp(argv[_argument], "--emit-c") == 0) {
            _emit_only_k = true;
        } else if (strcmp(argv[_argument], "--disable-comments") == 0) {
            _disable_comments_k = true;
        } else if (strcmp(argv[_argument], "--tto") == 0) {
            _tto_k = true;
        } else if (strcmp(argv[_argument], "--timing") == 0) {
            _timing_k = true;
        } else if (strcmp(argv[_argument], "--dump") == 0) {
            _dump_k = true;
        } else {
            zlang_cli_usage();
            return 1;
        }
    }

    const char *const _source_path_k = argv[1];
    zlang_diagnostics_t _diagnostic_state;
    zlang_diagnostics_init(&_diagnostic_state, _diagnostics,
                           sizeof(_diagnostics) / sizeof(_diagnostics[0]));
    zlang_modules_init(&_modules);
    if (!zlang_modules_assemble(&_modules, _source_path_k, &_diagnostic_state, _source,
                                sizeof(_source))) {
        return zlang_cli_report_diagnostics(&_diagnostic_state, _source_path_k) ? 3 : 2;
    }
    zlang_parser_t _parser;
    zlang_parse_result_t _result;
    zlang_semantic_context_t _semantic;
    zlang_memory_plan_t _plan;
    zlang_emitter_t _emitter;
    zlang_parser_init(&_parser, _tokens, sizeof(_tokens) / sizeof(_tokens[0]), _expressions,
                      sizeof(_expressions) / sizeof(_expressions[0]), _statements,
                      sizeof(_statements) / sizeof(_statements[0]), _parameters,
                      sizeof(_parameters) / sizeof(_parameters[0]), _comments,
                      sizeof(_comments) / sizeof(_comments[0]), &_diagnostic_state);

    if (!zlang_parse(&_parser, _source, _scratch, sizeof(_scratch), &_result)) {
        return zlang_cli_report_diagnostics(&_diagnostic_state, _source_path_k) ? 3 : 2;
    }
    zlang_semantic_context_init(&_semantic, _symbols, sizeof(_symbols) / sizeof(_symbols[0]),
                                &_diagnostic_state, &_result.program_);
    if (!zlang_analyze_program(&_result.program_, &_semantic)) {
        return zlang_cli_report_diagnostics(&_diagnostic_state, _source_path_k) ? 4 : 2;
    }

    if (_tto_k) {
        zlang_optimizer_context_init(&_optimizer);
        if (!zlang_optimize_program(&_result.program_, &_optimizer)) {
            return 2;
        }
    }

    zlang_memory_plan_init(&_plan, _plan_entries, sizeof(_plan_entries) / sizeof(_plan_entries[0]),
                           ZLANG_DEFAULT_STACK_LIMIT);
    if (!zlang_plan_memory(&_result.program_, &_plan)) {
        return 2;
    }
    zlang_emitter_init(&_emitter, _output, sizeof(_output), &_diagnostic_state, &_plan);
    _emitter.emit_comments_ = !_disable_comments_k;
    _emitter.force_timing_ = _timing_k;
    _emitter.force_dump_ = _dump_k;
    if (!zlang_emit_program(&_result.program_, &_emitter)) {
        return zlang_cli_report_diagnostics(&_diagnostic_state, _source_path_k) ? 5 : 2;
    }
    (void)zlang_cli_report_diagnostics(&_diagnostic_state, _source_path_k);

    if (!zlang_cli_make_output_path(_source_path_k, ".c", _c_path, sizeof(_c_path))) {
        (void)fprintf(stderr, "error: output path is too long\n");
        return 2;
    }
    if (!zlang_cli_write_output(_c_path, _output)) {
        return 2;
    }
    if (_emit_only_k) {
        return 0;
    }
    if (!zlang_cli_make_output_path(_source_path_k, ZLANG_EXECUTABLE_SUFFIX, _executable_path,
                                    sizeof(_executable_path))) {
        (void)fprintf(stderr, "error: executable path is too long\n");
        return 2;
    }
    return zlang_cli_compile_c(_c_path, _executable_path, &_modules) ? 0 : 6;
}
