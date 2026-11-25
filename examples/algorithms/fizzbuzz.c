#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 8U
static const char _zlang_literal_0_data[] = "fizzbuzz";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 4U
static const char _zlang_literal_1_data[] = "fizz";
static const zlang_string_t _zlang_literal_1 = { _zlang_literal_1_data, ZLANG_LITERAL_1_LENGTH };
#define ZLANG_LITERAL_2_LENGTH 4U
static const char _zlang_literal_2_data[] = "buzz";
static const zlang_string_t _zlang_literal_2 = { _zlang_literal_2_data, ZLANG_LITERAL_2_LENGTH };
#define ZLANG_LITERAL_3_LENGTH 1U
static const char _zlang_literal_3_data[] = "\n";
static const zlang_string_t _zlang_literal_3 = { _zlang_literal_3_data, ZLANG_LITERAL_3_LENGTH };

int main(void) {
    for (int32_t _value = 1; (_value <= 15); _value = zlang_add_i32(_value, 1)) {
        if (zlang_mod_i32(_value, 15) == 0) {
            zlang_print(_zlang_literal_0);
        } else if (zlang_mod_i32(_value, 3) == 0) {
            zlang_print(_zlang_literal_1);
        } else if (zlang_mod_i32(_value, 5) == 0) {
            zlang_print(_zlang_literal_2);
        } else {
            zlang_print_i32(_value);
        }

        zlang_print(_zlang_literal_3);
    }

    return ZLANG_ZERO;
}
