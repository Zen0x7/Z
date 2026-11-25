#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated capacities */
#define ZLANG_VALUES_CAPACITY 5U
#define ZLANG_VALUES_LENGTH ZLANG_VALUES_EXTENT_0
#define ZLANG_VALUES_EXTENT_0 5U


/* generated sequence types */
typedef struct zlang_values_t {
    int32_t data_[ZLANG_VALUES_CAPACITY];
} zlang_values_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = " ";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char _zlang_literal_1_data[] = "\n";
static const zlang_string_t _zlang_literal_1 = { _zlang_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int main(void) {
    zlang_values_t _values = { { 4, 5, 0, 0, 0 } };
    int32_t _capacity = 5;

    uint32_t _zlang_index_2 = 0;
    if (_zlang_index_2 < 5) {
        _values.data_[_zlang_index_2] = zlang_add_i32(zlang_array_get_i32(_values.data_, 1, 5), 1);
    } else {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }

    uint32_t _zlang_index_3 = 2;
    if (_zlang_index_3 < 5) {
        _values.data_[_zlang_index_3] = zlang_mul_i32(zlang_array_get_i32(_values.data_, 0, 5), 2);
    } else {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }

    uint32_t _zlang_index_4 = 3;
    if (_zlang_index_4 < 5) {
        _values.data_[_zlang_index_4] = zlang_sub_i32(zlang_array_get_i32(_values.data_, 2, 5), 1);
    } else {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }

    uint32_t _zlang_index_5 = 4;
    if (_zlang_index_5 < 5) {
        _values.data_[_zlang_index_5] = zlang_add_i32(zlang_array_get_i32(_values.data_, 3, 5), _capacity);
    } else {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }

    for (uint32_t _zlang_foreach_6 = ZLANG_ZERO; _zlang_foreach_6 < 5; ++_zlang_foreach_6) {
        int32_t _value = _values.data_[_zlang_foreach_6];
        zlang_print_i32(_value);
        zlang_print(_zlang_literal_0);
    }

    zlang_print(_zlang_literal_1);

    return ZLANG_ZERO;
}
