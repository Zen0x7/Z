#ifndef ZLANG_SPEC_FIXTURES_H
#define ZLANG_SPEC_FIXTURES_H

/* These literals are copied from the conformance examples in SPEC.md. The
   parser consumes the source strings and the emitter is checked against the
   expected C strings, keeping the contract in one place. */

#define ZLANG_SPEC_SCALAR_SOURCE "integer var = 10;"
#define ZLANG_SPEC_SCALAR_EXPECTED "int32_t var = 10;"

#define ZLANG_SPEC_PRINT_SOURCE "print(\"Hello\");"
#define ZLANG_SPEC_PRINT_EXPECTED "zlang_print((zlang_string_t){ \"Hello\", 5U });"

#define ZLANG_SPEC_FOREACH_SOURCE                                                                  \
    "integer values[4];\n"                                                                         \
    "foreach (item in values) {\n"                                                                 \
    "    print(\"Hello\");\n"                                                                      \
    "}"
#define ZLANG_SPEC_FOREACH_SEQUENCE_EXPECTED                                                       \
    "struct zlang_sequence_i32_4_t values = { { 0, 0, 0, 0 }, 0U };"
#define ZLANG_SPEC_FOREACH_LOOP_EXPECTED                                                           \
    "for (uint32_t index = 0U; index < values.length_; ++index)"
#define ZLANG_SPEC_FOREACH_ITEM_EXPECTED "int32_t item = values.data_[index];"
#define ZLANG_SPEC_FOREACH_CALLBACK "zlang_for_each"

#define ZLANG_SPEC_UNBOUNDED_SOURCE "integer values[];"
#define ZLANG_SPEC_UNBOUNDED_EXPECTED_CODE 3001U

#define ZLANG_SPEC_STATIC_SEQUENCE_SOURCE "integer values[32];"
#define ZLANG_SPEC_STATIC_SEQUENCE_EXPECTED "static struct zlang_sequence_i32_32_t values ="
#define ZLANG_SPEC_ADD_SOURCE "integer total = 1 + 2;"
#define ZLANG_SPEC_ADD_EXPECTED "int32_t total = zlang_add_i32(1, 2);"

#define ZLANG_SPEC_IF_SOURCE                                                                       \
    "boolean ready = true;\n"                                                                      \
    "if (ready) {\n"                                                                               \
    "    print(\"yes\");\n"                                                                        \
    "} else {\n"                                                                                   \
    "    print(\"no\");\n"                                                                         \
    "}"
#define ZLANG_SPEC_IF_EXPECTED "if (ready) {"
#define ZLANG_SPEC_IF_ELSE_EXPECTED "} else {"

#define ZLANG_SPEC_REPEAT_SOURCE "repeat (2U) { print(\"tick\"); }"
#define ZLANG_SPEC_REPEAT_EXPECTED "for (uint32_t _repeat = 0U; _repeat < 2U; ++_repeat) {"

#define ZLANG_SPEC_FUNCTION_SOURCE                                                                 \
    "function add(integer left, integer right): integer {\n"                                       \
    "    return left + right;\n"                                                                   \
    "}"
#define ZLANG_SPEC_FUNCTION_EXPECTED "int32_t add(int32_t left, int32_t right) {"
#define ZLANG_SPEC_FUNCTION_RETURN_EXPECTED "return zlang_add_i32(left, right);"

#define ZLANG_SPEC_APPEND_INDEX_SOURCE                                                             \
    "integer values[4];\n"                                                                         \
    "append(values, 7);\n"                                                                         \
    "integer first = values[0];"
#define ZLANG_SPEC_APPEND_EXPECTED "zlang_array_append(values.data_, &values.length_, 4U"
#define ZLANG_SPEC_INDEX_EXPECTED "values.data_[0]"

#define ZLANG_SPEC_CONFORMANCE_SOURCE                                                              \
    "integer first = 1;\n"                                                                         \
    "unsigned count = 2U;\n"                                                                       \
    "boolean ready = false;\n"                                                                     \
    "string label = \"text\";\n"                                                                   \
    "integer values[4];\n"                                                                         \
    "{ integer scoped = 3; }\n"                                                                    \
    "if (ready) { print(\"yes\"); }\n"                                                             \
    "while (ready) { }\n"                                                                          \
    "foreach (item in values) { print(label); }\n"                                                 \
    "repeat (2U) { }\n"                                                                            \
    "function noop() { return; }\n"                                                                \
    "first = 4;\n"                                                                                 \
    "boolean matched = first == 4;\n"

#define ZLANG_SPEC_CONFORMANCE_INTEGER_EXPECTED "int32_t first = 1;"
#define ZLANG_SPEC_CONFORMANCE_UNSIGNED_EXPECTED "uint32_t count = 2U;"
#define ZLANG_SPEC_CONFORMANCE_BOOLEAN_EXPECTED "bool ready = false;"
#define ZLANG_SPEC_CONFORMANCE_STRING_EXPECTED                                                     \
    "zlang_string_t label = (zlang_string_t){ \"text\", 4U };"
#define ZLANG_SPEC_CONFORMANCE_BLOCK_EXPECTED "{\n        int32_t scoped = 3;"
#define ZLANG_SPEC_CONFORMANCE_IF_EXPECTED "if (ready) {"
#define ZLANG_SPEC_CONFORMANCE_WHILE_EXPECTED "while (ready) {"
#define ZLANG_SPEC_CONFORMANCE_FOREACH_EXPECTED                                                    \
    "for (uint32_t index = 0U; index < values.length_; ++index)"
#define ZLANG_SPEC_CONFORMANCE_REPEAT_EXPECTED "for (uint32_t _repeat = 0U; _repeat < 2U;"
#define ZLANG_SPEC_CONFORMANCE_FUNCTION_EXPECTED "void noop() {"
#define ZLANG_SPEC_CONFORMANCE_RETURN_EXPECTED "return;"
#define ZLANG_SPEC_CONFORMANCE_ASSIGNMENT_EXPECTED "first = 4;"
#define ZLANG_SPEC_CONFORMANCE_EQUAL_EXPECTED "(first == 4)"
#define ZLANG_SPEC_CONFORMANCE_IDENTIFIER_EXPECTED "zlang_print(label);"

#define ZLANG_SPEC_TEST_STACK_LIMIT 64U

#endif /* ZLANG_SPEC_FIXTURES_H */
