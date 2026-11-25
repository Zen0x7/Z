#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated user types */
typedef struct app_record_point_t {
    int32_t x_;
    int32_t y_;
} app_record_point_t;

typedef enum app_enum_color_t {
    ZLANG_COLOR_RED = 0U,
    ZLANG_COLOR_GREEN = 1U,
    ZLANG_COLOR_BLUE = 2U,
} app_enum_color_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = " ";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int main(void) {
    app_record_point_t app_local_point = { ZLANG_ZERO };
    app_local_point.x_ = 3;
    app_local_point.y_ = 4;
    app_enum_color_t app_local_color = ZLANG_COLOR_BLUE;
    zlang_print_i32(app_local_point.x_);
    zlang_print(zlang_gen_literal_0);
    zlang_print_i32(app_local_point.y_);
    zlang_print(zlang_gen_literal_0);
    zlang_print_u32(app_local_color);
    zlang_print(zlang_gen_literal_1);

    return ZLANG_ZERO;
}
