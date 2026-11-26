#include "zlang/compiler/diagnostics.h"
#include "zlang/compiler/emitter.h"
#include "zlang/compiler/optimizer.h"
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
    zlang_comment_t comments_[64];
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
    char source_[8192];
    char scratch_[2048];
    char output_[8192];
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

static size_t zlang_spec_count_diagnostics(const zlang_spec_session_t *session, uint16_t code) {
    size_t _count_k = 0U;
    for (size_t _index = 0U; _index < session->diagnostics_.count_; ++_index) {
        if (session->diagnostic_items_[_index].code_ == code) {
            ++_count_k;
        }
    }
    return _count_k;
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
    const size_t _source_length_k = strlen(source);
    const bool _has_write_k = strstr(source, "write(") != NULL;
    const bool _has_import_k = strstr(source, "from \"@std/io\"") != NULL;
    if (_has_write_k && !_has_import_k) {
        const char _import_k[] = "\nimport { write } from \"@std/io\";";
        const size_t _import_length_k = sizeof(_import_k) - 1U;
        if (_source_length_k + _import_length_k >= sizeof(session->source_)) {
            return false;
        }
        memcpy(session->source_, source, _source_length_k);
        memcpy(session->source_ + _source_length_k, _import_k, _import_length_k + 1U);
        source = session->source_;
    }
    zlang_diagnostics_init(&session->diagnostics_, session->diagnostic_items_,
                           sizeof(session->diagnostic_items_) /
                               sizeof(session->diagnostic_items_[0]));
    zlang_parser_init(
        &session->parser_, session->tokens_, sizeof(session->tokens_) / sizeof(session->tokens_[0]),
        session->expressions_, sizeof(session->expressions_) / sizeof(session->expressions_[0]),
        session->statements_, sizeof(session->statements_) / sizeof(session->statements_[0]),
        session->parameters_, sizeof(session->parameters_) / sizeof(session->parameters_[0]),
        session->comments_, sizeof(session->comments_) / sizeof(session->comments_[0]),
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

    memset(&session->emitter_, 0, sizeof(session->emitter_));
    zlang_emitter_init(&session->emitter_, session->output_, sizeof(session->output_),
                       &session->diagnostics_, &session->plan_);
    return zlang_emit_program(&session->result_.program_, &session->emitter_);
}

static void
uses_static_storage_for_arrays_over_the_stack_limit(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_STATIC_SEQUENCE_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_STATIC_SEQUENCE_EXPECTED) != NULL);
}

static void keeps_small_arrays_automatic(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_FOREACH_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, "static app_array_values_t") == NULL);
}

static void emits_a_scalar_declaration(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_SCALAR_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_SCALAR_EXPECTED) != NULL);
}

static void emits_a_write_call(zlang_test_context_t *const context) {
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

static void rejects_an_unbounded_array_at_the_declaration(zlang_test_context_t *const context) {
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

static void preserves_comments_and_unescapes_strings(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k =
        "// first line\ninteger var = 10; /* block */\nwrite(\"say \\\"hi\\\"\");";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_SCALAR_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "say \\\"hi\\\"") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_gen_literal_0") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, "    // first line\n    int32_t app_local_var") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, "    /* block */\n    zlang_std_io_write") != NULL);
}

static void attaches_comments_to_source_constructs(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "// function note\n"
                                  "\n"
                                  "function abc(): integer {\n"
                                  "    // body note\n"
                                  "    // second body note\n"
                                  "    return 1;\n"
                                  "}\n"
                                  "// struct note\n"
                                  "struct A {\n"
                                  "    // field note\n"
                                  "    integer x;\n"
                                  "    /* field block\n"
                                  "       second line */\n"
                                  "    integer y;\n"
                                  "}\n"
                                  "// enum note\n"
                                  "enum E {\n"
                                  "    // item note\n"
                                  "    VALUE\n"
                                  "}\n";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context,
                strstr(_session.output_, "// function note\nint32_t app_fn_abc(void) {") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_,
                       "    // body note\n    // second body note\n    return 1;") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_,
                                "// struct note\ntypedef struct app_struct_a_t {") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "    // field note\n    int32_t x_;") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_,
                       "    /* field block\n       second line */\n    int32_t y_;") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, "// enum note\ntypedef enum app_enum_e_t {") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, "    // item note\n    ZLANG_E_VALUE = 0U,") != NULL);
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

