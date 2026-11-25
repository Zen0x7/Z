#ifndef ZLANG_SPEC_FIXTURES_H
#define ZLANG_SPEC_FIXTURES_H

/* These literals are copied from the conformance examples in SPEC.md. The
   parser consumes the source strings and the emitter is checked against the
   expected C strings, keeping the contract in one place. */

#define ZLANG_SPEC_SCALAR_SOURCE "integer var = 10;"
#define ZLANG_SPEC_SCALAR_EXPECTED "int32_t _var = 10;"

#define ZLANG_SPEC_PRINT_SOURCE "print(\"Hello\");"
#define ZLANG_SPEC_PRINT_EXPECTED "zlang_print(_zlang_literal_0);"

#define ZLANG_SPEC_FOREACH_SOURCE                                                                  \
    "array[4] integer values;\n"                                                                   \
    "foreach (item in values) {\n"                                                                 \
    "    print(\"Hello\");\n"                                                                      \
    "}"
#define ZLANG_SPEC_FOREACH_SEQUENCE_EXPECTED "zlang_values_t _values = { { 0, 0, 0, 0 } };"
#define ZLANG_SPEC_FOREACH_LOOP_EXPECTED                                                           \
    "for (uint32_t _zlang_foreach_1 = ZLANG_ZERO; _zlang_foreach_1 < 4; "                          \
    "++_zlang_foreach_1)"
#define ZLANG_SPEC_FOREACH_ITEM_EXPECTED "int32_t _item = _values.data_[_zlang_foreach_1];"
#define ZLANG_SPEC_FOREACH_CALLBACK "zlang_for_each"

#define ZLANG_SPEC_UNBOUNDED_SOURCE "array[] integer values;"
#define ZLANG_SPEC_UNBOUNDED_EXPECTED_CODE 3001U

#define ZLANG_SPEC_STATIC_SEQUENCE_SOURCE "array[32] integer values;"
#define ZLANG_SPEC_STATIC_SEQUENCE_EXPECTED "static zlang_values_t _values ="
#define ZLANG_SPEC_ADD_SOURCE "integer total = 1 + 2;"
#define ZLANG_SPEC_ADD_EXPECTED "int32_t _total = zlang_add_i32(1, 2);"

#define ZLANG_SPEC_IF_SOURCE                                                                       \
    "boolean ready = true;\n"                                                                      \
    "if (ready) {\n"                                                                               \
    "    print(\"yes\");\n"                                                                        \
    "} else {\n"                                                                                   \
    "    print(\"no\");\n"                                                                         \
    "}"
#define ZLANG_SPEC_IF_EXPECTED "if (_ready) {"
#define ZLANG_SPEC_IF_ELSE_EXPECTED "} else {"

#define ZLANG_SPEC_REPEAT_SOURCE "repeat (2U) { print(\"tick\"); }"
#define ZLANG_SPEC_REPEAT_EXPECTED                                                                 \
    "for (uint32_t _zlang_repeat_1 = ZLANG_ZERO; _zlang_repeat_1 < 2U; ++_zlang_repeat_1) {"

#define ZLANG_SPEC_FUNCTION_SOURCE                                                                 \
    "function add(clone integer left, clone integer right): integer {\n"                           \
    "    return left + right;\n"                                                                   \
    "}"
#define ZLANG_SPEC_FUNCTION_EXPECTED "int32_t _u_add(int32_t left, int32_t right) {"
#define ZLANG_SPEC_FUNCTION_RETURN_EXPECTED "return zlang_add_i32(left, right);"

#define ZLANG_SPEC_FUNCTION_USER_TYPES_SOURCE                                                      \
    "record Point { integer x; integer y; }\n"                                                     \
    "enum Color { RED, BLUE }\n"                                                                   \
    "function make(): Point {\n"                                                                   \
    "    Point result;\n"                                                                          \
    "    result.x = 1;\n"                                                                          \
    "    return result;\n"                                                                         \
    "}\n"                                                                                          \
    "function paint(clone Point value, clone Color color): Point {\n"                              \
    "    value.x = 2;\n"                                                                           \
    "    return value;\n"                                                                          \
    "}\n"                                                                                          \
    "Point made = make();\n"                                                                       \
    "Color chosen = BLUE;\n"                                                                       \
    "Point painted = paint(made, chosen);"
