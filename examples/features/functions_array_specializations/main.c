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

static inline int32_t _zlang_array_get_2_i32(const int32_t *data_, uint32_t extent_0, uint32_t extent_1, uint32_t index_0, uint32_t index_1) {
    return data_[_zlang_array_offset_2(extent_0, extent_1, index_0, index_1)];
}

static inline void _zlang_array_set_2_i32(int32_t *data_, uint32_t extent_0, uint32_t extent_1, uint32_t index_0, uint32_t index_1, int32_t value_) {
    data_[_zlang_array_offset_2(extent_0, extent_1, index_0, index_1)] = value_;
}

static void _zlang_array_helpers_keep(void) {
    (void)_zlang_array_offset_2;
    (void)_zlang_array_get_2_i32;
    (void)_zlang_array_set_2_i32;
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
typedef struct zlang_array_i32_2_3_t {
    int32_t data_[ZLANG_GRID_CAPACITY];
} zlang_array_i32_2_3_t;

typedef struct zlang_array_i32_4_t {
    int32_t data_[ZLANG_LINE_CAPACITY];
} zlang_array_i32_4_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = " ";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char _zlang_literal_1_data[] = "\n";
static const zlang_string_t _zlang_literal_1 = { _zlang_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int32_t _u_first_2_3_i32(zlang_array_i32_2_3_t source) {
    return zlang_array_get_i32(source.data_, 0, 2);
}

int32_t _u_first_4_i32(zlang_array_i32_4_t source_2) {
    return zlang_array_get_i32(source_2.data_, 0, 4);
}

int main(void) {
    _zlang_array_helpers_keep();
    zlang_array_i32_2_3_t _grid = { { 0, 0, 0, 0, 0, 0 } };

    _zlang_array_set_2_i32(_grid.data_, 2, 3, 0, 0, 7);

    zlang_array_i32_4_t _line = { { 0, 0, 0, 0 } };

    uint32_t _zlang_index_4 = 0;
    if (_zlang_index_4 < 4) {
        _line.data_[_zlang_index_4] = 9;
    } else {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }

    zlang_print_i32(_u_first_2_3_i32(_grid));
    zlang_print(_zlang_literal_0);
    zlang_print_i32(_u_first_4_i32(_line));
    zlang_print(_zlang_literal_1);

    return ZLANG_ZERO;
}
