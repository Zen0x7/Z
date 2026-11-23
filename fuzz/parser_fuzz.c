#include "fuzz_support.h"

#include <zlang/compiler/diagnostics.h>
#include <zlang/compiler/parser.h>

#define ZLANG_FUZZ_TOKEN_CAPACITY 512U
#define ZLANG_FUZZ_EXPRESSION_CAPACITY 512U
#define ZLANG_FUZZ_STATEMENT_CAPACITY 512U
#define ZLANG_FUZZ_PARAMETER_CAPACITY 64U
#define ZLANG_FUZZ_SCRATCH_CAPACITY 1024U
#define ZLANG_FUZZ_DIAGNOSTIC_CAPACITY 32U

int LLVMFuzzerTestOneInput(const uint8_t *const data, size_t size);

int LLVMFuzzerTestOneInput(const uint8_t *const data, size_t size) {
    char _source_k[ZLANG_FUZZ_SOURCE_CAPACITY];
    char _scratch_k[ZLANG_FUZZ_SCRATCH_CAPACITY];
    zlang_token_t _tokens_k[ZLANG_FUZZ_TOKEN_CAPACITY];
    zlang_ast_expression_t _expressions_k[ZLANG_FUZZ_EXPRESSION_CAPACITY];
    zlang_ast_statement_t _statements_k[ZLANG_FUZZ_STATEMENT_CAPACITY];
    zlang_parameter_t _parameters_k[ZLANG_FUZZ_PARAMETER_CAPACITY];
    zlang_diagnostic_t _diagnostics_k[ZLANG_FUZZ_DIAGNOSTIC_CAPACITY];
    zlang_parser_t _parser_k;
    zlang_diagnostics_t _diagnostic_sink_k;
    zlang_parse_result_t _result_k;

    zlang_fuzz_source(_source_k, sizeof(_source_k), data, size);
    zlang_diagnostics_init(&_diagnostic_sink_k, _diagnostics_k, ZLANG_FUZZ_DIAGNOSTIC_CAPACITY);
    zlang_parser_init(&_parser_k, _tokens_k, ZLANG_FUZZ_TOKEN_CAPACITY, _expressions_k,
                      ZLANG_FUZZ_EXPRESSION_CAPACITY, _statements_k, ZLANG_FUZZ_STATEMENT_CAPACITY,
                      _parameters_k, ZLANG_FUZZ_PARAMETER_CAPACITY, &_diagnostic_sink_k);
    (void)zlang_parse(&_parser_k, _source_k, _scratch_k, sizeof(_scratch_k), &_result_k);
    return 0;
}
