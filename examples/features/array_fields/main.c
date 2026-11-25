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
#define ZLANG_CELLS_CAPACITY 6U
#define ZLANG_CELLS_LENGTH ZLANG_CELLS_EXTENT_0
#define ZLANG_CELLS_EXTENT_0 2U
#define ZLANG_CELLS_EXTENT_1 3U


/* generated sequence types */
typedef struct zlang_gen_array_i32_2_3_t {
    int32_t data_[ZLANG_CELLS_CAPACITY];
} zlang_gen_array_i32_2_3_t;

/* generated user types */
typedef struct app_record_matrix_t {
    zlang_gen_array_i32_2_3_t cells_;
} app_record_matrix_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = "\n";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };

int main(void) {
    app_record_matrix_t app_local_matrix = { ZLANG_ZERO };
    zlang_gen_array_set_2_i32(app_local_matrix.cells_.data_, 2, 3, 1, 2, 7);
    int32_t app_local_value = zlang_gen_array_get_2_i32(app_local_matrix.cells_.data_, 2, 3, 1, 2);
    zlang_std_io_write_i32(app_local_value);
    zlang_std_io_write_string(zlang_gen_literal_0);

    return ZLANG_ZERO;
}
