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
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = "\n";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };

/* generated function prototypes */

int32_t app_fn_main(void);

int32_t app_fn_main(void) {
    int32_t app_local_value = 27;
    int32_t app_local_steps = 0;

    while (app_local_value != 1) {
        if (zlang_mod_i32(app_local_value, 2) == 0) {
            app_local_value = zlang_div_i32(app_local_value, 2);
        } else {
            app_local_value = zlang_add_i32(zlang_mul_i32(3, app_local_value), 1);
        }

        app_local_steps = zlang_add_i32(app_local_steps, 1);
    }

    zlang_std_io_write_i32(app_local_steps);
    zlang_std_io_write_string(zlang_gen_literal_0);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
