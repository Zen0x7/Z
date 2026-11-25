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

/* generated function prototypes */

int32_t _u_gcd(int32_t, int32_t);

int32_t _u_gcd(int32_t left, int32_t right) {
    while (right != 0) {
        int32_t _remainder = zlang_mod_i32(left, right);
        left = right;
        right = _remainder;
    }

    return left;
}

int main(void) {
    zlang_print_i32(_u_gcd(1071, 462));
    zlang_print(_zlang_literal_0);

    return ZLANG_ZERO;
}
