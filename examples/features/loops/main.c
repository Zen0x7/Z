#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated capacities */
#define ZLANG_VALUES_CAPACITY 4U
#define ZLANG_VALUES_LENGTH ZLANG_VALUES_EXTENT_0
#define ZLANG_VALUES_EXTENT_0 4U


/* generated sequence types */
typedef struct zlang_values_t {
    int32_t data_[ZLANG_VALUES_CAPACITY];
} zlang_values_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = ":";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char _zlang_literal_1_data[] = " ";
static const zlang_string_t _zlang_literal_1 = { _zlang_literal_1_data, ZLANG_LITERAL_1_LENGTH };
#define ZLANG_LITERAL_2_LENGTH 1U
static const char _zlang_literal_2_data[] = "\n";
static const zlang_string_t _zlang_literal_2 = { _zlang_literal_2_data, ZLANG_LITERAL_2_LENGTH };

int main(void) {
    int32_t _total = 0;

    while (_total < 2) {
        _total = zlang_add_i32(_total, 1);
    }

    for (uint32_t _zlang_repeat_3 = ZLANG_ZERO; _zlang_repeat_3 < 2U; ++_zlang_repeat_3) {
        _total = zlang_add_i32(_total, 1);
    }

    for (int32_t _index = 0; (_index < 3); _index = zlang_add_i32(_index, 1)) {
        if (_index == 1) {
            continue;
        }

        _total = zlang_add_i32(_total, _index);
    }

    zlang_values_t _values = { { 1, 2, 3, 4 } };

    for (uint32_t _zlang_foreach_4 = ZLANG_ZERO; _zlang_foreach_4 < 4; ++_zlang_foreach_4) {
        int32_t _value = _values.data_[_zlang_foreach_4];
        if (_value == 3) {
            break;
        }

        _total = zlang_add_i32(_total, _value);
    }

    for (uint32_t _zlang_foreach_5 = ZLANG_ZERO; _zlang_foreach_5 < 4; ++_zlang_foreach_5) {
        uint32_t _index = _zlang_foreach_5;
        int32_t _item = _values.data_[_zlang_foreach_5];
        zlang_print_u32(_index);
        zlang_print(_zlang_literal_0);
        zlang_print_i32(_item);
        zlang_print(_zlang_literal_1);
    }

    zlang_print_i32(_total);
    zlang_print(_zlang_literal_2);

    return ZLANG_ZERO;
}
