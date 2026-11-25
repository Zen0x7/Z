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
    int32_t y_;
} zlang_point_t;

typedef enum zlang_color_t {
    ZLANG_COLOR_RED = 0U,
    ZLANG_COLOR_GREEN = 1U,
    ZLANG_COLOR_BLUE = 2U,
} zlang_color_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = " ";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char _zlang_literal_1_data[] = "\n";
static const zlang_string_t _zlang_literal_1 = { _zlang_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int main(void) {
    zlang_point_t _point = { ZLANG_ZERO };
    _point.x_ = 3;
    _point.y_ = 4;
    zlang_color_t _color = ZLANG_COLOR_BLUE;
    zlang_print_i32(_point.x_);
    zlang_print(_zlang_literal_0);
    zlang_print_i32(_point.y_);
    zlang_print(_zlang_literal_0);
    zlang_print_u32(_color);
    zlang_print(_zlang_literal_1);

    return ZLANG_ZERO;
}
