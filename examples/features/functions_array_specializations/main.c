#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_I32

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated array helpers */
static inline uint32_t zlang_gen_array_offset_2(uint32_t extent_0, uint32_t extent_1, uint32_t index_0, uint32_t index_1) {
    if (index_0 >= extent_0 || index_1 >= extent_1) {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }
    return extent_1 * index_0 + index_1;
}

static inline void zlang_gen_array_set_2_i32(int32_t *data_, uint32_t extent_0, uint32_t extent_1, uint32_t index_0, uint32_t index_1, int32_t value_) {
    data_[zlang_gen_array_offset_2(extent_0, extent_1, index_0, index_1)] = value_;
}

/* generated capacities */
#define ZLANG_GRID_CAPACITY 6U
#define ZLANG_GRID_LENGTH ZLANG_GRID_EXTENT_0
#define ZLANG_GRID_EXTENT_0 2U
#define ZLANG_GRID_EXTENT_1 3U
#define ZLANG_LINE_CAPACITY 4U
#define ZLANG_LINE_LENGTH ZLANG_LINE_EXTENT_0
#define ZLANG_LINE_EXTENT_0 4U


/* generated sequence types */
typedef struct zlang_gen_array_i32_2_3_t {
    int32_t data_[ZLANG_GRID_CAPACITY];
} zlang_gen_array_i32_2_3_t;

typedef struct zlang_gen_array_i32_4_t {
    int32_t data_[ZLANG_LINE_CAPACITY];
} zlang_gen_array_i32_4_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = " ";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

/* generated function prototypes */

int32_t app_fn_first_2_3_i32(zlang_gen_array_i32_2_3_t app_param_source);
int32_t app_fn_first_4_i32(zlang_gen_array_i32_4_t app_param_source);
int32_t app_fn_main(void);

int32_t app_fn_first_2_3_i32(zlang_gen_array_i32_2_3_t app_param_source) {
    return zlang_array_get_i32(app_param_source.data_, 0, 2);
}

int32_t app_fn_first_4_i32(zlang_gen_array_i32_4_t app_param_source_2) {
    return zlang_array_get_i32(app_param_source_2.data_, 0, 4);
}

int32_t app_fn_main(void) {
    zlang_gen_array_i32_2_3_t app_local_grid = { { 0, 0, 0, 0, 0, 0 } };

    zlang_gen_array_set_2_i32(app_local_grid.data_, 2, 3, 0, 0, 7);

    zlang_gen_array_i32_4_t app_local_line = { { 0, 0, 0, 0 } };
    app_local_line.data_[0] = 9;
    zlang_std_io_write_i32(app_fn_first_2_3_i32(app_local_grid));
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_i32(app_fn_first_4_i32(app_local_line));
    zlang_std_io_write_string(zlang_gen_literal_1);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