static void emits_functions_with_user_defined_types(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_FUNCTION_USER_TYPES_SOURCE));
    ZLANG_CHECK(context,
                strstr(_session.output_, ZLANG_SPEC_FUNCTION_USER_TYPE_RETURN_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_,
                                ZLANG_SPEC_FUNCTION_USER_TYPE_PARAMETERS_EXPECTED) != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, ZLANG_SPEC_FUNCTION_USER_TYPE_CALL_EXPECTED) != NULL);
}

static void passes_complex_function_arguments(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k =
        "struct Point { integer x; integer y; }\n"
        "const integer base = 2;\n"
        "array[2] Point points;\n"
        "function add(clone integer left, clone integer right): integer { return left + right; }\n"
        "Point origin;\n"
        "points[0].x = 9;\n"
        "points[0].y = add(points[0].x, base);";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "app_local_base);") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "x_, 0);") == NULL);
}

static void emits_temporaries_without_synthetic_blocks(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "struct Point { integer x; }\n"
                                  "array[4] integer values;\n"
                                  "values[0] = 1;\n"
                                  "array[2] Point points;\n"
                                  "Point origin;\n"
                                  "points[0].x = 2;";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "\n    uint32_t zlang_gen_index_") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, "\n    app_struct_point_t *zlang_gen_field_ptr_") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "\n    {\n") == NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "\n        {\n") == NULL);
}

static void reuses_field_pointers_contextually(zlang_test_context_t *const context) {
    zlang_spec_session_t _reuse;
    const char *const _reuse_source_k = "struct Point { integer x; integer y; }\n"
                                        "array[2] Point points;\n"
                                        "Point origin;\n"
                                        "points[0].x = 9;\n"
                                        "points[0].y = 3;";

    ZLANG_CHECK(context, zlang_spec_transpile(&_reuse, _reuse_source_k));
    ZLANG_CHECK(context, zlang_spec_count_occurrences(
                             _reuse.output_, "app_struct_point_t *zlang_gen_field_ptr_") == 1U);
    ZLANG_CHECK(context, strstr(_reuse.output_, "->x_ = 9;") != NULL);
    ZLANG_CHECK(context, strstr(_reuse.output_, "->y_ = 3;") != NULL);

    zlang_spec_session_t _invalidate;
    const char *const _invalidate_source_k = "struct Point { integer x; }\n"
                                             "array[2] Point points;\n"
                                             "Point origin;\n"
                                             "unsigned index = 0U;\n"
                                             "points[index].x = 1;\n"
                                             "index = 1U;\n"
                                             "points[index].x = 2;";

    ZLANG_CHECK(context, zlang_spec_transpile(&_invalidate, _invalidate_source_k));
    ZLANG_CHECK(context,
                zlang_spec_count_occurrences(_invalidate.output_,
                                             "app_struct_point_t *zlang_gen_field_ptr_") == 2U);
}

static void reuses_index_values_across_if_branches(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "array[4] integer values = {1, 2, 3};\n"
                                  "if (values[0] > 3) {\n"
                                  "    write(\"big\");\n"
                                  "} else if (values[0] == 3) {\n"
                                  "    write(values[0]);\n"
                                  "} else {\n"
                                  "    write(\"small\");\n"
                                  "}";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context,
                zlang_spec_count_occurrences(_session.output_, "int32_t zlang_gen_value_") == 1U);
    ZLANG_CHECK(context,
                zlang_spec_count_occurrences(_session.output_, "zlang_array_get_i32") == 1U);
    ZLANG_CHECK(context, strstr(_session.output_, "} else if (zlang_gen_value_") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, "zlang_std_io_write_i32(zlang_gen_value_") != NULL);
}

static void keeps_short_circuit_index_reads_inline(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "boolean ready = true;\n"
                                  "array[4] integer values = {1};\n"
                                  "boolean ok = ready || values[0] == 1;";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_array_get_i32") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "int32_t zlang_gen_value_") == NULL);
}

