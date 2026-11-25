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

static inline int32_t zlang_gen_array_get_2_i32(const int32_t *data_, uint32_t extent_0, uint32_t extent_1, uint32_t index_0, uint32_t index_1) {
    return data_[zlang_gen_array_offset_2(extent_0, extent_1, index_0, index_1)];
}

/* generated capacities */
#define ZLANG_RESULT_CAPACITY 6U
#define ZLANG_RESULT_LENGTH ZLANG_RESULT_EXTENT_0
#define ZLANG_RESULT_EXTENT_0 2U
#define ZLANG_RESULT_EXTENT_1 3U
#define ZLANG_GRID_CAPACITY 6U
#define ZLANG_GRID_LENGTH ZLANG_GRID_EXTENT_0
#define ZLANG_GRID_EXTENT_0 2U
#define ZLANG_GRID_EXTENT_1 3U
#define ZLANG_STAMPED_CAPACITY 6U
#define ZLANG_STAMPED_LENGTH ZLANG_STAMPED_EXTENT_0
#define ZLANG_STAMPED_EXTENT_0 2U
#define ZLANG_STAMPED_EXTENT_1 3U


/* generated sequence types */
typedef struct zlang_gen_array_i32_2_3_t {
    int32_t data_[ZLANG_RESULT_CAPACITY];
} zlang_gen_array_i32_2_3_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = "\n";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };

/* generated function prototypes */

zlang_gen_array_i32_2_3_t app_fn_stamp_2_3_i32(zlang_gen_array_i32_2_3_t app_param_source);

zlang_gen_array_i32_2_3_t app_fn_stamp_2_3_i32(zlang_gen_array_i32_2_3_t app_param_source) {
    zlang_gen_array_i32_2_3_t app_local_result = { { 0, 0, 0, 0, 0, 0 } };

    zlang_gen_array_set_2_i32(app_local_result.data_, 2, 3, 0, 1, zlang_add_i32(zlang_gen_array_get_2_i32(app_param_source.data_, 2, 3, 0, 1), 1));

    return app_local_result;
}

int main(void) {
    zlang_gen_array_i32_2_3_t app_local_grid = { { 0, 0, 0, 0, 0, 0 } };

    zlang_gen_array_set_2_i32(app_local_grid.data_, 2, 3, 0, 1, 5);

    zlang_gen_array_i32_2_3_t app_local_stamped = app_fn_stamp_2_3_i32(app_local_grid);
    zlang_std_io_write_i32(zlang_gen_array_get_2_i32(app_local_stamped.data_, 2, 3, 0, 1));
    zlang_std_io_write_string(zlang_gen_literal_0);

    return ZLANG_ZERO;
}
