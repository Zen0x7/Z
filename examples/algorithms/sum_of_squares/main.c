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
static const char _zlang_literal_0_data[] = "\n";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };

int main(void) {
    zlang_values_t _values = { { 1, 2, 3, 4, 5 } };
    int32_t _total = 0;

    for (uint32_t _zlang_foreach_1 = ZLANG_ZERO; _zlang_foreach_1 < 5; ++_zlang_foreach_1) {
        int32_t _value = _values.data_[_zlang_foreach_1];
        _total = zlang_add_i32(_total, zlang_mul_i32(_value, _value));
    }

    zlang_print_i32(_total);
    zlang_print(_zlang_literal_0);

    return ZLANG_ZERO;
}
