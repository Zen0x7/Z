#include "zlang/compiler/diagnostics.h"
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
    zlang_ast_expression_t expressions_[64];
    zlang_ast_statement_t statements_[64];
    zlang_parameter_t parameters_[32];
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

static const zlang_diagnostic_t *zlang_spec_find_diagnostic(const zlang_spec_session_t *session,
                                                            uint16_t code) {
    for (size_t _index = 0U; _index < session->diagnostics_.count_; ++_index) {
        if (session->diagnostic_items_[_index].code_ == code) {
            return &session->diagnostic_items_[_index];
        }
    }
    return NULL;
}

static size_t zlang_spec_count_occurrences(const char *text, const char *needle) {
    size_t _count_k = 0U;
    const char *const _needle_k = needle;
    while (text != NULL && *text != '\0' && _needle_k[0] != '\0') {
        const char *const _found_k = strstr(text, _needle_k);
        if (_found_k == NULL) {
            break;
        }
        ++_count_k;
        text = _found_k + strlen(_needle_k);
    }
    return _count_k;
}

static void zlang_spec_expect_diagnostic(zlang_test_context_t *context,
                                         const zlang_spec_session_t *session, uint16_t code,
                                         uint32_t line, uint32_t column) {
    const zlang_diagnostic_t *const _item_k = zlang_spec_find_diagnostic(session, code);

    ZLANG_CHECK(context, _item_k != NULL);
    ZLANG_CHECK(context, _item_k->line_ == line);
    ZLANG_CHECK(context, _item_k->column_ == column);
}

static void zlang_spec_expect_warning(zlang_test_context_t *context,
                                      const zlang_spec_session_t *session, uint16_t code,
                                      uint32_t line, uint32_t column) {
    const zlang_diagnostic_t *const _item_k = zlang_spec_find_diagnostic(session, code);

    ZLANG_CHECK(context, _item_k != NULL);
    ZLANG_CHECK(context, _item_k->is_warning_);
    ZLANG_CHECK(context, _item_k->line_ == line);
    ZLANG_CHECK(context, _item_k->column_ == column);
}

static bool zlang_spec_transpile(zlang_spec_session_t *session, const char *source) {
    session->output_[0] = '\0';
    zlang_diagnostics_init(&session->diagnostics_, session->diagnostic_items_,
                           sizeof(session->diagnostic_items_) /
                               sizeof(session->diagnostic_items_[0]));
    zlang_parser_init(
        &session->parser_, session->tokens_, sizeof(session->tokens_) / sizeof(session->tokens_[0]),
        session->expressions_, sizeof(session->expressions_) / sizeof(session->expressions_[0]),
        session->statements_, sizeof(session->statements_) / sizeof(session->statements_[0]),
        session->parameters_, sizeof(session->parameters_) / sizeof(session->parameters_[0]),
        &session->diagnostics_);

    if (!zlang_parse(&session->parser_, source, session->scratch_, sizeof(session->scratch_),
                     &session->result_)) {
        return false;
    }

    zlang_semantic_context_init(&session->semantic_, session->symbols_,
                                sizeof(session->symbols_) / sizeof(session->symbols_[0]),
                                &session->diagnostics_, &session->result_.program_);
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

static void reports_lexer_diagnostics_and_emits_while(zlang_test_context_t *const context) {
    zlang_spec_session_t _invalid_character_session;
    zlang_spec_session_t _while_session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_invalid_character_session, "integer var = @;"));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_invalid_character_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_INVALID_CHARACTER));

    ZLANG_CHECK(context, zlang_spec_transpile(&_while_session, "while (true) { }"));
    ZLANG_CHECK(context, strstr(_while_session.output_, "while (true) {") != NULL);
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

static void emits_checked_addition(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_ADD_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_ADD_EXPECTED) != NULL);
}

static void emits_if_and_else(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_IF_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_IF_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_IF_ELSE_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "\"yes\"") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "\"no\"") != NULL);
}

