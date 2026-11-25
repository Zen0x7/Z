#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = ".";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char _zlang_literal_1_data[] = "\n";
static const zlang_string_t _zlang_literal_1 = { _zlang_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int main(void) {
    int32_t _scale = 100000;
    int32_t _total = 0;
    int32_t _denominator = 1;
    bool _add_term = true;

    for (int32_t _index = 0; (_index < 200000); _index = zlang_add_i32(_index, 1)) {
        if (_add_term) {
            _total = zlang_add_i32(_total, zlang_div_i32(zlang_mul_i32(4, _scale), _denominator));
        } else {
            _total = zlang_sub_i32(_total, zlang_div_i32(zlang_mul_i32(4, _scale), _denominator));
        }

        _denominator = zlang_add_i32(_denominator, 2);
        _add_term = (!_add_term);
    }

    zlang_print_i32(zlang_div_i32(_total, _scale));
    zlang_print(_zlang_literal_0);
    zlang_print_i32(zlang_mod_i32(_total, _scale));
    zlang_print(_zlang_literal_1);

    return ZLANG_ZERO;
}
