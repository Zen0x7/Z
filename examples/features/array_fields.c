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
#define ZLANG_CELLS_CAPACITY 6U
#define ZLANG_CELLS_LENGTH ZLANG_CELLS_EXTENT_0
#define ZLANG_CELLS_EXTENT_0 2U
#define ZLANG_CELLS_EXTENT_1 3U


/* generated sequence types */
typedef struct zlang_array_i32_2_3_t {
    int32_t data_[ZLANG_CELLS_CAPACITY];
} zlang_array_i32_2_3_t;

/* generated user types */
typedef struct zlang_matrix_t {
    zlang_array_i32_2_3_t cells_;
} zlang_matrix_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = "\n";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };

int main(void) {
    _zlang_array_helpers_keep();
    zlang_matrix_t _matrix = { ZLANG_ZERO };
    _zlang_array_set_2_i32(_matrix.cells_.data_, 2, 3, 1, 2, 7);
    int32_t _value = _zlang_array_get_2_i32(_matrix.cells_.data_, 2, 3, 1, 2);
    zlang_print_i32(_value);
    zlang_print(_zlang_literal_0);

    return ZLANG_ZERO;
}