static void emits_repeat_as_native_for(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_REPEAT_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_REPEAT_EXPECTED) != NULL);
}

static void emits_function_and_return(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_FUNCTION_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_FUNCTION_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_FUNCTION_RETURN_EXPECTED) != NULL);
}

static void covers_every_annex_a_production(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_CONFORMANCE_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_CONFORMANCE_INTEGER_EXPECTED) != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, ZLANG_SPEC_CONFORMANCE_UNSIGNED_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_CONFORMANCE_BOOLEAN_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_CONFORMANCE_STRING_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_CONFORMANCE_BLOCK_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_CONFORMANCE_IF_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_CONFORMANCE_WHILE_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_CONFORMANCE_FOREACH_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_CONFORMANCE_REPEAT_EXPECTED) != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, ZLANG_SPEC_CONFORMANCE_FUNCTION_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_CONFORMANCE_RETURN_EXPECTED) != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, ZLANG_SPEC_CONFORMANCE_ASSIGNMENT_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_CONFORMANCE_EQUAL_EXPECTED) != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, ZLANG_SPEC_CONFORMANCE_IDENTIFIER_EXPECTED) != NULL);
}

static void reports_syntax_diagnostics_with_source_locations(zlang_test_context_t *const context) {
    zlang_spec_session_t _unexpected;
    zlang_spec_session_t _unterminated;
    zlang_spec_session_t _invalid;
    zlang_spec_session_t _unsupported;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_unexpected, "integer var 10;"));
    zlang_spec_expect_diagnostic(context, &_unexpected, ZLANG_DIAGNOSTIC_UNEXPECTED_TOKEN, 1U, 13U);

    ZLANG_CHECK(context, !zlang_spec_transpile(&_unterminated, "print(\"oops);"));
    zlang_spec_expect_diagnostic(context, &_unterminated, ZLANG_DIAGNOSTIC_UNTERMINATED_STRING, 1U,
                                 7U);

    ZLANG_CHECK(context, !zlang_spec_transpile(&_invalid, "integer var = @;"));
    zlang_spec_expect_diagnostic(context, &_invalid, ZLANG_DIAGNOSTIC_INVALID_CHARACTER, 1U, 15U);

    ZLANG_CHECK(context, !zlang_spec_transpile(&_unsupported, "break;"));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_unsupported.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT));
}

static void
reports_type_and_resource_diagnostics_with_source_locations(zlang_test_context_t *const context) {
    zlang_spec_session_t _range;
    zlang_spec_session_t _duplicate;
    zlang_spec_session_t _undefined;
    zlang_spec_session_t _mismatch;
    zlang_spec_session_t _unbounded;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_range, "integer var = 99999999999999999999;"));
    zlang_spec_expect_diagnostic(context, &_range, ZLANG_DIAGNOSTIC_LITERAL_OUT_OF_RANGE, 1U, 15U);

    ZLANG_CHECK(context, !zlang_spec_transpile(&_duplicate, "integer var = 10; integer var = 20;"));
    zlang_spec_expect_diagnostic(context, &_duplicate, ZLANG_DIAGNOSTIC_DUPLICATE_DECLARATION, 1U,
                                 19U);

    ZLANG_CHECK(context, !zlang_spec_transpile(&_undefined, "integer var = missing;"));
    zlang_spec_expect_diagnostic(context, &_undefined, ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER, 1U,
                                 15U);

    ZLANG_CHECK(context, !zlang_spec_transpile(&_mismatch, "integer var = \"text\";"));
    zlang_spec_expect_diagnostic(context, &_mismatch, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, 1U, 15U);

    ZLANG_CHECK(context, !zlang_spec_transpile(&_unbounded, "sequence[] integer values;"));
    zlang_spec_expect_diagnostic(context, &_unbounded, ZLANG_DIAGNOSTIC_UNBOUNDED_SEQUENCE, 1U, 1U);
}