static void emits_parameter_qualifiers(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k =
        "struct Point { integer x; }\n"
        "function bump(reference Point point, clone integer amount) {\n"
        "    point.x = point.x + amount;\n"
        "}\n"
        "function add_to(reference integer value, const clone integer amount) {\n"
        "    value = value + amount;\n"
        "}\n"
        "Point origin;\n"
        "origin.x = 1;\n"
        "integer total = 1;\n"
        "bump(origin, 2);\n"
        "add_to(total, 2);";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_,
                                "void app_fn_bump(app_struct_point_t *app_param_point, ") != NULL);
    ZLANG_CHECK(
        context,
        strstr(_session.output_,
               "app_param_point->x_ = zlang_add_i32(app_param_point->x_, app_param_amount);") !=
            NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "app_fn_bump(&app_local_origin, 2);") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_,
                       "int32_t *app_param_value, const int32_t app_param_amount") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "app_fn_add_to(&app_local_total, 2);") != NULL);
}

static void rejects_missing_parameter_qualifier(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    ZLANG_CHECK(context,
                !zlang_spec_transpile(&_session, "function bad(integer value): void { return; }"));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT));
}

static void rejects_mutation_of_const_parameter(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "struct Point { integer x; }\n"
                                  "function bad(const reference Point point) { point.x = 1; }";
    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context,
                zlang_diagnostics_contains(&_session.diagnostics_, ZLANG_DIAGNOSTIC_TYPE_MISMATCH));
}

static void rejects_non_lvalue_reference_argument(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "function bad(reference integer value) { return; }\n"
                                  "bad(1);";
    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context,
                zlang_diagnostics_contains(&_session.diagnostics_, ZLANG_DIAGNOSTIC_TYPE_MISMATCH));
}

static void warns_about_unnecessary_references(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k =
        "function read(const reference integer value): integer { return value; }\n"
        "function unused(reference boolean flag) { return; }";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, zlang_spec_count_diagnostics(
                             &_session, ZLANG_DIAGNOSTIC_UNNECESSARY_REFERENCE) == 2U);
}

static void accepts_mutated_scalar_reference(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "function bump(reference integer value) { value = value + 1; }";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, zlang_spec_count_diagnostics(
                             &_session, ZLANG_DIAGNOSTIC_UNNECESSARY_REFERENCE) == 0U);
}

static void emits_fixed_multidimensional_arrays(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "array[2][3] integer grid;\n"
                                  "grid[1][2] = 7;\n"
                                  "integer value = grid[1][2];";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_gen_array_offset_2") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_gen_array_get_2_i32") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_gen_array_set_2_i32") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "ZLANG_GRID_EXTENT_0 2U") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "ZLANG_GRID_EXTENT_1 3U") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_,
                       "zlang_gen_array_set_2_i32(app_local_grid.data_, 2, 3, 1, 2, 7);") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_,
                       "zlang_gen_array_get_2_i32(app_local_grid.data_, 2, 3, 1, 2)") != NULL);
}

static void rejects_source_array_helpers(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "array[2][3] integer grid;\n"
                                  "array_set(grid, {1, 2}, 7);";

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT));
}

static void infers_local_array_shape(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "array integer grid;\n"
                                  "for (integer row = 0; row < 2; row += 1) {\n"
                                  "    for (integer col = 0; col < 3; col += 1) {\n"
                                  "        grid[row][col] = row + col;\n"
                                  "    }\n"
                                  "}";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "ZLANG_GRID_EXTENT_0 2U") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "ZLANG_GRID_EXTENT_1 3U") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, "zlang_gen_array_set_2_i32(app_local_grid.data_") != NULL);
}

static void infers_array_function_shape(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "function abc(clone array value): array {\n"
                                  "    array result;\n"
                                  "    return result;\n"
                                  "}\n"
                                  "array[2][3] integer grid;\n"
                                  "array[2][3] integer copied = abc(grid);";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context,
                strstr(_session.output_,
                       "zlang_gen_array_i32_2_3_t app_fn_abc_2_3_i32(zlang_gen_array_i32_2_3_t "
                       "app_param_value)") != NULL);
    ZLANG_CHECK(
        context,
        strstr(
            _session.output_,
            "zlang_gen_array_i32_2_3_t app_local_copied = app_fn_abc_2_3_i32(app_local_grid);") !=
            NULL);
}