#define ZLANG_SPEC_FUNCTION_USER_TYPE_RETURN_EXPECTED "zlang_point_t _u_make(void) {"
#define ZLANG_SPEC_FUNCTION_USER_TYPE_PARAMETERS_EXPECTED                                          \
    "zlang_point_t _u_paint(zlang_point_t value, zlang_color_t color) {"
#define ZLANG_SPEC_FUNCTION_USER_TYPE_CALL_EXPECTED                                                \
    "zlang_point_t _painted = _u_paint(_made, _chosen);"

#define ZLANG_SPEC_APPEND_INDEX_SOURCE                                                             \
    "array[4] integer values;\n"                                                                   \
    "integer first = values[0];"
#define ZLANG_SPEC_APPEND_EXPECTED "zlang_values_t _values = "
#define ZLANG_SPEC_INDEX_EXPECTED "zlang_array_get_i32(_values.data_, 0"

#define ZLANG_SPEC_CONFORMANCE_SOURCE                                                              \
    "integer first = 1;\n"                                                                         \
    "unsigned count = 2U;\n"                                                                       \
    "boolean ready = false;\n"                                                                     \
    "string label = \"text\";\n"                                                                   \
    "array[4] integer values;\n"                                                                   \
    "{ integer scoped = 3; }\n"                                                                    \
    "if (ready) { print(\"yes\"); }\n"                                                             \
    "while (ready) { }\n"                                                                          \
    "foreach (item in values) { print(label); }\n"                                                 \
    "repeat (2U) { }\n"                                                                            \
    "function noop() { return; }\n"                                                                \
    "first = 4;\n"                                                                                 \
    "boolean matched = first == 4;\n"

#define ZLANG_SPEC_CONFORMANCE_INTEGER_EXPECTED "int32_t _first = 1;"
#define ZLANG_SPEC_CONFORMANCE_UNSIGNED_EXPECTED "uint32_t _count = 2U;"
#define ZLANG_SPEC_CONFORMANCE_BOOLEAN_EXPECTED "bool _ready = false;"
#define ZLANG_SPEC_CONFORMANCE_STRING_EXPECTED "zlang_string_t _label = _zlang_literal_0;"
#define ZLANG_SPEC_CONFORMANCE_BLOCK_EXPECTED "{\n        int32_t _scoped = 3;"
#define ZLANG_SPEC_CONFORMANCE_IF_EXPECTED "if (_ready) {"
#define ZLANG_SPEC_CONFORMANCE_WHILE_EXPECTED "while (_ready) {"
#define ZLANG_SPEC_CONFORMANCE_FOREACH_EXPECTED                                                    \
    "for (uint32_t _zlang_foreach_2 = ZLANG_ZERO; _zlang_foreach_2 < 4; "                          \
    "++_zlang_foreach_2)"
#define ZLANG_SPEC_CONFORMANCE_REPEAT_EXPECTED                                                     \
    "for (uint32_t _zlang_repeat_3 = ZLANG_ZERO; _zlang_repeat_3 < 2U;"
#define ZLANG_SPEC_CONFORMANCE_FUNCTION_EXPECTED "void _u_noop(void) {"
#define ZLANG_SPEC_CONFORMANCE_RETURN_EXPECTED "return;"
#define ZLANG_SPEC_CONFORMANCE_ASSIGNMENT_EXPECTED "_first = 4;"
#define ZLANG_SPEC_CONFORMANCE_EQUAL_EXPECTED "_first == 4"
#define ZLANG_SPEC_CONFORMANCE_IDENTIFIER_EXPECTED "zlang_print(_label);"

#define ZLANG_SPEC_INFERRED_INITIALIZER_SOURCE "array[] integer values = {1, 2, 3};"
#define ZLANG_SPEC_INFERRED_INITIALIZER_EXPECTED "zlang_array_i32_3_t _values = { { 1, 2, 3 } };"

#define ZLANG_SPEC_PARTIAL_INITIALIZER_SOURCE "array[5] integer values = {1, 2, 3};"
#define ZLANG_SPEC_PARTIAL_INITIALIZER_EXPECTED "zlang_values_t _values = { { 1, 2, 3, 0, 0 } };"

#define ZLANG_SPEC_INDEX_ASSIGNMENT_SOURCE                                                         \
    "array[4] integer values;\n"                                                                   \
    "values[0] = 10;"
