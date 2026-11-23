#include "zlang/compiler/emitter.h"
#include "zlang/compiler/parser.h"
#include "zlang/compiler/planner.h"
#include "zlang/compiler/semantics.h"
#include "zlang/testing/testing.h"

#include "fixtures.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define ZLANG_TEST_CASE(name) static void name(zlang_test_context_t *const context);
#include "test_cases.h"
#undef ZLANG_TEST_CASE

typedef struct zlang_spec_session_t {
    zlang_token_t tokens_[128];
    zlang_ast_expression_t expressions_[32];
    zlang_ast_statement_t statements_[32];
    zlang_diagnostic_t diagnostic_items_[8];
    zlang_symbol_t symbols_[32];
    zlang_memory_plan_entry_t plan_entries_[32];
    zlang_diagnostics_t diagnostics_;
    zlang_semantic_context_t semantic_;
    zlang_memory_plan_t plan_;
    zlang_parser_t parser_;
    zlang_parse_result_t result_;
    char scratch_[1024];
    char output_[2048];
    zlang_emitter_t emitter_;
} zlang_spec_session_t;

static bool zlang_spec_transpile(zlang_spec_session_t *session, const char *source) {
    session->output_[0] = '\0';
    zlang_diagnostics_init(&session->diagnostics_, session->diagnostic_items_,
                           sizeof(session->diagnostic_items_) /
                               sizeof(session->diagnostic_items_[0]));
    zlang_parser_init(
        &session->parser_, session->tokens_, sizeof(session->tokens_) / sizeof(session->tokens_[0]),
        session->expressions_, sizeof(session->expressions_) / sizeof(session->expressions_[0]),
        session->statements_, sizeof(session->statements_) / sizeof(session->statements_[0]),
        &session->diagnostics_);

    if (!zlang_parse(&session->parser_, source, session->scratch_, sizeof(session->scratch_),
                     &session->result_)) {
        return false;
    }

    zlang_semantic_context_init(&session->semantic_, session->symbols_,
                                sizeof(session->symbols_) / sizeof(session->symbols_[0]),
                                &session->diagnostics_);
    if (!zlang_analyze_program(&session->result_.program_, &session->semantic_)) {
        return false;
    }

    zlang_memory_plan_init(&session->plan_, session->plan_entries_,
                           sizeof(session->plan_entries_) / sizeof(session->plan_entries_[0]),
                           ZLANG_SPEC_TEST_STACK_LIMIT);
    if (!zlang_plan_memory(&session->result_.program_, &session->plan_)) {
        return false;
    }

    zlang_emitter_init(&session->emitter_, session->output_, sizeof(session->output_),
                       &session->diagnostics_, &session->plan_);
    return zlang_emit_program(&session->result_.program_, &session->emitter_);
}

static void
uses_static_storage_for_sequences_over_the_stack_limit(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_STATIC_SEQUENCE_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_STATIC_SEQUENCE_EXPECTED) != NULL);
}

static void keeps_small_sequences_automatic(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_FOREACH_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, "static ") == NULL);
}

static void emits_a_scalar_declaration(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_SCALAR_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_SCALAR_EXPECTED) != NULL);
}

static void emits_a_print_call(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_PRINT_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_PRINT_EXPECTED) != NULL);
}

static void emits_a_native_foreach_loop(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_FOREACH_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_FOREACH_SEQUENCE_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_FOREACH_LOOP_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_FOREACH_ITEM_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_PRINT_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_FOREACH_CALLBACK) == NULL);
}

static void rejects_an_unbounded_sequence_at_the_declaration(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, ZLANG_SPEC_UNBOUNDED_SOURCE));
    ZLANG_CHECK(context, _session.diagnostics_.count_ == 1U);
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_session.diagnostics_,
                                                    ZLANG_SPEC_UNBOUNDED_EXPECTED_CODE));
    ZLANG_CHECK(context, _session.diagnostic_items_[0].line_ == 1U);
    ZLANG_CHECK(context, _session.diagnostic_items_[0].column_ == 1U);
    ZLANG_CHECK(context, strstr(_session.output_, "values") == NULL);
}

static void reports_lexer_and_parser_diagnostics(zlang_test_context_t *const context) {
    zlang_spec_session_t _invalid_character_session;
    zlang_spec_session_t _unsupported_statement_session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_invalid_character_session, "integer var = @;"));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_invalid_character_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_INVALID_CHARACTER));

    ZLANG_CHECK(context,
                !zlang_spec_transpile(&_unsupported_statement_session, "while (true) { }"));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_unsupported_statement_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT));
}

static void skips_comments_and_unescapes_strings(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k =
        "// first line\ninteger var = 10; /* block */\nprint(\"say \\\"hi\\\"\");";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_SCALAR_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "say \\\"hi\\\"") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ", 8U });") != NULL);
}

static void rejects_a_duplicate_declaration(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "integer var = 10; integer var = 20;"));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_DUPLICATE_DECLARATION));
    ZLANG_CHECK(context, _session.output_[0] == '\0');
}

static void rejects_an_undefined_initializer(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "integer var = missing;"));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER));
    ZLANG_CHECK(context, _session.output_[0] == '\0');
}

static void rejects_a_type_mismatch(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "integer var = \"text\";"));
    ZLANG_CHECK(context,
                zlang_diagnostics_contains(&_session.diagnostics_, ZLANG_DIAGNOSTIC_TYPE_MISMATCH));
    ZLANG_CHECK(context, _session.output_[0] == '\0');
}

#define ZLANG_TEST_CASE(name) {#name, name},
static const zlang_test_case_t cases_k[] = {
#include "test_cases.h"
};
#undef ZLANG_TEST_CASE

int main(const int argc, char **const argv) {
    return zlang_test_main(argc, argv, "zlang.spec", cases_k, sizeof(cases_k) / sizeof(cases_k[0]));
}
