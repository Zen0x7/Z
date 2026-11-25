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

static inline int32_t zlang_gen_array_get_2_i32(const int32_t *data_, uint32_t extent_0, uint32_t extent_1, uint32_t index_0, uint32_t index_1) {
    return data_[zlang_gen_array_offset_2(extent_0, extent_1, index_0, index_1)];
}

static inline void zlang_gen_array_set_2_i32(int32_t *data_, uint32_t extent_0, uint32_t extent_1, uint32_t index_0, uint32_t index_1, int32_t value_) {
    data_[zlang_gen_array_offset_2(extent_0, extent_1, index_0, index_1)] = value_;
}

/* generated capacities */
#define ZLANG_LEFT_CAPACITY 4U
#define ZLANG_LEFT_LENGTH ZLANG_LEFT_EXTENT_0
#define ZLANG_LEFT_EXTENT_0 2U
#define ZLANG_LEFT_EXTENT_1 2U
#define ZLANG_RIGHT_CAPACITY 4U
#define ZLANG_RIGHT_LENGTH ZLANG_RIGHT_EXTENT_0
#define ZLANG_RIGHT_EXTENT_0 2U
#define ZLANG_RIGHT_EXTENT_1 2U
#define ZLANG_RESULT_CAPACITY 4U
#define ZLANG_RESULT_LENGTH ZLANG_RESULT_EXTENT_0
#define ZLANG_RESULT_EXTENT_0 2U
#define ZLANG_RESULT_EXTENT_1 2U


/* generated sequence types */
typedef struct app_array_left_t {
    int32_t data_[ZLANG_LEFT_CAPACITY];
} app_array_left_t;

typedef struct app_array_right_t {
    int32_t data_[ZLANG_RIGHT_CAPACITY];
} app_array_right_t;

typedef struct app_array_result_t {
    int32_t data_[ZLANG_RESULT_CAPACITY];
} app_array_result_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = " ";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int main(void) {
    app_array_left_t app_local_left = { { 1, 2, 3, 4 } };
    app_array_right_t app_local_right = { { 5, 6, 7, 8 } };
    app_array_result_t app_local_result = { { 0, 0, 0, 0 } };

    for (int32_t app_local_row = 0; (app_local_row < 2); app_local_row = zlang_add_i32(app_local_row, 1)) {
        for (int32_t app_local_column = 0; (app_local_column < 2); app_local_column = zlang_add_i32(app_local_column, 1)) {
            int32_t app_local_cell = 0;

            for (int32_t app_local_inner = 0; (app_local_inner < 2); app_local_inner = zlang_add_i32(app_local_inner, 1)) {
                app_local_cell = zlang_add_i32(app_local_cell, zlang_mul_i32(zlang_gen_array_get_2_i32(app_local_left.data_, 2, 2, (uint32_t)app_local_row, (uint32_t)app_local_inner), zlang_gen_array_get_2_i32(app_local_right.data_, 2, 2, (uint32_t)app_local_inner, (uint32_t)app_local_column)));
            }

            zlang_gen_array_set_2_i32(app_local_result.data_, 2, 2, (uint32_t)app_local_row, (uint32_t)app_local_column, app_local_cell);
        }
    }

    for (uint32_t app_local_row_3 = ZLANG_ZERO; app_local_row_3 < 2; ++app_local_row_3) {
        for (uint32_t app_local_column_4 = ZLANG_ZERO; app_local_column_4 < 2; ++app_local_column_4) {
            zlang_std_io_write_i32(zlang_gen_array_get_2_i32(app_local_result.data_, 2, 2, app_local_row_3, app_local_column_4));
            zlang_std_io_write_string(zlang_gen_literal_0);
        }
    }

    zlang_std_io_write_string(zlang_gen_literal_1);

    return ZLANG_ZERO;
}