static void rejects_unknown_array_extent(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "integer index = 1;\n"
                                  "array integer values;\n"
                                  "values[index] = 3;";

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_UNBOUNDED_SEQUENCE));
}

static void emits_nested_array_initializer(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "array integer grid = {{1, 2, 3}, {4, 5, 6}};";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "ZLANG_GRID_EXTENT_0 2U") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "ZLANG_GRID_EXTENT_1 3U") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "{ { 1, 2, 3, 4, 5, 6 } }") != NULL);
}

static void rejects_const_array_write(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "const array[3] integer values = {1, 2, 3};\n"
                                  "values[1] = 9;";

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context,
                zlang_diagnostics_contains(&_session.diagnostics_, ZLANG_DIAGNOSTIC_TYPE_MISMATCH));
}

static void emits_whole_array_copy(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "array[3] integer source = {1, 2, 3};\n"
                                  "array[3] integer target = {0, 0, 0};\n"
                                  "target = source;";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context,
                strstr(_session.output_, "memcpy(&app_local_target, &app_local_source") != NULL);
}

static void emits_multidimensional_foreach(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "array[2][3] integer grid = {{1, 2, 3}, {4, 5, 6}};\n"
                                  "integer total = 0;\n"
                                  "foreach (row, col in grid) {\n"
                                  "    total += grid[row][col];\n"
                                  "}";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "for (uint32_t app_local_row") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "for (uint32_t app_local_col") != NULL);
    ZLANG_CHECK(context, zlang_spec_count_occurrences(_session.output_, "for (uint32_t ") == 2U);
}

static void emits_array_record_field_access(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "struct Matrix { array[2][3] integer cells; }\n"
                                  "Matrix matrix;\n"
                                  "matrix.cells[1][2] = 7;\n"
                                  "integer value = matrix.cells[1][2];";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_gen_array_i32_2_3_t cells_;") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_,
                       "zlang_gen_array_set_2_i32(app_local_matrix.cells_.data_, 2, 3") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_,
                       "zlang_gen_array_get_2_i32(app_local_matrix.cells_.data_, 2, 3") != NULL);
}

static void emits_multiple_array_specializations(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "function stamp(clone array source): array {\n"
                                  "    array result;\n"
                                  "    return result;\n"
                                  "}\n"
                                  "array[2][3] integer grid;\n"
                                  "array[2][3] integer copied = stamp(grid);\n"
                                  "array[4] integer line;\n"
                                  "array[4] integer copied2 = stamp(line);";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "app_fn_stamp_2_3_i32") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "app_fn_stamp_4_i32") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "app_fn_stamp_i32(") == NULL);
}

