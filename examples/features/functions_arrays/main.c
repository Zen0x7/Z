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
typedef struct zlang_array_i32_2_3_t {
    int32_t data_[ZLANG_RESULT_CAPACITY];
} zlang_array_i32_2_3_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = "\n";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };

/* generated function prototypes */

zlang_array_i32_2_3_t _u_stamp_2_3_i32(zlang_array_i32_2_3_t);

zlang_array_i32_2_3_t _u_stamp_2_3_i32(zlang_array_i32_2_3_t source) {
    zlang_array_i32_2_3_t _result = { { 0, 0, 0, 0, 0, 0 } };

    _zlang_array_set_2_i32(_result.data_, 2, 3, 0, 1, zlang_add_i32(_zlang_array_get_2_i32(source.data_, 2, 3, 0, 1), 1));

    return _result;
}

int main(void) {
    zlang_array_i32_2_3_t _grid = { { 0, 0, 0, 0, 0, 0 } };

    _zlang_array_set_2_i32(_grid.data_, 2, 3, 0, 1, 5);

    zlang_array_i32_2_3_t _stamped = _u_stamp_2_3_i32(_grid);
    zlang_print_i32(_zlang_array_get_2_i32(_stamped.data_, 2, 3, 0, 1));
    zlang_print(_zlang_literal_0);

    return ZLANG_ZERO;
}