static void emits_append_and_checked_index(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_APPEND_INDEX_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_APPEND_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_INDEX_EXPECTED) != NULL);
}

static void emits_an_inferred_sequence_initializer(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_INFERRED_INITIALIZER_SOURCE));
    ZLANG_CHECK(context,
                strstr(_session.output_, ZLANG_SPEC_INFERRED_INITIALIZER_EXPECTED) != NULL);
}

static void emits_a_partial_explicit_sequence_initializer(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_PARTIAL_INITIALIZER_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_PARTIAL_INITIALIZER_EXPECTED) != NULL);
    zlang_spec_expect_warning(context, &_session, ZLANG_DIAGNOSTIC_UNUSED_CAPACITY, 1U, 1U);
}

static void emits_index_assignment_with_a_checked_store(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_INDEX_ASSIGNMENT_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_INDEX_ASSIGNMENT_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_INDEX_ASSIGNMENT_TRAP) != NULL);
}

static void folds_literal_string_concatenation(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_LITERAL_CONCAT_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_LITERAL_CONCAT_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_string_concat") == NULL);
}

static void emits_bounded_variable_string_concatenation(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_VARIABLE_CONCAT_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_VARIABLE_CONCAT_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "32U") != NULL);
}

static void emits_direct_function_calls(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_CALL_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_CALL_EXPECTED) != NULL);
}

static void emits_ordering_and_inequality_operators(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_OPERATORS_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_LESS_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "> 2") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "<= 4") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ">= 4") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_NOT_EQUAL_EXPECTED) != NULL);
}

static void renames_sibling_scope_declarations(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_SIBLING_SCOPES_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_SIBLING_SCOPES_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_print_i32(item_0)") != NULL);
}

static void rejects_a_shadowed_declaration(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context,
                !zlang_spec_transpile(&_session, "integer outer = 1; { integer outer = 2; }"));
    ZLANG_CHECK(context, _session.diagnostics_.count_ == 1U);
    zlang_spec_expect_diagnostic(context, &_session, ZLANG_DIAGNOSTIC_SHADOWED_DECLARATION, 1U,
                                 22U);
}

static void rejects_variable_concatenation_without_capacity(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "string prefix = \"a\";\nstring label = prefix + \"b\";";

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_STRING_CAPACITY_REQUIRED));
}

static void warns_about_unused_explicit_capacity(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_PARTIAL_INITIALIZER_SOURCE));
    zlang_spec_expect_warning(context, &_session, ZLANG_DIAGNOSTIC_UNUSED_CAPACITY, 1U, 1U);
}

static void rejects_an_initializer_longer_than_capacity(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context,
                !zlang_spec_transpile(&_session, "sequence[2] integer values = {1, 2, 3};"));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_INITIALIZER_TOO_LONG));
}

static void rejects_an_empty_inferred_sequence(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "sequence[] integer values = {};"));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_UNBOUNDED_SEQUENCE));
}

static void emits_else_if_chains(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_ELSE_IF_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_ELSE_IF_SECOND_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_ELSE_IF_THIRD_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_ELSE_IF_FINAL_EXPECTED) != NULL);
}

static void emits_logical_operators(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_LOGICAL_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_LOGICAL_NOT_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_LOGICAL_AND_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_LOGICAL_OR_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_LOGICAL_GROUP_EXPECTED) != NULL);
}

static void rejects_non_boolean_logical_operands(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "boolean bad = !1;"));
    ZLANG_CHECK(context,
                zlang_diagnostics_contains(&_session.diagnostics_, ZLANG_DIAGNOSTIC_TYPE_MISMATCH));
}

static void emits_break_and_continue(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_LOOP_JUMPS_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_BREAK_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_CONTINUE_EXPECTED) != NULL);
}

static void rejects_break_outside_loop(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "break;"));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT));
}

static void rejects_continue_outside_loop(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "continue;"));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT));
}

