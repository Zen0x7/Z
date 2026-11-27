#define ZLANG_STD_RESULT

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
static const char zlang_gen_literal_0_data[] = " ";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

/* generated function prototypes */

zlang_std_result_i32_status_t app_fn_query(void);
int32_t app_fn_main(void);

zlang_std_result_i32_status_t app_fn_query(void) {
    zlang_std_result_i32_status_t app_local_result = { ZLANG_ZERO };
    app_local_result.value_ = 7;

    return app_local_result;
}

int32_t app_fn_main(void) {
    zlang_std_result_i32_status_t app_local_value = app_fn_query();
    zlang_std_result_i32_status_t app_local_copied = app_fn_query();
    zlang_std_io_write_i32(app_local_value.value_);
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_i32(app_local_copied.value_);
    zlang_std_io_write_string(zlang_gen_literal_1);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
