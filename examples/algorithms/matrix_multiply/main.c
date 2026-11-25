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
typedef struct zlang_left_t {
    int32_t data_[ZLANG_LEFT_CAPACITY];
} zlang_left_t;

typedef struct zlang_right_t {
    int32_t data_[ZLANG_RIGHT_CAPACITY];
} zlang_right_t;

typedef struct zlang_result_t {
    int32_t data_[ZLANG_RESULT_CAPACITY];
} zlang_result_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = " ";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char _zlang_literal_1_data[] = "\n";
static const zlang_string_t _zlang_literal_1 = { _zlang_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int main(void) {
    zlang_left_t _left = { { 1, 2, 3, 4 } };
    zlang_right_t _right = { { 5, 6, 7, 8 } };
    zlang_result_t _result = { { 0, 0, 0, 0 } };

    for (int32_t _row = 0; (_row < 2); _row = zlang_add_i32(_row, 1)) {
        for (int32_t _column = 0; (_column < 2); _column = zlang_add_i32(_column, 1)) {
            int32_t _cell = 0;

            for (int32_t _inner = 0; (_inner < 2); _inner = zlang_add_i32(_inner, 1)) {
                _cell = zlang_add_i32(_cell, zlang_mul_i32(_zlang_array_get_2_i32(_left.data_, 2, 2, (uint32_t)_row, (uint32_t)_inner), _zlang_array_get_2_i32(_right.data_, 2, 2, (uint32_t)_inner, (uint32_t)_column)));
            }

            _zlang_array_set_2_i32(_result.data_, 2, 2, (uint32_t)_row, (uint32_t)_column, _cell);
        }
    }

    for (uint32_t _row_3 = ZLANG_ZERO; _row_3 < 2; ++_row_3) {
        for (uint32_t _column_4 = ZLANG_ZERO; _column_4 < 2; ++_column_4) {
            zlang_print_i32(_zlang_array_get_2_i32(_result.data_, 2, 2, _row_3, _column_4));
            zlang_print(_zlang_literal_0);
        }
    }

    zlang_print(_zlang_literal_1);

    return ZLANG_ZERO;
}
