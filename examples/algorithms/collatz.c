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
    int32_t _value = 27;
    int32_t _steps = 0;

    while (_value != 1) {
        if (zlang_mod_i32(_value, 2) == 0) {
            _value = zlang_div_i32(_value, 2);
        } else {
            _value = zlang_add_i32(zlang_mul_i32(3, _value), 1);
        }

        _steps = zlang_add_i32(_steps, 1);
    }

    zlang_print_i32(_steps);
    zlang_print(_zlang_literal_0);

    return ZLANG_ZERO;
}
