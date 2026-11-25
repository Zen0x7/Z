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
typedef struct app_record_point_t {
    int32_t x_;
} app_record_point_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = " ";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

/* generated function prototypes */

int32_t app_fn_read(const app_record_point_t *app_param_point);
int32_t app_fn_copy_value(const int32_t app_param_value);
int32_t app_fn_main(void);

int32_t app_fn_read(const app_record_point_t *app_param_point) {
    return app_param_point->x_;
}

int32_t app_fn_copy_value(const int32_t app_param_value) {
    return app_param_value;
}

int32_t app_fn_main(void) {
    app_record_point_t app_local_point = { ZLANG_ZERO };
    app_local_point.x_ = 6;
    zlang_std_io_write_i32(app_fn_read(&app_local_point));
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_i32(app_fn_copy_value(app_local_point.x_));
    zlang_std_io_write_string(zlang_gen_literal_1);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
