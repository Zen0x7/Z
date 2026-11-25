#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_I32

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated capacities */
#define ZLANG_VALUES_CAPACITY 5U
#define ZLANG_VALUES_LENGTH ZLANG_VALUES_EXTENT_0
#define ZLANG_VALUES_EXTENT_0 5U


/* generated sequence types */
typedef struct app_array_values_t {
    int32_t data_[ZLANG_VALUES_CAPACITY];
} app_array_values_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = " ";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int main(void) {
    app_array_values_t app_local_values = { { 4, 5, 0, 0, 0 } };
    int32_t app_local_capacity = 5;
    app_local_values.data_[0] = 6;
    app_local_values.data_[2] = zlang_mul_i32(zlang_array_get_i32(app_local_values.data_, 0, 5), 2);
    app_local_values.data_[3] = zlang_sub_i32(zlang_array_get_i32(app_local_values.data_, 2, 5), 1);
    app_local_values.data_[4] = zlang_add_i32(zlang_array_get_i32(app_local_values.data_, 3, 5), app_local_capacity);

    for (uint32_t zlang_gen_foreach_2 = ZLANG_ZERO; zlang_gen_foreach_2 < 5; ++zlang_gen_foreach_2) {
        int32_t app_local_value = app_local_values.data_[zlang_gen_foreach_2];
        zlang_std_io_write_i32(app_local_value);
        zlang_std_io_write_string(zlang_gen_literal_0);
    }

    zlang_std_io_write_string(zlang_gen_literal_1);

    return ZLANG_ZERO;
}
