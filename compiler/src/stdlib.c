/** @file
 * @brief Static standard library registry.
 */

#include "zlang/compiler/stdlib.h"

#include <string.h>

static const zlang_std_function_t functions_k[] = {{"random",
                                                    "generate_int32",
                                                    "integer",
                                                    "zlang_std_random_generate_int32",
                                                    "ZLANG_STD_RANDOM_GENERATE_INT32",
                                                    2U,
                                                    {"integer", "integer"},
                                                    true,
                                                    true,
                                                    {false, false}},
                                                   {"random",
                                                    "generate_uint32",
                                                    "unsigned",
                                                    "zlang_std_random_generate_uint32",
                                                    "ZLANG_STD_RANDOM_GENERATE_UINT32",
                                                    2U,
                                                    {"unsigned", "unsigned"},
                                                    true,
                                                    true,
                                                    {false, false}},
                                                   {"random",
                                                    "generate_int64",
                                                    "int64",
                                                    "zlang_std_random_generate_int64",
                                                    "ZLANG_STD_RANDOM_GENERATE_INT64",
                                                    2U,
                                                    {"int64", "int64"},
                                                    true,
                                                    true,
                                                    {false, false}},
                                                   {"random",
                                                    "generate_uint64",
                                                    "uint64",
                                                    "zlang_std_random_generate_uint64",
                                                    "ZLANG_STD_RANDOM_GENERATE_UINT64",
                                                    2U,
                                                    {"uint64", "uint64"},
                                                    true,
                                                    true,
                                                    {false, false}},
                                                   {"random",
                                                    "generate_float32",
                                                    "float",
                                                    "zlang_std_random_generate_float32",
                                                    "ZLANG_STD_RANDOM_GENERATE_FLOAT32",
                                                    2U,
                                                    {"float", "float"},
                                                    true,
                                                    true,
                                                    {false, false}},
                                                   {"random",
                                                    "generate_float64",
                                                    "double",
                                                    "zlang_std_random_generate_float64",
                                                    "ZLANG_STD_RANDOM_GENERATE_FLOAT64",
                                                    2U,
                                                    {"double", "double"},
                                                    true,
                                                    true,
                                                    {false, false}},
                                                   {"program_options",
                                                    "argument_count",
                                                    "unsigned",
                                                    "zlang_std_program_options_argument_count",
                                                    "ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT",
                                                    1U,
                                                    {"arguments"},
                                                    false,
                                                    true,
                                                    {true}},
                                                   {"program_options",
                                                    "argument_at",
                                                    "string",
                                                    "zlang_std_program_options_argument_at",
                                                    "ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_AT",
                                                    2U,
                                                    {"arguments", "unsigned"},
                                                    false,
                                                    true,
                                                    {true, false}},
                                                   {"program_options",
                                                    "has_option",
                                                    "boolean",
                                                    "zlang_std_program_options_has_option",
                                                    "ZLANG_STD_PROGRAM_OPTIONS_HAS_OPTION",
                                                    2U,
                                                    {"arguments", "string"},
                                                    false,
                                                    true,
                                                    {true, false}},
                                                   {"program_options",
                                                    "option_value",
                                                    "string",
                                                    "zlang_std_program_options_option_value",
                                                    "ZLANG_STD_PROGRAM_OPTIONS_OPTION_VALUE",
                                                    2U,
                                                    {"arguments", "string"},
                                                    false,
                                                    true,
                                                    {true, false}},
                                                   {"program_options",
                                                    "has_flag",
                                                    "boolean",
                                                    "zlang_std_program_options_has_flag",
                                                    "ZLANG_STD_PROGRAM_OPTIONS_HAS_FLAG",
                                                    2U,
                                                    {"arguments", "string"},
                                                    false,
                                                    true,
                                                    {true, false}}};

const zlang_std_function_t *zlang_std_find(const char *domain, const char *name) {
    if (domain == NULL || name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < sizeof(functions_k) / sizeof(functions_k[0]); ++_index) {
        const zlang_std_function_t *const _entry_k = &functions_k[_index];
        if (strcmp(_entry_k->domain_, domain) == 0 && strcmp(_entry_k->name_, name) == 0) {
            return _entry_k;
        }
    }
    return NULL;
}

const zlang_std_function_t *zlang_std_find_by_name(const char *name) {
    if (name == NULL) {
        return NULL;
    }
    for (size_t _index = 0U; _index < sizeof(functions_k) / sizeof(functions_k[0]); ++_index) {
        if (strcmp(functions_k[_index].name_, name) == 0) {
            return &functions_k[_index];
        }
    }
    return NULL;
}