#define ZLANG_SPEC_INDEX_ASSIGNMENT_EXPECTED "_values.data_[_zlang_index_0] = 10;"
#define ZLANG_SPEC_INDEX_ASSIGNMENT_TRAP "zlang_trap(ZLANG_TRAP_BOUNDS);"

#define ZLANG_SPEC_LITERAL_CONCAT_SOURCE "string message = \"temp_\" + \"_01\";"
#define ZLANG_SPEC_LITERAL_CONCAT_EXPECTED "_zlang_literal_0;"

#define ZLANG_SPEC_VARIABLE_CONCAT_SOURCE                                                          \
    "string prefix = \"temp_\";\n"                                                                 \
    "string[32] label = prefix + \"sensor\";"
#define ZLANG_SPEC_VARIABLE_CONCAT_EXPECTED "ZLANG_LABEL_CAPACITY"

#define ZLANG_SPEC_CALL_SOURCE                                                                     \
    "function add(clone integer left, clone integer right): integer {\n"                           \
    "    return left + right;\n"                                                                   \
    "}\n"                                                                                          \
    "integer total = add(2, 3);"
#define ZLANG_SPEC_CALL_EXPECTED "int32_t _total = _u_add(2, 3);"

#define ZLANG_SPEC_OPERATORS_SOURCE                                                                \
    "integer total = 4;\n"                                                                         \
    "boolean smaller = total < 10;\n"                                                              \
    "boolean bigger = total > 2;\n"                                                                \
    "boolean lower_equal = total <= 4;\n"                                                          \
    "boolean greater_equal = total >= 4;\n"                                                        \
    "boolean different = total != 2;"
#define ZLANG_SPEC_LESS_EXPECTED "bool _smaller = _total < 10;"
#define ZLANG_SPEC_NOT_EQUAL_EXPECTED "bool _different = _total != 2;"

#define ZLANG_SPEC_SIBLING_SCOPES_SOURCE                                                           \
    "{ integer item = 1; }\n"                                                                      \
    "{ integer item = 2; print(item); }"
#define ZLANG_SPEC_SIBLING_SCOPES_EXPECTED "int32_t _item_0 = 2;"

#define ZLANG_SPEC_ELSE_IF_SOURCE                                                                  \
    "integer value = 2;\n"                                                                         \
    "if (value == 1) { print(\"one\"); }\n"                                                        \
    "else if (value == 2) { print(\"two\"); }\n"                                                   \
    "else if (value == 3) { print(\"three\"); }\n"                                                 \
    "else { print(\"other\"); }"
#define ZLANG_SPEC_ELSE_IF_SECOND_EXPECTED "} else if (_value == 2) {"
#define ZLANG_SPEC_ELSE_IF_THIRD_EXPECTED "} else if (_value == 3) {"
#define ZLANG_SPEC_ELSE_IF_FINAL_EXPECTED "} else {"

#define ZLANG_SPEC_LOGICAL_SOURCE                                                                  \
    "boolean a = true;\n"                                                                          \
    "boolean b = false;\n"                                                                         \
    "boolean negated = !a;\n"                                                                      \
    "boolean both = a && b;\n"                                                                     \
    "boolean either = a || b;\n"                                                                   \
    "boolean mixed = !(a && b) || a;\n"                                                            \
    "integer value = 2;\n"                                                                         \
    "boolean compared = !(value > 10);"
#define ZLANG_SPEC_LOGICAL_NOT_EXPECTED "bool _negated = (!_a);"
#define ZLANG_SPEC_LOGICAL_AND_EXPECTED "bool _both = (_a && _b);"
#define ZLANG_SPEC_LOGICAL_OR_EXPECTED "bool _either = (_a || _b);"
#define ZLANG_SPEC_LOGICAL_GROUP_EXPECTED "bool _mixed = ((!(_a && _b)) || _a);"
#define ZLANG_SPEC_LOGICAL_COMPARISON_EXPECTED "bool _compared = (!(_value > 10));"

#define ZLANG_SPEC_LOOP_JUMPS_SOURCE                                                               \
    "array[] integer values = {1, 2, 3};\n"                                                        \
    "foreach (item in values) {\n"                                                                 \
    "    if (item == 2) { continue; }\n"                                                           \
    "    if (item == 3) { break; }\n"                                                              \
    "    print(\"kept\");\n"                                                                       \
    "}\n"                                                                                          \
    "while (true) { break; }\n"                                                                    \
    "repeat (2U) { continue; }"