static void emits_three_dimensional_arrays(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k =
        "array[2][2][2] integer cube = {{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}};\n"
        "integer sum = 0;\n"
        "foreach (i, j, k in cube) {\n"
        "    sum += cube[i][j][k];\n"
        "}";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_gen_array_offset_3") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "for (uint32_t app_local_i") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "for (uint32_t app_local_j") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "for (uint32_t app_local_k") != NULL);
}

static void emits_four_dimensional_arrays(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "array[2][2][2][2] integer hyper = {\n"
                                  "    {{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}},\n"
                                  "    {{{9, 10}, {11, 12}}, {{13, 14}, {15, 16}}}\n"
                                  "};\n"
                                  "integer sum = 0;\n"
                                  "foreach (a, b, c, d in hyper) {\n"
                                  "    sum += hyper[a][b][c][d];\n"
                                  "}";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_gen_array_offset_4") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "for (uint32_t app_local_a") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "for (uint32_t app_local_b") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "for (uint32_t app_local_c") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "for (uint32_t app_local_d") != NULL);
}

static void rejects_reserved_identifiers(zlang_test_context_t *const context) {
    static const char *const sources_k[] = {
        "integer zlang_value = 1;", "integer zlang_gen_value = 1;", "integer ZLANG_VALUE = 1;"};
    for (size_t _index = 0U; _index < sizeof(sources_k) / sizeof(sources_k[0]); ++_index) {
        zlang_spec_session_t _session;
        ZLANG_CHECK(context, !zlang_spec_transpile(&_session, sources_k[_index]));
        ZLANG_CHECK(context, zlang_diagnostics_contains(&_session.diagnostics_,
                                                        ZLANG_DIAGNOSTIC_UNSUPPORTED_CONSTRUCT));
        ZLANG_CHECK(context, _session.output_[0] == '\0');
    }
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

    ZLANG_CHECK(context, !zlang_spec_transpile(&_unterminated, "write(\"oops);"));
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

    ZLANG_CHECK(context, !zlang_spec_transpile(&_unbounded, "array[] integer values;"));
    zlang_spec_expect_diagnostic(context, &_unbounded, ZLANG_DIAGNOSTIC_UNBOUNDED_SEQUENCE, 1U, 1U);
}

static void emits_append_and_checked_index(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_APPEND_INDEX_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_APPEND_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_INDEX_EXPECTED) != NULL);
}

static void emits_an_inferred_array_initializer(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_INFERRED_INITIALIZER_SOURCE));
    ZLANG_CHECK(context,
                strstr(_session.output_, ZLANG_SPEC_INFERRED_INITIALIZER_EXPECTED) != NULL);
}

static void emits_a_partial_explicit_array_initializer(zlang_test_context_t *const context) {
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
    ZLANG_CHECK(context, strstr(_session.output_, "ZLANG_LABEL_CAPACITY") != NULL);
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
    ZLANG_CHECK(context,
                strstr(_session.output_, "zlang_std_io_write_i32(app_local_item_0)") != NULL);
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

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "array[2] integer values = {1, 2, 3};"));
    ZLANG_CHECK(context, zlang_diagnostics_contains(&_session.diagnostics_,
                                                    ZLANG_DIAGNOSTIC_INITIALIZER_TOO_LONG));
}

static void rejects_an_empty_inferred_array(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "array[] integer values = {};"));
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
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_LOGICAL_COMPARISON_EXPECTED) != NULL);
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

static void emits_scalar_write(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_SCALAR_PRINT_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_PRINT_I32_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_PRINT_U32_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_PRINT_BOOL_EXPECTED) != NULL);
}

static void emits_string_array_operations(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_STRING_SEQUENCE_SOURCE));
    ZLANG_CHECK(context,
                strstr(_session.output_, ZLANG_SPEC_STRING_SEQUENCE_TYPE_EXPECTED) != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, ZLANG_SPEC_STRING_SEQUENCE_APPEND_EXPECTED) != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, ZLANG_SPEC_STRING_SEQUENCE_STORAGE_EXPECTED) != NULL);
}

static void rejects_legacy_array_syntax(zlang_test_context_t *const context) {
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
    ZLANG_CHECK(context,
                strstr(_session.output_, "zlang_std_io_write_u32(app_local_index)") != NULL);
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

static void uses_readable_emitted_layout(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_READABLE_LAYOUT_SOURCE));
    const char *const _type_position_k =
        strstr(_session.output_, "typedef struct app_array_values_t");
    const char *const _main_position_k = strstr(_session.output_, "int main(void)");
    ZLANG_CHECK(context, strstr(_session.output_, "#define ZLANG_VALUES_CAPACITY 3U") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "int32_t app_local_base = 2;") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "app_array_values_t app_local_values") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_gen_literal_0") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_array_get_i32") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "#define ZLANG_ZERO 0U") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "#define ZLANG_ONE 1U") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "#define ZLANG_TWO 2U") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "ZLANG_LITERAL_0_LENGTH") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "ZLANG_VALUES_LENGTH") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "(void)app_local_values") == NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "(void)app_local_label") == NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "(void)app_local_total") == NULL);
    ZLANG_CHECK(context, zlang_spec_count_occurrences(_session.output_, "\n\n    ") > 0U);
    ZLANG_CHECK(context, _type_position_k != NULL && _main_position_k != NULL &&
                             _type_position_k < _main_position_k);
}

static void emits_unique_array_types_for_sibling_scopes(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_SIBLING_SEQUENCE_TYPES_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_SIBLING_CAPACITY_FIRST) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_SIBLING_CAPACITY_SECOND) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_SIBLING_TYPE_SECOND) != NULL);
}

