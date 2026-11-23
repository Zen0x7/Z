#include "zlang/compiler/diagnostics.h"
#include "zlang/compiler/emitter.h"
#include "zlang/compiler/parser.h"
#include "zlang/compiler/planner.h"
#include "zlang/compiler/semantics.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ZLANG_CLI_SOURCE_CAPACITY 65536U
#define ZLANG_CLI_OUTPUT_CAPACITY 262144U
#define ZLANG_CLI_PATH_CAPACITY 4096U
#define ZLANG_CLI_COMMAND_CAPACITY 8192U
#define ZLANG_CLI_TOKEN_CAPACITY 512U
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
    (void)fprintf(stderr, "usage: zlang-compile <source.zlang> [--emit-c]\n");
}

static bool zlang_cli_read_source(const char *path, char *buffer, size_t capacity, size_t *length) {
    FILE *const _stream_k = fopen(path, "rb");
    if (_stream_k == NULL) {
        (void)fprintf(stderr, "error: cannot open '%s' for reading\n", path);
        return false;
    }
    const size_t _read_k = fread(buffer, 1U, capacity - 1U, _stream_k);
    const bool _failed_k = ferror(_stream_k) != 0;
    (void)fclose(_stream_k);
    if (_failed_k) {
        (void)fprintf(stderr, "error: cannot read '%s'\n", path);
        return false;
    }
    buffer[_read_k] = '\0';
    *length = _read_k;
    return true;
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
        (void)fprintf(stderr, "%s:%u:%u: error: ZLANG%04u: %s\n", source_path, _item_k->line_,
                      _item_k->column_, (unsigned int)_item_k->code_,
                      _item_k->message_ != NULL ? _item_k->message_ : "diagnostic");
        _reported_k = true;
    }
    return _reported_k;
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

static bool zlang_cli_compile_c(const char *c_path, const char *executable_path) {
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
    bool _built_k = zlang_cli_append(_command, sizeof(_command), &_command_length_k, _compiler_arg);
#if defined(_WIN32)
    _built_k = _built_k &&
               zlang_cli_append(_command, sizeof(_command), &_command_length_k, " /nologo /I \"") &&
               zlang_cli_append(_command, sizeof(_command), &_command_length_k,
                                ZLANG_RUNTIME_INCLUDE_DIR) &&
               zlang_cli_append(_command, sizeof(_command), &_command_length_k, "\" /Fe") &&
               zlang_cli_append(_command, sizeof(_command), &_command_length_k, _executable_arg) &&
               zlang_cli_append(_command, sizeof(_command), &_command_length_k, " ") &&
               zlang_cli_append(_command, sizeof(_command), &_command_length_k, _c_arg);
#else
    _built_k =
        _built_k &&
        zlang_cli_append(_command, sizeof(_command), &_command_length_k, " -std=c99 -O2 -I \"") &&
        zlang_cli_append(_command, sizeof(_command), &_command_length_k,
                         ZLANG_RUNTIME_INCLUDE_DIR) &&
        zlang_cli_append(_command, sizeof(_command), &_command_length_k, "\" ") &&
        zlang_cli_append(_command, sizeof(_command), &_command_length_k, _c_arg) &&
        zlang_cli_append(_command, sizeof(_command), &_command_length_k, " -o ") &&
        zlang_cli_append(_command, sizeof(_command), &_command_length_k, _executable_arg);
#endif
    if (!_built_k) {
        (void)fprintf(stderr, "error: compile command is too long\n");
        return false;
    }
    const int _status_k = system(_command);
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
    static zlang_ast_expression_t _expressions[ZLANG_CLI_EXPRESSION_CAPACITY];
    static zlang_ast_statement_t _statements[ZLANG_CLI_STATEMENT_CAPACITY];
    static zlang_parameter_t _parameters[ZLANG_CLI_PARAMETER_CAPACITY];
    static zlang_symbol_t _symbols[ZLANG_CLI_SYMBOL_CAPACITY];
    static zlang_memory_plan_entry_t _plan_entries[ZLANG_CLI_PLAN_CAPACITY];
    static zlang_diagnostic_t _diagnostics[ZLANG_CLI_DIAGNOSTIC_CAPACITY];

    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        zlang_cli_usage();
        return argc < 2 ? 1 : 0;
    }
    bool _emit_only_k = false;
    if (argc == 3) {
        if (strcmp(argv[2], "--emit-c") != 0) {
            zlang_cli_usage();
            return 1;
        }
        _emit_only_k = true;
    } else if (argc != 2) {
        zlang_cli_usage();
        return 1;
    }

    const char *const _source_path_k = argv[1];
    size_t _source_length_k = 0U;
    if (!zlang_cli_read_source(_source_path_k, _source, sizeof(_source), &_source_length_k)) {
        return 2;
    }
    (void)_source_length_k;

    zlang_diagnostics_t _diagnostic_state;
    zlang_parser_t _parser;
    zlang_parse_result_t _result;
    zlang_semantic_context_t _semantic;
    zlang_memory_plan_t _plan;
    zlang_emitter_t _emitter;
    zlang_diagnostics_init(&_diagnostic_state, _diagnostics,
                           sizeof(_diagnostics) / sizeof(_diagnostics[0]));
    zlang_parser_init(&_parser, _tokens, sizeof(_tokens) / sizeof(_tokens[0]), _expressions,
                      sizeof(_expressions) / sizeof(_expressions[0]), _statements,
                      sizeof(_statements) / sizeof(_statements[0]), _parameters,
                      sizeof(_parameters) / sizeof(_parameters[0]), &_diagnostic_state);

    if (!zlang_parse(&_parser, _source, _scratch, sizeof(_scratch), &_result)) {
        return zlang_cli_report_diagnostics(&_diagnostic_state, _source_path_k) ? 3 : 2;
    }
    zlang_semantic_context_init(&_semantic, _symbols, sizeof(_symbols) / sizeof(_symbols[0]),
                                &_diagnostic_state, &_result.program_);
    if (!zlang_analyze_program(&_result.program_, &_semantic)) {
        return zlang_cli_report_diagnostics(&_diagnostic_state, _source_path_k) ? 4 : 2;
    }

    zlang_memory_plan_init(&_plan, _plan_entries, sizeof(_plan_entries) / sizeof(_plan_entries[0]),
                           ZLANG_DEFAULT_STACK_LIMIT);
    if (!zlang_plan_memory(&_result.program_, &_plan)) {
        return 2;
    }
    zlang_emitter_init(&_emitter, _output, sizeof(_output), &_diagnostic_state, &_plan);
    if (!zlang_emit_program(&_result.program_, &_emitter)) {
        return zlang_cli_report_diagnostics(&_diagnostic_state, _source_path_k) ? 5 : 2;
    }

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
    return zlang_cli_compile_c(_c_path, _executable_path) ? 0 : 6;
}
