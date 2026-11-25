#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_I32

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 8U
static const char zlang_gen_literal_0_data[] = "fizzbuzz";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 4U
static const char zlang_gen_literal_1_data[] = "fizz";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };
#define ZLANG_LITERAL_2_LENGTH 4U
static const char zlang_gen_literal_2_data[] = "buzz";
static const zlang_string_t zlang_gen_literal_2 = { zlang_gen_literal_2_data, ZLANG_LITERAL_2_LENGTH };
#define ZLANG_LITERAL_3_LENGTH 1U
static const char zlang_gen_literal_3_data[] = "\n";
static const zlang_string_t zlang_gen_literal_3 = { zlang_gen_literal_3_data, ZLANG_LITERAL_3_LENGTH };

/* generated function prototypes */

int32_t app_fn_main(void);

int32_t app_fn_main(void) {
    for (int32_t app_local_value = 1; (app_local_value <= 15); app_local_value = zlang_add_i32(app_local_value, 1)) {
        if (zlang_mod_i32(app_local_value, 15) == 0) {
            zlang_std_io_write_string(zlang_gen_literal_0);
        } else if (zlang_mod_i32(app_local_value, 3) == 0) {
            zlang_std_io_write_string(zlang_gen_literal_1);
        } else if (zlang_mod_i32(app_local_value, 5) == 0) {
            zlang_std_io_write_string(zlang_gen_literal_2);
        } else {
            zlang_std_io_write_i32(app_local_value);
        }

        zlang_std_io_write_string(zlang_gen_literal_3);
    }

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