static void deduplicates_string_literals(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_LITERAL_DEDUPE_SOURCE));
    ZLANG_CHECK(context, zlang_spec_count_occurrences(
                             _session.output_, "static const char zlang_gen_literal_") == 1U);
}

static void emits_structs_and_enums(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_RECORD_ENUM_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_RECORD_TYPE_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_ENUM_TYPE_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_ENUM_VALUE_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_FIELD_ASSIGN_EXPECTED) != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, ZLANG_SPEC_SEQUENCE_FIELD_ASSIGN_EXPECTED) != NULL);
}

static void emits_function_prototypes(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_FORWARD_CALL_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_FORWARD_PROTOTYPE_EXPECTED) != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "/* generated function prototypes */") != NULL);
}

static void emits_io_read_calls(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "import { read, read_line } from \"@std/io\";\n"
                                  "string[16] line;\n"
                                  "integer a = read_line(line);\n"
                                  "integer b = read(line);";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_std_io_read_line(") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_std_io_read(") != NULL);
}

static void rejects_read_without_bounded_string(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "import { read } from \"@std/io\";\n"
                                  "string view = \"x\";\n"
                                  "integer n = read(view);";

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context,
                zlang_spec_find_diagnostic(&_session, ZLANG_DIAGNOSTIC_TYPE_MISMATCH) != NULL);
}

static bool zlang_spec_transpile_tto(zlang_spec_session_t *session, const char *source) {
    if (!zlang_spec_transpile(session, source)) {
        return false;
    }
    zlang_optimizer_context_t _optimizer;
    zlang_optimizer_context_init(&_optimizer);
    if (!zlang_optimize_program(&session->result_.program_, &_optimizer)) {
        return false;
    }
    session->output_[0] = '\0';
    memset(&session->emitter_, 0, sizeof(session->emitter_));
    zlang_emitter_init(&session->emitter_, session->output_, sizeof(session->output_),
                       &session->diagnostics_, &session->plan_);
    return zlang_emit_program(&session->result_.program_, &session->emitter_);
}

static void folds_proven_loop_indices(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "import { write } from \"@std/io\";\n"
                                  "array[10] integer values;\n"
                                  "for (integer index = 2; index < 10; index += 1) {\n"
                                  "    values[index] = values[index - 1] + values[index - 2];\n"
                                  "}";

    ZLANG_CHECK(context, zlang_spec_transpile_tto(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_trap(ZLANG_TRAP_BOUNDS)") == NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_array_get_i32") == NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ".data_[(uint32_t)app_local_index]") != NULL);
}

static void materializes_proven_nested_reads(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "import { write } from \"@std/io\";\n"
                                  "array[10] integer values;\n"
                                  "for (integer index = 2; index < 10; index += 1) {\n"
                                  "    values[index] = values[index - 1] + values[index - 2];\n"
                                  "}";

    ZLANG_CHECK(context, zlang_spec_transpile_tto(&_session, _source_k));
    ZLANG_CHECK(context,
                zlang_spec_count_occurrences(_session.output_, "int32_t zlang_gen_read_") == 2U);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_add_i32(zlang_gen_read_") != NULL);
}

static void keeps_unproven_bounds_traps(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "import { write } from \"@std/io\";\n"
                                  "array[4] integer values;\n"
                                  "integer limit = 6;\n"
                                  "for (integer index = 0; index < limit; index += 1) {\n"
                                  "    values[index] = 1;\n"
                                  "}";

    ZLANG_CHECK(context, zlang_spec_transpile_tto(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_trap(ZLANG_TRAP_BOUNDS)") != NULL);
}

static void emits_struct_constructor_and_methods(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k =
        "struct Counter {\n"
        "    integer value;\n"
        "    constructor(clone integer initial) { value = initial; }\n"
        "    function increment(): integer { value += 1; return value; }\n"
        "}\n"
        "Counter counter = Counter(1);\n"
        "integer result = counter.increment();";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "app_fn_counter_construct") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "app_fn_counter_increment") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "self->value_") != NULL);
}

