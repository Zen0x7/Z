#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated user types */
typedef struct zlang_point_t {
    int32_t x_;
} zlang_point_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = "\n";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };

zlang_point_t _u_make(void) {
    zlang_point_t _result = { ZLANG_ZERO };
    _result.x_ = 1;

    return _result;
}

void _u_shift(zlang_point_t *point, int32_t amount) {
    point->x_ = zlang_add_i32(point->x_, amount);
}

int main(void) {
    zlang_point_t _point = _u_make();
    _u_shift(&_point, 4);
    zlang_print_i32(_point.x_);
    zlang_print(_zlang_literal_0);

    return ZLANG_ZERO;
}