static void rejects_non_boolean_if_condition(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "if (1) { }"));
    zlang_spec_expect_diagnostic(context, &_session, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, 1U, 5U);
}

static void rejects_non_unsigned_repeat_count(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "repeat (2) { }"));
    zlang_spec_expect_diagnostic(context, &_session, ZLANG_DIAGNOSTIC_TYPE_MISMATCH, 1U, 9U);
}

static void validates_return_types(zlang_test_context_t *const context) {
    zlang_spec_session_t _valid;
    zlang_spec_session_t _invalid;

    ZLANG_CHECK(context, zlang_spec_transpile(&_valid, "function pick(): integer { return 1; }"));
    ZLANG_CHECK(context, !zlang_spec_transpile(&_invalid, "function pick(): integer { return; }"));
    ZLANG_CHECK(context,
                zlang_diagnostics_contains(&_invalid.diagnostics_, ZLANG_DIAGNOSTIC_TYPE_MISMATCH));
}

static void rejects_return_outside_function(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "return 1;"));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT));
}

static void accepts_self_concatenation_with_explicit_capacity(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_SELF_CONCAT_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_SELF_CONCAT_EXPECTED) != NULL);
}

static void does_not_duplicate_nested_statements(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_NESTED_NO_DUPLICATES_SOURCE));
    ZLANG_CHECK(context, zlang_spec_count_occurrences(_session.output_, "\"once\"") == 1U);
}

static void emits_arithmetic_operators(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_ARITHMETIC_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_SUB_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_MUL_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_DIV_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_MOD_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_NEGATIVE_EXPECTED) != NULL);
}

static void emits_compound_assignments(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_COMPOUND_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_COMPOUND_ADD_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_COMPOUND_MOD_EXPECTED) != NULL);
}

static void emits_length_builtin(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_LENGTH_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_LENGTH_EXPECTED) != NULL);
}

static void emits_scalar_print(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_SCALAR_PRINT_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_PRINT_I32_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_PRINT_U32_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_PRINT_BOOL_EXPECTED) != NULL);
}

static void emits_string_sequence_operations(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_STRING_SEQUENCE_SOURCE));
    ZLANG_CHECK(context,
                strstr(_session.output_, ZLANG_SPEC_STRING_SEQUENCE_TYPE_EXPECTED) != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, ZLANG_SPEC_STRING_SEQUENCE_APPEND_EXPECTED) != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, ZLANG_SPEC_STRING_SEQUENCE_STORAGE_EXPECTED) != NULL);
}

static void rejects_legacy_sequence_syntax(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, ZLANG_SPEC_LEGACY_SEQUENCE_SOURCE));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT));
}

static void emits_native_for_loops(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_FOR_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_FOR_EXPECTED) != NULL);
}

static void emits_nested_loops(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_NESTED_LOOPS_SOURCE));
    ZLANG_CHECK(context, zlang_spec_count_occurrences(_session.output_, "for (") == 4U);
}

static void emits_indexed_foreach(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_INDEXED_FOREACH_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_INDEXED_FOREACH_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_print_u32(index)") != NULL);
}

static void emits_const_declarations(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_CONST_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_CONST_EXPECTED) != NULL);
}

static void rejects_assignment_to_const(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, ZLANG_SPEC_CONST_ASSIGN_SOURCE));
    ZLANG_CHECK(context,
                zlang_diagnostics_contains(&_session.diagnostics_, ZLANG_DIAGNOSTIC_TYPE_MISMATCH));
}

static void emits_string_index_read(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_STRING_INDEX_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_STRING_INDEX_EXPECTED) != NULL);
}

#define ZLANG_TEST_CASE(name) {#name, name},
static const zlang_test_case_t cases_k[] = {
#include "test_cases.h"
};
#undef ZLANG_TEST_CASE

int main(const int argc, char **const argv) {
    return zlang_test_main(argc, argv, "zlang.spec", cases_k, sizeof(cases_k) / sizeof(cases_k[0]));
}
