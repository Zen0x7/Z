#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated capacities */
#define ZLANG_VALUES_CAPACITY 6U
#define ZLANG_VALUES_LENGTH ZLANG_VALUES_EXTENT_0
#define ZLANG_VALUES_EXTENT_0 6U


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
    zlang_values_t _values = { { 5, 1, 4, 2, 8, 3 } };

    for (int32_t _pass = 0; (_pass < 5); _pass = zlang_add_i32(_pass, 1)) {
        for (int32_t _index = 0; (_index < zlang_sub_i32(5, _pass)); _index = zlang_add_i32(_index, 1)) {

            int32_t _zlang_value_2 = zlang_array_get_i32(_values.data_, (uint32_t)_index, 6);

            int32_t _zlang_value_3 = zlang_array_get_i32(_values.data_, (uint32_t)zlang_add_i32(_index, 1), 6);
            if (_zlang_value_2 > _zlang_value_3) {
                int32_t _temporary = _zlang_value_2;

                uint32_t _zlang_index_4 = (uint32_t)_index;
                if (_zlang_index_4 < 6) {
                    _values.data_[_zlang_index_4] = zlang_array_get_i32(_values.data_, (uint32_t)zlang_add_i32(_index, 1), 6);
                } else {
                    zlang_trap(ZLANG_TRAP_BOUNDS);
                }

                uint32_t _zlang_index_5 = (uint32_t)zlang_add_i32(_index, 1);
                if (_zlang_index_5 < 6) {
                    _values.data_[_zlang_index_5] = _temporary;
                } else {
                    zlang_trap(ZLANG_TRAP_BOUNDS);
                }
            }
        }
    }

    for (uint32_t _zlang_foreach_6 = ZLANG_ZERO; _zlang_foreach_6 < 6; ++_zlang_foreach_6) {
        int32_t _value = _values.data_[_zlang_foreach_6];
        zlang_print_i32(_value);
        zlang_print(_zlang_literal_0);
    }

    zlang_print(_zlang_literal_1);

    return ZLANG_ZERO;
}
