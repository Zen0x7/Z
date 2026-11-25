#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated capacities */
#define ZLANG_FIB_CAPACITY 10U
#define ZLANG_FIB_LENGTH ZLANG_FIB_EXTENT_0
#define ZLANG_FIB_EXTENT_0 10U


/* generated sequence types */
typedef struct app_array_fib_t {
    int32_t data_[ZLANG_FIB_CAPACITY];
} app_array_fib_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = " ";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int main(void) {
    app_array_fib_t app_local_fib = { { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } };

    for (int32_t app_local_index = 2; (app_local_index < 10); app_local_index = zlang_add_i32(app_local_index, 1)) {
        uint32_t zlang_gen_index_2 = (uint32_t)app_local_index;
        if (zlang_gen_index_2 < 10) {
            app_local_fib.data_[zlang_gen_index_2] = zlang_add_i32(zlang_array_get_i32(app_local_fib.data_, (uint32_t)zlang_sub_i32(app_local_index, 1), 10), zlang_array_get_i32(app_local_fib.data_, (uint32_t)zlang_sub_i32(app_local_index, 2), 10));
        } else {
            zlang_trap(ZLANG_TRAP_BOUNDS);
        }
    }

    for (uint32_t zlang_gen_foreach_3 = ZLANG_ZERO; zlang_gen_foreach_3 < 10; ++zlang_gen_foreach_3) {
        int32_t app_local_value = app_local_fib.data_[zlang_gen_foreach_3];
        zlang_print_i32(app_local_value);
        zlang_print(zlang_gen_literal_0);
    }

    zlang_print(zlang_gen_literal_1);

    return ZLANG_ZERO;
}