#define ZLANG_SPEC_BREAK_EXPECTED "break;"
#define ZLANG_SPEC_CONTINUE_EXPECTED "continue;"

#define ZLANG_SPEC_SELF_CONCAT_SOURCE                                                              \
    "string[16] label = \"a\";\n"                                                                  \
    "label = label + \"b\";"
#define ZLANG_SPEC_SELF_CONCAT_EXPECTED "(zlang_string_t[]){ _label, "

#define ZLANG_SPEC_NESTED_NO_DUPLICATES_SOURCE                                                     \
    "array[] integer values = {1};\n"                                                              \
    "foreach (item in values) {\n"                                                                 \
    "    if (item == 1) { print(\"once\"); }\n"                                                    \
    "}"

#define ZLANG_SPEC_ARITHMETIC_SOURCE                                                               \
    "integer total = 10 - 2;\n"                                                                    \
    "integer product = total * 3;\n"                                                               \
    "integer quotient = product / 2;\n"                                                            \
    "integer remainder = product % 5;\n"                                                           \
    "integer negative = -total;"
#define ZLANG_SPEC_SUB_EXPECTED "int32_t _total = zlang_sub_i32(10, 2);"
#define ZLANG_SPEC_MUL_EXPECTED "zlang_mul_i32(_total, 3)"
#define ZLANG_SPEC_DIV_EXPECTED "zlang_div_i32(_product, 2)"
#define ZLANG_SPEC_MOD_EXPECTED "zlang_mod_i32(_product, 5)"
#define ZLANG_SPEC_NEGATIVE_EXPECTED "int32_t _negative = (-_total);"

#define ZLANG_SPEC_COMPOUND_SOURCE                                                                 \
    "integer total = 1;\n"                                                                         \
    "total += 2;\n"                                                                                \
    "total -= 1;\n"                                                                                \
    "total *= 3;\n"                                                                                \
    "total /= 2;\n"                                                                                \
    "total %= 2;"
#define ZLANG_SPEC_COMPOUND_ADD_EXPECTED "_total = zlang_add_i32(_total, 2);"
#define ZLANG_SPEC_COMPOUND_MOD_EXPECTED "_total = zlang_mod_i32(_total, 2);"

#define ZLANG_SPEC_LENGTH_SOURCE                                                                   \
    "array[4] integer values = {1, 2};\n"                                                          \
    "unsigned count = length(values);"
#define ZLANG_SPEC_LENGTH_EXPECTED "uint32_t _count = 4;"

#define ZLANG_SPEC_SCALAR_PRINT_SOURCE                                                             \
    "print(1);\n"                                                                                  \
    "print(2U);\n"                                                                                 \
    "print(true);"
#define ZLANG_SPEC_PRINT_I32_EXPECTED "zlang_print_i32(1);"
#define ZLANG_SPEC_PRINT_U32_EXPECTED "zlang_print_u32(2U);"
#define ZLANG_SPEC_PRINT_BOOL_EXPECTED "zlang_print_bool(true);"

#define ZLANG_SPEC_STRING_SEQUENCE_SOURCE                                                          \
    "array[2] string[8] names = {\"a\", \"b\"};\n"                                                 \
    "names[0] = \"d\";"
#define ZLANG_SPEC_STRING_SEQUENCE_TYPE_EXPECTED "typedef struct zlang_names_t"
#define ZLANG_SPEC_STRING_SEQUENCE_APPEND_EXPECTED "_names.data_[0] = zlang_string_concat("
#define ZLANG_SPEC_STRING_SEQUENCE_STORAGE_EXPECTED "ZLANG_NAMES_ELEMENT_CAPACITY + ZLANG_ONE"

#define ZLANG_SPEC_LEGACY_SEQUENCE_SOURCE "integer values[4];"

#define ZLANG_SPEC_FOR_SOURCE                                                                      \
    "array[3] integer values = {1, 2, 3};\n"                                                       \
    "for (unsigned index = 0U; index < length(values); index += 1U) {\n"                           \
    "    print(values[index]);\n"                                                                  \
    "}"
#define ZLANG_SPEC_FOR_EXPECTED                                                                    \
    "for (uint32_t _index = 0U; (_index < 3); _index = "                                           \
    "zlang_add_u32(_index, 1U))"