static void emits_static_struct_method(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "struct Counter {\n"
                                  "    integer value;\n"
                                  "    constructor(clone integer initial) { value = initial; }\n"
                                  "    static function zero(): Counter { return Counter(0); }\n"
                                  "}\n"
                                  "Counter counter = Counter.zero();";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "app_fn_counter_zero(void)") != NULL);
}

static void rejects_method_field_name_conflict(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "struct Counter {\n"
                                  "    integer value;\n"
                                  "    function value(): integer { return 1; }\n"
                                  "}";

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, zlang_spec_find_diagnostic(
                             &_session, ZLANG_DIAGNOSTIC_DUPLICATE_DECLARATION) != NULL);
}

static void rejects_print_keyword(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "print(\"x\");"));
    ZLANG_CHECK(context, zlang_spec_find_diagnostic(&_session,
                                                    ZLANG_DIAGNOSTIC_UNDEFINED_IDENTIFIER) != NULL);
}

static void rejects_mixed_numeric_addition(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "integer value = 1 + 2U;"));
    ZLANG_CHECK(context,
                zlang_spec_find_diagnostic(&_session, ZLANG_DIAGNOSTIC_TYPE_MISMATCH) != NULL);
}

static void rejects_mixed_numeric_comparison(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "boolean value = 1.5 == 1;"));
    ZLANG_CHECK(context,
                zlang_spec_find_diagnostic(&_session, ZLANG_DIAGNOSTIC_TYPE_MISMATCH) != NULL);
}

static void compares_strings_by_content(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, "boolean value = \"a\" == \"a\";"));
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_string_equal(") != NULL);
}

static void rejects_string_ordering(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, "boolean value = \"a\" < \"b\";"));
    ZLANG_CHECK(context,
                zlang_spec_find_diagnostic(&_session, ZLANG_DIAGNOSTIC_TYPE_MISMATCH) != NULL);
}

static void emits_checked_unary_negation(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, "integer counter = 3;\n"
                                                         "integer value = -counter;"));
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_neg_i32(app_local_counter)") != NULL);
}

static void emits_math_overloads(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "import { abs, clamp, min, max, sqrt } from \"@std/math\";\n"
                                  "integer a = abs(3);\n"
                                  "double b = sqrt(9.0);\n"
                                  "integer c = clamp(7, 1, 5);\n"
                                  "integer d = min(2, 3);\n"
                                  "integer e = max(2, 3);";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_std_math_abs_i32(3)") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_std_math_sqrt_f64(9.0)") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_std_math_clamp_i32(") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_std_math_min_i32(") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_std_math_max_i32(") != NULL);
}

static void emits_strings_api(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k =
        "import { compare, contains, starts_with } from \"@std/strings\";\n"
        "integer a = compare(\"a\", \"b\");\n"
        "boolean b = contains(\"abc\", \"b\");\n"
        "boolean c = starts_with(\"abc\", \"a\");";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_std_strings_compare(") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_std_strings_contains(") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_std_strings_starts_with(") != NULL);
}

static void emits_wide_and_floating_primitives(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "int64 big = 9000000000i64 + 1i64;\n"
                                  "uint64 wide = 18000000000000000000u64;\n"
                                  "float ratio = 1.5f * 2f;\n"
                                  "double exact = 2.5 + 1.25;\n"
                                  "write(big);\n"
                                  "write(wide);\n"
                                  "write(ratio);\n"
                                  "write(exact);";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "int64_t app_local_big") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "uint64_t app_local_wide") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "float app_local_ratio") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "double app_local_exact") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_add_i64(9000000000LL, 1LL)") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "(1.5f * 2.0f)") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, "zlang_std_io_write_f32(app_local_ratio)") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, "zlang_std_io_write_f64(app_local_exact)") != NULL);
}

static void emits_explicit_numeric_conversions(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "integer truncated = to_integer(3.75);\n"
                                  "double widened = to_double(7);\n"
                                  "int64 signed_value = to_int64(9U);\n"
                                  "float narrowed = to_float(widened);";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_to_i32_f64(") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "(double)7") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_to_i64_i64(") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "(float)app_local_widened") != NULL);
}

static void rejects_implicit_numeric_conversion(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "integer value = 2.5;";

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context,
                zlang_spec_find_diagnostic(&_session, ZLANG_DIAGNOSTIC_TYPE_MISMATCH) != NULL);
}

