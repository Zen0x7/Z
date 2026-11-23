#include "fuzz_support.h"

#include <zlang/compiler/diagnostics.h>
#include <zlang/compiler/lexer.h>

#define ZLANG_FUZZ_TOKEN_CAPACITY 512U
#define ZLANG_FUZZ_SCRATCH_CAPACITY 1024U
#define ZLANG_FUZZ_DIAGNOSTIC_CAPACITY 32U

int LLVMFuzzerTestOneInput(const uint8_t *const data, size_t size);

int LLVMFuzzerTestOneInput(const uint8_t *const data, size_t size) {
    char _source_k[ZLANG_FUZZ_SOURCE_CAPACITY];
    char _scratch_k[ZLANG_FUZZ_SCRATCH_CAPACITY];
    zlang_token_t _tokens_k[ZLANG_FUZZ_TOKEN_CAPACITY];
    zlang_diagnostic_t _diagnostics_k[ZLANG_FUZZ_DIAGNOSTIC_CAPACITY];
    zlang_lexer_t _lexer_k;
    zlang_diagnostics_t _diagnostic_sink_k;
    size_t _token_count_k = 0U;

    zlang_fuzz_source(_source_k, sizeof(_source_k), data, size);
    zlang_diagnostics_init(&_diagnostic_sink_k, _diagnostics_k, ZLANG_FUZZ_DIAGNOSTIC_CAPACITY);
    zlang_lexer_init(&_lexer_k, _source_k, _scratch_k, sizeof(_scratch_k));
    (void)zlang_lex(&_lexer_k, _tokens_k, ZLANG_FUZZ_TOKEN_CAPACITY, &_token_count_k,
                    &_diagnostic_sink_k);
    return 0;
}
