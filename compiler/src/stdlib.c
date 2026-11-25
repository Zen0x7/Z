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
                                                    {"integer", "integer"},
                                                    2U,
                                                    true,
                                                    true}};

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
