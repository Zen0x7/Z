#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_I32

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated user types */
typedef enum app_variant_shape_tag_t {
    APP_VARIANT_SHAPE_CIRCLE = 1,
    APP_VARIANT_SHAPE_RECT = 2,
} app_variant_shape_tag_t;

typedef struct app_variant_shape_t {
    app_variant_shape_tag_t tag_;
    union {
        struct {
            int32_t radius_;
        } circle_;
        struct {
            int32_t width_;
            int32_t height_;
        } rect_;
    } value_;
} app_variant_shape_t;

app_variant_shape_t app_variant_shape_circle(int32_t app_param_radius);
app_variant_shape_t app_variant_shape_rect(int32_t app_param_width, int32_t app_param_height);
/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = " ";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

/* generated function prototypes */

int32_t app_fn_area(app_variant_shape_t app_param_shape);
int32_t app_fn_main(void);

/* generated variant constructors */

app_variant_shape_t app_variant_shape_circle(int32_t app_param_radius) {
    app_variant_shape_t self = { ZLANG_ZERO };
    self.tag_ = APP_VARIANT_SHAPE_CIRCLE;
    self.value_.circle_.radius_ = app_param_radius;
    return self;
}

app_variant_shape_t app_variant_shape_rect(int32_t app_param_width, int32_t app_param_height) {
    app_variant_shape_t self = { ZLANG_ZERO };
    self.tag_ = APP_VARIANT_SHAPE_RECT;
    self.value_.rect_.width_ = app_param_width;
    self.value_.rect_.height_ = app_param_height;
    return self;
}

int32_t app_fn_area(app_variant_shape_t app_param_shape) {
    return (app_param_shape.tag_ == APP_VARIANT_SHAPE_CIRCLE ? zlang_mul_i32(app_param_shape.value_.circle_.radius_, app_param_shape.value_.circle_.radius_) : app_param_shape.tag_ == APP_VARIANT_SHAPE_RECT ? zlang_mul_i32(app_param_shape.value_.rect_.width_, app_param_shape.value_.rect_.height_) : 0);
}

int32_t app_fn_main(void) {
    zlang_std_io_write_i32(app_fn_area(app_variant_shape_circle(7)));
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_i32(app_fn_area(app_variant_shape_rect(3, 4)));
    zlang_std_io_write_string(zlang_gen_literal_1);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
