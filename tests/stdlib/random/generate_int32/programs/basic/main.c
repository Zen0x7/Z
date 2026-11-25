#define ZLANG_STD_RANDOM_GENERATE_INT32

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = "\n";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };

int main(void) {
    zlang_runtime_context_t _zlang_context = { 0x6D2B79F5U };

    int32_t app_local_value = zlang_std_random_generate_int32(&_zlang_context, 1, 1);
    zlang_print_i32(app_local_value);
    zlang_print(zlang_gen_literal_0);

    return ZLANG_ZERO;
}
