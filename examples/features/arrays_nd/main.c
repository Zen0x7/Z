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

static inline uint32_t _zlang_array_offset_3(uint32_t extent_0, uint32_t extent_1, uint32_t extent_2, uint32_t index_0, uint32_t index_1, uint32_t index_2) {
    if (index_0 >= extent_0 || index_1 >= extent_1 || index_2 >= extent_2) {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }
    return extent_1 * extent_2 * index_0 + extent_2 * index_1 + index_2;
}

static inline uint32_t _zlang_array_offset_4(uint32_t extent_0, uint32_t extent_1, uint32_t extent_2, uint32_t extent_3, uint32_t index_0, uint32_t index_1, uint32_t index_2, uint32_t index_3) {
    if (index_0 >= extent_0 || index_1 >= extent_1 || index_2 >= extent_2 || index_3 >= extent_3) {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }
    return extent_1 * extent_2 * extent_3 * index_0 + extent_2 * extent_3 * index_1 + extent_3 * index_2 + index_3;
}

static inline int32_t _zlang_array_get_2_i32(const int32_t *data_, uint32_t extent_0, uint32_t extent_1, uint32_t index_0, uint32_t index_1) {
    return data_[_zlang_array_offset_2(extent_0, extent_1, index_0, index_1)];
}

static inline int32_t _zlang_array_get_3_i32(const int32_t *data_, uint32_t extent_0, uint32_t extent_1, uint32_t extent_2, uint32_t index_0, uint32_t index_1, uint32_t index_2) {
    return data_[_zlang_array_offset_3(extent_0, extent_1, extent_2, index_0, index_1, index_2)];
}

static inline int32_t _zlang_array_get_4_i32(const int32_t *data_, uint32_t extent_0, uint32_t extent_1, uint32_t extent_2, uint32_t extent_3, uint32_t index_0, uint32_t index_1, uint32_t index_2, uint32_t index_3) {
    return data_[_zlang_array_offset_4(extent_0, extent_1, extent_2, extent_3, index_0, index_1, index_2, index_3)];
}

/* generated capacities */
#define ZLANG_SQUARE_CAPACITY 4U
#define ZLANG_SQUARE_LENGTH ZLANG_SQUARE_EXTENT_0
#define ZLANG_SQUARE_EXTENT_0 2U
#define ZLANG_SQUARE_EXTENT_1 2U
#define ZLANG_CUBE_CAPACITY 8U
#define ZLANG_CUBE_LENGTH ZLANG_CUBE_EXTENT_0
#define ZLANG_CUBE_EXTENT_0 2U
#define ZLANG_CUBE_EXTENT_1 2U
#define ZLANG_CUBE_EXTENT_2 2U
#define ZLANG_HYPER_CAPACITY 16U
#define ZLANG_HYPER_LENGTH ZLANG_HYPER_EXTENT_0
#define ZLANG_HYPER_EXTENT_0 2U
#define ZLANG_HYPER_EXTENT_1 2U
#define ZLANG_HYPER_EXTENT_2 2U
#define ZLANG_HYPER_EXTENT_3 2U


/* generated sequence types */
typedef struct zlang_square_t {
    int32_t data_[ZLANG_SQUARE_CAPACITY];
} zlang_square_t;

typedef struct zlang_cube_t {
    int32_t data_[ZLANG_CUBE_CAPACITY];
} zlang_cube_t;

typedef struct zlang_hyper_t {
    int32_t data_[ZLANG_HYPER_CAPACITY];
} zlang_hyper_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = "\n";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };

int main(void) {
    zlang_square_t _square = { { 1, 2, 3, 4 } };
    zlang_cube_t _cube = { { 1, 2, 3, 4, 5, 6, 7, 8 } };
    zlang_hyper_t _hyper = { { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } };
    int32_t _total = 0;

    for (uint32_t _row = ZLANG_ZERO; _row < 2; ++_row) {
        for (uint32_t _col = ZLANG_ZERO; _col < 2; ++_col) {
            _total = zlang_add_i32(_total, _zlang_array_get_2_i32(_square.data_, 2, 2, _row, _col));
        }
    }

    for (uint32_t _i = ZLANG_ZERO; _i < 2; ++_i) {
        for (uint32_t _j = ZLANG_ZERO; _j < 2; ++_j) {
            for (uint32_t _k = ZLANG_ZERO; _k < 2; ++_k) {
                _total = zlang_add_i32(_total, _zlang_array_get_3_i32(_cube.data_, 2, 2, 2, _i, _j, _k));
            }
        }
    }

    for (uint32_t _a = ZLANG_ZERO; _a < 2; ++_a) {
        for (uint32_t _b = ZLANG_ZERO; _b < 2; ++_b) {
            for (uint32_t _c = ZLANG_ZERO; _c < 2; ++_c) {
                for (uint32_t _d = ZLANG_ZERO; _d < 2; ++_d) {
                    _total = zlang_add_i32(_total, _zlang_array_get_4_i32(_hyper.data_, 2, 2, 2, 2, _a, _b, _c, _d));
                }
            }
        }
    }

    zlang_print_i32(_total);
    zlang_print(_zlang_literal_0);

    return ZLANG_ZERO;
}
