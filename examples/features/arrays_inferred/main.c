#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated array helpers */
static inline uint32_t _zlang_array_offset_2(uint32_t extent_0, uint32_t extent_1, uint32_t index_0, uint32_t index_1) {
    if (index_0 >= extent_0 || index_1 >= extent_1) {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }
    return extent_1 * index_0 + index_1;
}

static inline void _zlang_array_set_2_i32(int32_t *data_, uint32_t extent_0, uint32_t extent_1, uint32_t index_0, uint32_t index_1, int32_t value_) {
    data_[_zlang_array_offset_2(extent_0, extent_1, index_0, index_1)] = value_;
}

static inline int32_t _zlang_array_get_2_i32(const int32_t *data_, uint32_t extent_0, uint32_t extent_1, uint32_t index_0, uint32_t index_1) {
    return data_[_zlang_array_offset_2(extent_0, extent_1, index_0, index_1)];
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
typedef struct zlang_array_i32_3_t {
    int32_t data_[ZLANG_FLAT_CAPACITY];
} zlang_array_i32_3_t;

typedef struct zlang_grid_t {
    int32_t data_[ZLANG_GRID_CAPACITY];
} zlang_grid_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = " ";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char _zlang_literal_1_data[] = "\n";
static const zlang_string_t _zlang_literal_1 = { _zlang_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int main(void) {
    zlang_array_i32_3_t _flat = { { 1, 2, 3 } };
    zlang_grid_t _grid = { { 1, 2, 3, 4 } };
    uint32_t _flat_length = 3;
    uint32_t _grid_length = 2;

    _zlang_array_set_2_i32(_grid.data_, 2, 2, 1, 1, zlang_add_i32(zlang_array_get_i32(_flat.data_, 2, 3), _zlang_array_get_2_i32(_grid.data_, 2, 2, 0, 1)));

    zlang_print_u32(_flat_length);
    zlang_print(_zlang_literal_0);
    zlang_print_u32(_grid_length);
    zlang_print(_zlang_literal_0);
    zlang_print_i32(_zlang_array_get_2_i32(_grid.data_, 2, 2, 1, 1));
    zlang_print(_zlang_literal_1);

    return ZLANG_ZERO;
}
