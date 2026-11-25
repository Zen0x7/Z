#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = "\n";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };

int32_t _u_add(int32_t left, int32_t right) {
    return zlang_add_i32(left, right);
}

void _u_bump(int32_t *value) {
    (*value) = zlang_add_i32((*value), 1);
}

int main(void) {
    int32_t _total = _u_add(2, 3);
    _u_bump(&_total);
    zlang_print_i32(_total);
    zlang_print(_zlang_literal_0);

    return ZLANG_ZERO;
}
