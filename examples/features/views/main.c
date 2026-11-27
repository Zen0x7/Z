#define ZLANG_STD_VIEW

#define ZLANG_STD_RESULT

#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_I32

#define ZLANG_STD_VIEW

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated capacities */
#define ZLANG_DATA_CAPACITY 6U
#define ZLANG_DATA_LENGTH ZLANG_DATA_EXTENT_0
#define ZLANG_DATA_EXTENT_0 6U


/* generated sequence types */
typedef struct app_array_data_t {
    int32_t data_[ZLANG_DATA_CAPACITY];
} app_array_data_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = "\n";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };

/* generated function prototypes */

int32_t app_fn_sum(zlang_view_t app_param_values);
int32_t app_fn_main(void);

int32_t app_fn_sum(zlang_view_t app_param_values) {
    return zlang_add_i32((*(int32_t *)zlang_array_get(app_param_values.data_, 0, app_param_values.length_, app_param_values.element_size_)), (*(int32_t *)zlang_array_get(app_param_values.data_, 1, app_param_values.length_, app_param_values.element_size_)));
}

int32_t app_fn_main(void) {
    app_array_data_t app_local_data = { { 0, 0, 0, 0, 0, 0 } };
    zlang_view_t app_local_window = zlang_view_make(app_local_data.data_, 6, 2U, 4U, sizeof(app_local_data.data_[0]));

        (*(int32_t *)zlang_array_get(app_local_window.data_, 0, app_local_window.length_, app_local_window.element_size_)) = 3;

        (*(int32_t *)zlang_array_get(app_local_window.data_, 1, app_local_window.length_, app_local_window.element_size_)) = 4;

    zlang_std_io_write_i32(app_fn_sum(app_local_window));
    zlang_std_io_write_string(zlang_gen_literal_0);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