static void rejects_index_outside_inferred_shape(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "function stamp(clone array source): array {\n"
                                  "    array result;\n"
                                  "    result[9] = 1;\n"
                                  "    return result;\n"
                                  "}\n"
                                  "array[2] integer small;\n"
                                  "array[4] integer large;\n"
                                  "array[2] integer a = stamp(small);\n"
                                  "array[4] integer b = stamp(large);";

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context,
                zlang_spec_find_diagnostic(&_session, ZLANG_DIAGNOSTIC_UNBOUNDED_SEQUENCE) != NULL);
}

static void rejects_float_modulo(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "float value = 2.5f % 1.5f;";

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context,
                zlang_spec_find_diagnostic(&_session, ZLANG_DIAGNOSTIC_TYPE_MISMATCH) != NULL);
}

static void emits_only_used_array_helpers(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_USED_HELPERS_SOURCE));
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_gen_array_get_2_i32") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ZLANG_SPEC_UNUSED_HELPER) == NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_gen_array_helpers_keep") == NULL);
}

static void reuses_repeated_index_reads(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, ZLANG_SPEC_CSE_SOURCE));
    ZLANG_CHECK(context,
                zlang_spec_count_occurrences(_session.output_, "zlang_array_get_i32") == 1U);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_gen_value_") != NULL);
}

static void emits_struct_inheritance_and_vtable(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k =
        "struct Shape { integer value; function area(): integer { return value; } }\n"
        "struct Square : Shape { integer side; }\n"
        "Square square;\n"
        "integer result = square.value;";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "app_struct_shape_t zlang_base_") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "ZLANG_TYPE_SQUARE") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_vtable_shape") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "app_local_square.zlang_base_.value_") != NULL);
}

static void emits_dynamic_dispatch_call(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k =
        "struct Shape { integer value; function area(): integer { return value; } }\n"
        "struct Square : Shape { integer side; function area(): integer { return side; } }\n"
        "function describe(reference Shape shape): integer { return shape.area(); }\n"
        "Square square;\n"
        "integer result = describe(square);";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_slot_area_") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, "zlang_vtable_square_area") != NULL);
    ZLANG_CHECK(context, strstr(_session.output_, ")->zlang_vtable_->zlang_slot_area_") != NULL);
}

static void emits_trait_method_direct_call(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "trait Named requires integer value {\n"
                                  "    function label(): integer { return value; }\n"
                                  "}\n"
                                  "struct Counter uses Named { integer value; }\n"
                                  "function main(): integer {\n"
                                  "    Counter counter;\n"
                                  "    integer result = counter.label();\n"
                                  "    return result;\n"
                                  "}";

    ZLANG_CHECK(context, zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context, strstr(_session.output_, "app_fn_counter_label") != NULL);
    ZLANG_CHECK(context,
                strstr(_session.output_, "app_fn_counter_label(&app_local_counter)") != NULL);
}

static void rejects_downcast(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k = "struct Shape { integer value; }\n"
                                  "struct Square : Shape { integer side; }\n"
                                  "Shape base;\n"
                                  "Square square = base;";

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context,
                zlang_spec_find_diagnostic(&_session, ZLANG_DIAGNOSTIC_TYPE_MISMATCH) != NULL);
}

static void rejects_override_signature_mismatch(zlang_test_context_t *const context) {
    zlang_spec_session_t _session;
    const char *const _source_k =
        "struct Shape { function area(): integer { return 1; } }\n"
        "struct Square : Shape { function area(clone integer scale): integer { return scale; } }";

    ZLANG_CHECK(context, !zlang_spec_transpile(&_session, _source_k));
    ZLANG_CHECK(context,
                zlang_spec_find_diagnostic(&_session, ZLANG_DIAGNOSTIC_TYPE_MISMATCH) != NULL);
}

#define ZLANG_TEST_CASE(name) {#name, name},
static const zlang_test_case_t cases_k[] = {
#include "test_cases.h"
};
#undef ZLANG_TEST_CASE

int main(const int argc, char **const argv) {
    return zlang_test_main(argc, argv, "zlang.spec", cases_k, sizeof(cases_k) / sizeof(cases_k[0]));
}
