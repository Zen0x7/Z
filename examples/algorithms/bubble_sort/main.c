#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated capacities */
#define ZLANG_VALUES_CAPACITY 6U
#define ZLANG_VALUES_LENGTH ZLANG_VALUES_EXTENT_0
#define ZLANG_VALUES_EXTENT_0 6U


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
    app_array_values_t app_local_values = { { 5, 1, 4, 2, 8, 3 } };

    for (int32_t app_local_pass = 0; (app_local_pass < 5); app_local_pass = zlang_add_i32(app_local_pass, 1)) {
        for (int32_t app_local_index = 0; (app_local_index < zlang_sub_i32(5, app_local_pass)); app_local_index = zlang_add_i32(app_local_index, 1)) {

            int32_t zlang_gen_value_2 = zlang_array_get_i32(app_local_values.data_, (uint32_t)app_local_index, 6);

            int32_t zlang_gen_value_3 = zlang_array_get_i32(app_local_values.data_, (uint32_t)zlang_add_i32(app_local_index, 1), 6);
            if (zlang_gen_value_2 > zlang_gen_value_3) {
                int32_t app_local_temporary = zlang_gen_value_2;

                uint32_t zlang_gen_index_4 = (uint32_t)app_local_index;
                if (zlang_gen_index_4 < 6) {
                    app_local_values.data_[zlang_gen_index_4] = zlang_array_get_i32(app_local_values.data_, (uint32_t)zlang_add_i32(app_local_index, 1), 6);
                } else {
                    zlang_trap(ZLANG_TRAP_BOUNDS);
                }

                uint32_t zlang_gen_index_5 = (uint32_t)zlang_add_i32(app_local_index, 1);
                if (zlang_gen_index_5 < 6) {
                    app_local_values.data_[zlang_gen_index_5] = app_local_temporary;
                } else {
                    zlang_trap(ZLANG_TRAP_BOUNDS);
                }
            }
        }
    }

    for (uint32_t zlang_gen_foreach_6 = ZLANG_ZERO; zlang_gen_foreach_6 < 6; ++zlang_gen_foreach_6) {
        int32_t app_local_value = app_local_values.data_[zlang_gen_foreach_6];
        zlang_print_i32(app_local_value);
        zlang_print(zlang_gen_literal_0);
    }

    zlang_print(zlang_gen_literal_1);

    return ZLANG_ZERO;
}
