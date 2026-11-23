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

#endif /* ZLANG_SPEC_FIXTURES_H */
