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
#define ZLANG_FLAT_CAPACITY 3U
#define ZLANG_FLAT_LENGTH ZLANG_FLAT_EXTENT_0
#define ZLANG_FLAT_EXTENT_0 3U
#define ZLANG_GRID_CAPACITY 4U
#define ZLANG_GRID_LENGTH ZLANG_GRID_EXTENT_0
#define ZLANG_GRID_EXTENT_0 2U
#define ZLANG_GRID_EXTENT_1 2U


/* generated sequence types */
typedef struct zlang_gen_array_i32_3_t {
    int32_t data_[ZLANG_FLAT_CAPACITY];
} zlang_gen_array_i32_3_t;

typedef struct app_array_grid_t {
    int32_t data_[ZLANG_GRID_CAPACITY];
} app_array_grid_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = " ";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int main(void) {
    zlang_gen_array_i32_3_t app_local_flat = { { 1, 2, 3 } };
    (void)app_local_flat;
    app_array_grid_t app_local_grid = { { 1, 2, 3, 4 } };
    uint32_t app_local_flat_length = 0U;
    uint32_t app_local_grid_length = 0U;

    zlang_gen_array_set_2_i32(app_local_grid.data_, 2, 2, 1, 1, zlang_add_i32(3, zlang_gen_array_get_2_i32(app_local_grid.data_, 2, 2, 0, 1)));

    zlang_print_u32(app_local_flat_length);
    zlang_print(zlang_gen_literal_0);
    zlang_print_u32(app_local_grid_length);
    zlang_print(zlang_gen_literal_0);
    zlang_print_i32(zlang_gen_array_get_2_i32(app_local_grid.data_, 2, 2, 1, 1));
    zlang_print(zlang_gen_literal_1);

    return ZLANG_ZERO;
}