#define ZLANG_SPEC_NESTED_LOOPS_SOURCE                                                             \
    "array[2] integer values = {1, 2};\n"                                                          \
    "for (unsigned outer = 0U; outer < 2U; outer += 1U) {\n"                                       \
    "    for (unsigned inner = 0U; inner < 2U; inner += 1U) {\n"                                   \
    "        print(outer);\n"                                                                      \
    "    }\n"                                                                                      \
    "}\n"                                                                                          \
    "foreach (row, item in values) {\n"                                                            \
    "    foreach (column, other in values) {\n"                                                    \
    "        print(row);\n"                                                                        \
    "    }\n"                                                                                      \
    "}"

#define ZLANG_SPEC_INDEXED_FOREACH_SOURCE                                                          \
    "array[2] integer values = {1, 2};\n"                                                          \
    "foreach (index, item in values) {\n"                                                          \
    "    print(index);\n"                                                                          \
    "    print(item);\n"                                                                           \
    "}"
#define ZLANG_SPEC_INDEXED_FOREACH_EXPECTED "uint32_t _index = _zlang_foreach_0;"

#define ZLANG_SPEC_CONST_SOURCE "const integer limit = 10;"
#define ZLANG_SPEC_CONST_EXPECTED "const int32_t _limit = 10;"
#define ZLANG_SPEC_CONST_ASSIGN_SOURCE "const integer limit = 10;\nlimit = 11;"

#define ZLANG_SPEC_STRING_INDEX_SOURCE                                                             \
    "string[8] text = \"abc\";\n"                                                                  \
    "unsigned first = text[0];"
#define ZLANG_SPEC_STRING_INDEX_EXPECTED "zlang_string_byte(_text, 0)"

#define ZLANG_SPEC_SIBLING_SEQUENCE_TYPES_SOURCE                                                   \
    "{ array[2] integer values; }\n"                                                               \
    "{ array[3] integer values; }"
#define ZLANG_SPEC_SIBLING_CAPACITY_FIRST "ZLANG_VALUES_CAPACITY 2U"
#define ZLANG_SPEC_SIBLING_CAPACITY_SECOND "ZLANG_VALUES_0_CAPACITY 3U"
#define ZLANG_SPEC_SIBLING_TYPE_SECOND "zlang_values_0_t"

#define ZLANG_SPEC_LITERAL_DEDUPE_SOURCE "print(\"a\");\nprint(\"a\");"

#define ZLANG_SPEC_RECORD_ENUM_SOURCE                                                              \
    "record Point {\n"                                                                             \
    "    integer x;\n"                                                                             \
    "    integer y;\n"                                                                             \
    "}\n"                                                                                          \
    "enum Color {\n"                                                                               \
    "    RED,\n"                                                                                   \
    "    GREEN,\n"                                                                                 \
    "    BLUE\n"                                                                                   \
    "}\n"                                                                                          \
    "Point point;\n"                                                                               \
    "point.x = 1;\n"                                                                               \
    "array[2] Point points;\n"                                                                     \
    "points[0].x = 2;\n"                                                                           \
    "print(points[0].y);\n"                                                                        \
    "Color current = GREEN;\n"                                                                     \
    "print(current);"
#define ZLANG_SPEC_RECORD_TYPE_EXPECTED "typedef struct zlang_point_t"
#define ZLANG_SPEC_ENUM_TYPE_EXPECTED "typedef enum zlang_color_t"
#define ZLANG_SPEC_ENUM_VALUE_EXPECTED "ZLANG_COLOR_GREEN"
#define ZLANG_SPEC_FIELD_ASSIGN_EXPECTED "_point.x_ = 1;"
#define ZLANG_SPEC_SEQUENCE_FIELD_ASSIGN_EXPECTED "_zlang_field_ptr_"

#define ZLANG_SPEC_CSE_SOURCE                                                                      \
    "array[2] integer values = {1, 2};\n"                                                          \
    "if (values[0] > 1) { print(\"a\"); }\n"                                                       \
    "else if (values[0] == 2) { print(\"b\"); }"

#define ZLANG_SPEC_READABLE_LAYOUT_SOURCE                                                          \
    "const integer base = 2;\n"                                                                    \
    "array[3] integer values = {1, 2, 3};\n"                                                       \
    "string label = \"text\";\n"                                                                   \
    "integer first = values[0];\n"                                                                 \
    "print(label);"

#define ZLANG_SPEC_TEST_STACK_LIMIT 64U

#endif /* ZLANG_SPEC_FIXTURES_H */
