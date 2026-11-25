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

int32_t app_fn_m_lib_math_z_add(int32_t app_param_value);
int32_t app_fn_main(void);

static int32_t app_local_m_lib_config_z_base = 40;
int32_t app_fn_m_lib_math_z_add(int32_t app_param_value) {
    return zlang_add_i32(app_param_value, 2);
}

int32_t app_fn_main(void) {
    zlang_std_io_write_i32(app_fn_m_lib_math_z_add(app_local_m_lib_config_z_base));
    zlang_std_io_write_string(zlang_gen_literal_0);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
