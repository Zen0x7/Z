/** @file
 * @brief Browser entry point for the Zlang compiler core.
 *
 * The JavaScript host allocates the input and output buffers with `_malloc`,
 * calls `zlang_wasm_compile`, then reads the C text and diagnostics from linear
 * memory. The driver keeps no mutable state between calls.
 */

#include "zlang/compiler/diagnostics.h"
#include "zlang/compiler/emitter.h"
#include "zlang/compiler/optimizer.h"
#include "zlang/compiler/parser.h"
#include "zlang/compiler/planner.h"
#include "zlang/compiler/semantics.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ZLANG_WASM_SOURCE_CAPACITY 65536U
#define ZLANG_WASM_SCRATCH_CAPACITY 65536U
#define ZLANG_WASM_TOKEN_CAPACITY 1024U
#define ZLANG_WASM_COMMENT_CAPACITY 128U
#define ZLANG_WASM_EXPRESSION_CAPACITY 256U
#define ZLANG_WASM_STATEMENT_CAPACITY 256U
#define ZLANG_WASM_PARAMETER_CAPACITY 128U
#define ZLANG_WASM_SYMBOL_CAPACITY 256U
#define ZLANG_WASM_PLAN_CAPACITY 256U
#define ZLANG_WASM_DIAGNOSTIC_CAPACITY 64U

#define ZLANG_WASM_OK 0
#define ZLANG_WASM_INVALID_ARGUMENT 1
#define ZLANG_WASM_PARSE_FAILED 2
#define ZLANG_WASM_ANALYSIS_FAILED 3
#define ZLANG_WASM_PLAN_FAILED 4
#define ZLANG_WASM_EMIT_FAILED 5

static size_t zlang_wasm_append(char *buffer, const size_t capacity, size_t *length,
                                const char *text) {
    const size_t _text_k = strlen(text);
    if (*length + _text_k >= capacity) {
        return 0U;
    }
    memcpy(buffer + *length, text, _text_k);
    *length += _text_k;
    buffer[*length] = '\0';
    return _text_k;
}

static void zlang_wasm_report(const zlang_diagnostics_t *state, char *buffer,
                              const size_t capacity) {
    size_t _length_k = 0U;
    char _line[256];
    buffer[0] = '\0';
    for (size_t _index_k = 0U; _index_k < state->count_; ++_index_k) {
        const zlang_diagnostic_t *const _item_k = &state->items_[_index_k];
        (void)snprintf(_line, sizeof(_line), "%u:%u: %s[ZLANG%04u]: %s\n", _item_k->line_,
                       _item_k->column_, _item_k->is_warning_ ? "warning" : "error",
                       (unsigned int)_item_k->code_,
                       _item_k->message_ != NULL ? _item_k->message_ : "diagnostic");
        if (zlang_wasm_append(buffer, capacity, &_length_k, _line) == 0U) {
            break;
        }
    }
}

int zlang_wasm_compile(const char *source, const int optimize, char *output,
                       const size_t output_capacity, char *diagnostics,
                       const size_t diagnostics_capacity) {
    if (source == NULL || output == NULL || diagnostics == NULL || output_capacity == 0U ||
        diagnostics_capacity == 0U) {
        return ZLANG_WASM_INVALID_ARGUMENT;
    }

    char _scratch[ZLANG_WASM_SCRATCH_CAPACITY];
    zlang_token_t _tokens[ZLANG_WASM_TOKEN_CAPACITY];
    zlang_comment_t _comments[ZLANG_WASM_COMMENT_CAPACITY];
    zlang_ast_expression_t _expressions[ZLANG_WASM_EXPRESSION_CAPACITY];
    zlang_ast_statement_t _statements[ZLANG_WASM_STATEMENT_CAPACITY];
    zlang_parameter_t _parameters[ZLANG_WASM_PARAMETER_CAPACITY];
    zlang_symbol_t _symbols[ZLANG_WASM_SYMBOL_CAPACITY];
    zlang_memory_plan_entry_t _plan_entries[ZLANG_WASM_PLAN_CAPACITY];
    zlang_diagnostic_t _diagnostics[ZLANG_WASM_DIAGNOSTIC_CAPACITY];
    zlang_optimizer_context_t _optimizer;

    output[0] = '\0';
    diagnostics[0] = '\0';

    zlang_diagnostics_t _diagnostic_state;
    zlang_diagnostics_init(&_diagnostic_state, _diagnostics,
                           sizeof(_diagnostics) / sizeof(_diagnostics[0]));

    zlang_parser_t _parser;
    zlang_parse_result_t _result;
    zlang_parser_init(&_parser, _tokens, sizeof(_tokens) / sizeof(_tokens[0]), _expressions,
                      sizeof(_expressions) / sizeof(_expressions[0]), _statements,
                      sizeof(_statements) / sizeof(_statements[0]), _parameters,
                      sizeof(_parameters) / sizeof(_parameters[0]), _comments,
                      sizeof(_comments) / sizeof(_comments[0]), &_diagnostic_state);

    if (!zlang_parse(&_parser, source, _scratch, sizeof(_scratch), &_result)) {
        zlang_wasm_report(&_diagnostic_state, diagnostics, diagnostics_capacity);
        return ZLANG_WASM_PARSE_FAILED;
    }

    zlang_semantic_context_t _semantic;
    zlang_semantic_context_init(&_semantic, _symbols, sizeof(_symbols) / sizeof(_symbols[0]),
                                &_diagnostic_state, &_result.program_);
    if (!zlang_analyze_program(&_result.program_, &_semantic)) {
        zlang_wasm_report(&_diagnostic_state, diagnostics, diagnostics_capacity);
        return ZLANG_WASM_ANALYSIS_FAILED;
    }

    if (optimize != 0) {
        zlang_optimizer_context_init(&_optimizer);
        if (!zlang_optimize_program(&_result.program_, &_optimizer)) {
            zlang_wasm_report(&_diagnostic_state, diagnostics, diagnostics_capacity);
            return ZLANG_WASM_PLAN_FAILED;
        }
    }

    zlang_memory_plan_t _plan;
    zlang_memory_plan_init(&_plan, _plan_entries, sizeof(_plan_entries) / sizeof(_plan_entries[0]),
                           ZLANG_DEFAULT_STACK_LIMIT);
    if (!zlang_plan_memory(&_result.program_, &_plan)) {
        zlang_wasm_report(&_diagnostic_state, diagnostics, diagnostics_capacity);
        return ZLANG_WASM_PLAN_FAILED;
    }

    zlang_emitter_t _emitter;
    zlang_emitter_init(&_emitter, output, output_capacity, &_diagnostic_state, &_plan);
    if (!zlang_emit_program(&_result.program_, &_emitter)) {
        zlang_wasm_report(&_diagnostic_state, diagnostics, diagnostics_capacity);
        return ZLANG_WASM_EMIT_FAILED;
    }

    zlang_wasm_report(&_diagnostic_state, diagnostics, diagnostics_capacity);
    return ZLANG_WASM_OK;
}

const char *zlang_wasm_version(void) { return "zc-wasm"; }
