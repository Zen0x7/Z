#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = " ";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char _zlang_literal_1_data[] = "\n";
static const zlang_string_t _zlang_literal_1 = { _zlang_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int main(void) {
    int32_t _left = 10;
    int32_t _right = 3;
    uint32_t _count = 5U;
    int32_t _sum = zlang_add_i32(_left, _right);
    int32_t _difference = zlang_sub_i32(_left, _right);
    int32_t _product = zlang_mul_i32(_left, _right);
    int32_t _quotient = zlang_div_i32(_left, _right);
    int32_t _remainder = zlang_mod_i32(_left, _right);
    uint32_t _grown = zlang_add_u32(_count, 2U);
    int32_t _negative = (-_sum);
    _left = zlang_add_i32(_left, _right);
    _left = zlang_sub_i32(_left, _right);
    _left = zlang_mul_i32(_left, _right);
    _left = zlang_div_i32(_left, _right);
    _left = zlang_mod_i32(_left, _right);
    zlang_print_i32(_sum);
    zlang_print(_zlang_literal_0);
    zlang_print_i32(_difference);
    zlang_print(_zlang_literal_0);
    zlang_print_i32(_product);
    zlang_print(_zlang_literal_0);
    zlang_print_i32(_quotient);
    zlang_print(_zlang_literal_0);
    zlang_print_i32(_remainder);
    zlang_print(_zlang_literal_0);
    zlang_print_u32(_grown);
    zlang_print(_zlang_literal_0);
    zlang_print_i32(_negative);
    zlang_print(_zlang_literal_0);
    zlang_print_i32(_left);
    zlang_print(_zlang_literal_1);

    return ZLANG_ZERO;
}
