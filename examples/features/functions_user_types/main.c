#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_I32

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated user types */
typedef struct app_struct_point_t {
    int32_t x_;
} app_struct_point_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = "\n";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };

/* generated function prototypes */

app_struct_point_t app_fn_make(void);
void app_fn_shift(app_struct_point_t *app_param_point, int32_t app_param_amount);
int32_t app_fn_main(void);

app_struct_point_t app_fn_make(void) {
    app_struct_point_t app_local_result = { ZLANG_ZERO };
    app_local_result.x_ = 1;

    return app_local_result;
}

void app_fn_shift(app_struct_point_t *app_param_point, int32_t app_param_amount) {
    app_param_point->x_ = zlang_add_i32(app_param_point->x_, app_param_amount);
}

int32_t app_fn_main(void) {
    app_struct_point_t app_local_point = app_fn_make();
    app_fn_shift(&app_local_point, 4);
    zlang_std_io_write_i32(app_local_point.x_);
    zlang_std_io_write_string(zlang_gen_literal_0);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
