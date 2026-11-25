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
    int32_t _factorial = 1;

    for (int32_t _factor = 2; (_factor <= 6); _factor = zlang_add_i32(_factor, 1)) {
        _factorial = zlang_mul_i32(_factorial, _factor);
    }

    zlang_print_i32(_factorial);
    zlang_print(_zlang_literal_0);

    return ZLANG_ZERO;
}
