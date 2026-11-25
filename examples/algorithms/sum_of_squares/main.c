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
static const char zlang_gen_literal_0_data[] = "\n";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };

int main(void) {
    app_array_values_t app_local_values = { { 1, 2, 3, 4, 5 } };
    int32_t app_local_total = 0;

    for (uint32_t zlang_gen_foreach_1 = ZLANG_ZERO; zlang_gen_foreach_1 < 5; ++zlang_gen_foreach_1) {
        int32_t app_local_value = app_local_values.data_[zlang_gen_foreach_1];
        app_local_total = zlang_add_i32(app_local_total, zlang_mul_i32(app_local_value, app_local_value));
    }

    zlang_std_io_write_i32(app_local_total);
    zlang_std_io_write_string(zlang_gen_literal_0);

    return ZLANG_ZERO;
}
