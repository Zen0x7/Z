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

/* generated function prototypes */

int32_t app_fn_main(void);

int32_t app_fn_main(void) {
    app_array_fib_t app_local_fib = { { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 } };

    for (int32_t app_local_index = 2; (app_local_index < 10); app_local_index = zlang_add_i32(app_local_index, 1)) {
        int32_t zlang_gen_read_3 = app_local_fib.data_[(uint32_t)zlang_sub_i32(app_local_index, 1)];
        int32_t zlang_gen_read_4 = app_local_fib.data_[(uint32_t)zlang_sub_i32(app_local_index, 2)];
        app_local_fib.data_[(uint32_t)app_local_index] = zlang_add_i32(zlang_gen_read_3, zlang_gen_read_4);
    }

    for (uint32_t zlang_gen_foreach_5 = ZLANG_ZERO; zlang_gen_foreach_5 < 10; ++zlang_gen_foreach_5) {
        int32_t app_local_value = app_local_fib.data_[zlang_gen_foreach_5];
        zlang_std_io_write_i32(app_local_value);
        zlang_std_io_write_string(zlang_gen_literal_0);
    }

    zlang_std_io_write_string(zlang_gen_literal_1);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
