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
static const char _zlang_literal_0_data[] = "\n";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };

int main(void) {
    zlang_std_random_state_t _zlang_std_state = { 0x6D2B79F5U };

    int32_t _value = zlang_std_random_generate_int32(&_zlang_std_state, 1, 1);
    zlang_print_i32(_value);
    zlang_print(_zlang_literal_0);

    return ZLANG_ZERO;
}
