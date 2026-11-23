#include "zlang/compiler/emitter.h"
#include "zlang/testing/testing.h"

#include "fixtures.h"

#include <stddef.h>
#include <string.h>

#define ZLANG_TEST_CASE(name) static void name(zlang_test_context_t *const context);
#include "test_cases.h"
#undef ZLANG_TEST_CASE

static void zlang_spec_init(zlang_emitter_t *emitter, char *buffer, size_t buffer_capacity,
                            zlang_diagnostics_t *diagnostics, zlang_diagnostic_t *items,
                            size_t item_capacity) {
    zlang_diagnostics_init(diagnostics, items, item_capacity);
    zlang_emitter_init(emitter, buffer, buffer_capacity, diagnostics);
}

static void emits_a_scalar_declaration(zlang_test_context_t *const context) {
    const zlang_ast_expression_t _initializer_k = {
        .kind_ = zlang_ast_expression_integer_literal_,
        .integer_value_ = 10,
        .string_ = {NULL, 0U},
        .name_ = NULL,
    };
    const zlang_ast_statement_t _statement_k = {
        .kind_ = zlang_ast_statement_declaration_,
        .type_name_ = "integer",
        .name_ = "var",
        .initializer_ = &_initializer_k,
        .capacity_ = 0U,
        .iterable_name_ = NULL,
        .body_ = NULL,
        .body_count_ = 0U,
        .line_ = 1U,
        .column_ = 1U,
    };
    const zlang_program_t _program_k = {&_statement_k, 1U};
    char _output[1024];
    zlang_diagnostic_t _items[4];
    zlang_diagnostics_t _diagnostics;
    zlang_emitter_t _emitter;

    zlang_spec_init(&_emitter, _output, sizeof(_output), &_diagnostics, _items,
                    sizeof(_items) / sizeof(_items[0]));

    ZLANG_CHECK(context, zlang_emit_program(&_program_k, &_emitter));
    ZLANG_CHECK(context, strstr(_output, ZLANG_SPEC_SCALAR_EXPECTED) != NULL);
}

static void emits_a_print_call(zlang_test_context_t *const context) {
    const zlang_ast_expression_t _message_k = {
        .kind_ = zlang_ast_expression_string_literal_,
        .integer_value_ = 0,
        .string_ = {"Hello", 5U},
        .name_ = NULL,
    };
    const zlang_ast_statement_t _statement_k = {
        .kind_ = zlang_ast_statement_print_,
        .type_name_ = NULL,
        .name_ = NULL,
        .initializer_ = &_message_k,
        .capacity_ = 0U,
        .iterable_name_ = NULL,
        .body_ = NULL,
        .body_count_ = 0U,
        .line_ = 1U,
        .column_ = 1U,
    };
    const zlang_program_t _program_k = {&_statement_k, 1U};
    char _output[1024];
    zlang_diagnostic_t _items[4];
    zlang_diagnostics_t _diagnostics;
    zlang_emitter_t _emitter;

    zlang_spec_init(&_emitter, _output, sizeof(_output), &_diagnostics, _items,
                    sizeof(_items) / sizeof(_items[0]));

    ZLANG_CHECK(context, zlang_emit_program(&_program_k, &_emitter));
    ZLANG_CHECK(context, strstr(_output, ZLANG_SPEC_PRINT_EXPECTED) != NULL);
}

static void emits_a_native_foreach_loop(zlang_test_context_t *const context) {
    const zlang_ast_expression_t _message_k = {
        .kind_ = zlang_ast_expression_string_literal_,
        .integer_value_ = 0,
        .string_ = {"Hello", 5U},
        .name_ = NULL,
    };
    const zlang_ast_statement_t _body_k[] = {{
        .kind_ = zlang_ast_statement_print_,
        .type_name_ = NULL,
        .name_ = NULL,
        .initializer_ = &_message_k,
        .capacity_ = 0U,
        .iterable_name_ = NULL,
        .body_ = NULL,
        .body_count_ = 0U,
        .line_ = 5U,
        .column_ = 5U,
    }};
    const zlang_ast_statement_t _statements_k[] = {
        {
            .kind_ = zlang_ast_statement_sequence_declaration_,
            .type_name_ = "integer",
            .name_ = "values",
            .initializer_ = NULL,
            .capacity_ = 4U,
            .iterable_name_ = NULL,
            .body_ = NULL,
            .body_count_ = 0U,
            .line_ = 2U,
            .column_ = 1U,
        },
        {
            .kind_ = zlang_ast_statement_foreach_,
            .type_name_ = "integer",
            .name_ = "item",
            .initializer_ = NULL,
            .capacity_ = 0U,
            .iterable_name_ = "values",
            .body_ = _body_k,
            .body_count_ = 1U,
            .line_ = 4U,
            .column_ = 1U,
        },
    };
    const zlang_program_t _program_k = {_statements_k, 2U};
    char _output[2048];
    zlang_diagnostic_t _items[4];
    zlang_diagnostics_t _diagnostics;
    zlang_emitter_t _emitter;

    zlang_spec_init(&_emitter, _output, sizeof(_output), &_diagnostics, _items,
                    sizeof(_items) / sizeof(_items[0]));

    ZLANG_CHECK(context, zlang_emit_program(&_program_k, &_emitter));
    ZLANG_CHECK(context, strstr(_output, ZLANG_SPEC_FOREACH_SEQUENCE_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_output, ZLANG_SPEC_FOREACH_LOOP_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_output, ZLANG_SPEC_FOREACH_ITEM_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_output, ZLANG_SPEC_PRINT_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_output, ZLANG_SPEC_FOREACH_CALLBACK) == NULL);
}

static void rejects_an_unbounded_sequence_at_the_declaration(zlang_test_context_t *const context) {
    const zlang_ast_statement_t _statement_k = {
        .kind_ = zlang_ast_statement_sequence_declaration_,
        .type_name_ = "integer",
        .name_ = "values",
        .initializer_ = NULL,
        .capacity_ = 0U,
        .iterable_name_ = NULL,
        .body_ = NULL,
        .body_count_ = 0U,
        .line_ = 3U,
        .column_ = 9U,
    };
    const zlang_program_t _program_k = {&_statement_k, 1U};
    char _output[1024];
    zlang_diagnostic_t _items[4];
    zlang_diagnostics_t _diagnostics;
    zlang_emitter_t _emitter;

    zlang_spec_init(&_emitter, _output, sizeof(_output), &_diagnostics, _items,
                    sizeof(_items) / sizeof(_items[0]));

    ZLANG_CHECK(context, !zlang_emit_program(&_program_k, &_emitter));
    ZLANG_CHECK(context, _diagnostics.count_ == 1U);
    ZLANG_CHECK(context,
                zlang_diagnostics_contains(&_diagnostics, ZLANG_SPEC_UNBOUNDED_EXPECTED_CODE));
    ZLANG_CHECK(context, _items[0].line_ == 3U);
    ZLANG_CHECK(context, _items[0].column_ == 9U);
    ZLANG_CHECK(context, strstr(_output, "values") == NULL);
}

#define ZLANG_TEST_CASE(name) {#name, name},
static const zlang_test_case_t cases_k[] = {
#include "test_cases.h"
};
#undef ZLANG_TEST_CASE

int main(const int argc, char **const argv) {
    return zlang_test_main(argc, argv, "zlang.spec", cases_k, sizeof(cases_k) / sizeof(cases_k[0]));
}
