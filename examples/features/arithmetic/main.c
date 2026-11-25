#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_I32

#define ZLANG_STD_IO_WRITE_U32

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = " ";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

/* generated function prototypes */

int32_t app_fn_main(void);

int32_t app_fn_main(void) {
    int32_t app_local_left = 10;
    int32_t app_local_right = 3;
    uint32_t app_local_count = 5U;
    int32_t app_local_sum = zlang_add_i32(app_local_left, app_local_right);
    int32_t app_local_difference = zlang_sub_i32(app_local_left, app_local_right);
    int32_t app_local_product = zlang_mul_i32(app_local_left, app_local_right);
    int32_t app_local_quotient = zlang_div_i32(app_local_left, app_local_right);
    int32_t app_local_remainder = zlang_mod_i32(app_local_left, app_local_right);
    uint32_t app_local_grown = zlang_add_u32(app_local_count, 2U);
    int32_t app_local_negative = zlang_neg_i32(app_local_sum);
    app_local_left = zlang_add_i32(app_local_left, app_local_right);
    app_local_left = zlang_sub_i32(app_local_left, app_local_right);
    app_local_left = zlang_mul_i32(app_local_left, app_local_right);
    app_local_left = zlang_div_i32(app_local_left, app_local_right);
    app_local_left = zlang_mod_i32(app_local_left, app_local_right);
    zlang_std_io_write_i32(app_local_sum);
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_i32(app_local_difference);
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_i32(app_local_product);
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_i32(app_local_quotient);
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_i32(app_local_remainder);
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_u32(app_local_grown);
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_i32(app_local_negative);
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_i32(app_local_left);
    zlang_std_io_write_string(zlang_gen_literal_1);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
