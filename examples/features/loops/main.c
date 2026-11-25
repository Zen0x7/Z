#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_U32

#define ZLANG_STD_IO_WRITE_I32

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated capacities */
#define ZLANG_VALUES_CAPACITY 4U
#define ZLANG_VALUES_LENGTH ZLANG_VALUES_EXTENT_0
#define ZLANG_VALUES_EXTENT_0 4U


/* generated sequence types */
typedef struct app_array_values_t {
    int32_t data_[ZLANG_VALUES_CAPACITY];
} app_array_values_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = ":";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = " ";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };
#define ZLANG_LITERAL_2_LENGTH 1U
static const char zlang_gen_literal_2_data[] = "\n";
static const zlang_string_t zlang_gen_literal_2 = { zlang_gen_literal_2_data, ZLANG_LITERAL_2_LENGTH };

/* generated function prototypes */

int32_t app_fn_main(void);

int32_t app_fn_main(void) {
    int32_t app_local_total = 0;

    while (app_local_total < 2) {
        app_local_total = zlang_add_i32(app_local_total, 1);
    }

    for (uint32_t zlang_gen_repeat_3 = ZLANG_ZERO; zlang_gen_repeat_3 < 2U; ++zlang_gen_repeat_3) {
        app_local_total = zlang_add_i32(app_local_total, 1);
    }

    for (int32_t app_local_index = 0; (app_local_index < 3); app_local_index = zlang_add_i32(app_local_index, 1)) {
        if (app_local_index == 1) {
            continue;
        }

        app_local_total = zlang_add_i32(app_local_total, app_local_index);
    }

    app_array_values_t app_local_values = { { 1, 2, 3, 4 } };

    for (uint32_t zlang_gen_foreach_4 = ZLANG_ZERO; zlang_gen_foreach_4 < 4; ++zlang_gen_foreach_4) {
        int32_t app_local_value = app_local_values.data_[zlang_gen_foreach_4];
        if (app_local_value == 3) {
            break;
        }

        app_local_total = zlang_add_i32(app_local_total, app_local_value);
    }

    for (uint32_t zlang_gen_foreach_5 = ZLANG_ZERO; zlang_gen_foreach_5 < 4; ++zlang_gen_foreach_5) {
        uint32_t app_local_index = zlang_gen_foreach_5;
        int32_t app_local_item = app_local_values.data_[zlang_gen_foreach_5];
        zlang_std_io_write_u32(app_local_index);
        zlang_std_io_write_string(zlang_gen_literal_0);
        zlang_std_io_write_i32(app_local_item);
        zlang_std_io_write_string(zlang_gen_literal_1);
    }

    zlang_std_io_write_i32(app_local_total);
    zlang_std_io_write_string(zlang_gen_literal_2);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
