#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_I32

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated type tags */
#define ZLANG_TYPE_SHAPE 1U
#define ZLANG_TYPE_SQUARE 2U

/* generated vtable types */
typedef struct zlang_vtable_shape_t zlang_vtable_shape_t;

/* generated user types */
typedef struct app_struct_shape_t {
    const zlang_vtable_shape_t *zlang_vtable_;
    uint32_t zlang_type_;
    int32_t value_;
} app_struct_shape_t;

typedef struct app_struct_square_t {
    app_struct_shape_t zlang_base_;
    int32_t side_;
} app_struct_square_t;

/* generated vtable definitions */

struct zlang_vtable_shape_t {
    int32_t (*zlang_slot_area_)(void *self);
};

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = " ";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

/* generated method prototypes */

app_struct_shape_t app_fn_shape_construct(int32_t app_param_initial);
int32_t app_fn_shape_area(app_struct_shape_t *self);
app_struct_square_t app_fn_square_construct(int32_t app_param_initial, int32_t app_param_side_value);
int32_t app_fn_square_area(app_struct_square_t *self);

int32_t app_fn_shape_label(app_struct_shape_t *self);
static int32_t zlang_vtable_shape_area(void *self);
static int32_t zlang_vtable_square_area(void *self);
/* generated vtables */

const zlang_vtable_shape_t zlang_vtable_shape = { zlang_vtable_shape_area };
const zlang_vtable_shape_t zlang_vtable_square = { zlang_vtable_square_area };

/* generated function prototypes */

int32_t app_fn_describe(app_struct_shape_t *app_param_shape);
int32_t app_fn_main(void);

/* generated methods */

app_struct_shape_t app_fn_shape_construct(int32_t app_param_initial) {
    app_struct_shape_t self = { ZLANG_ZERO };
    self.zlang_vtable_ = &zlang_vtable_shape;
    self.zlang_type_ = ZLANG_TYPE_SHAPE;
    self.value_ = app_param_initial;
    return self;
}

int32_t app_fn_shape_area(app_struct_shape_t *self) {
    return self->value_;
}

app_struct_square_t app_fn_square_construct(int32_t app_param_initial, int32_t app_param_side_value) {
    app_struct_square_t self = { ZLANG_ZERO };
    self.zlang_base_.zlang_vtable_ = &zlang_vtable_square;
    self.zlang_base_.zlang_type_ = ZLANG_TYPE_SQUARE;
    self.side_ = app_param_side_value;
    self.zlang_base_.value_ = app_param_initial;
    return self;
}

int32_t app_fn_square_area(app_struct_square_t *self) {
    return zlang_mul_i32(self->side_, self->side_);
}

int32_t app_fn_shape_label(app_struct_shape_t *self) {
    return self->value_;
}

static int32_t zlang_vtable_shape_area(void *self) {
    return app_fn_shape_area((app_struct_shape_t *)self);
}

static int32_t zlang_vtable_square_area(void *self) {
    return app_fn_square_area((app_struct_square_t *)self);
}

int32_t app_fn_describe(app_struct_shape_t *app_param_shape) {
    return ((app_struct_shape_t *)app_param_shape)->zlang_vtable_->zlang_slot_area_((app_struct_shape_t *)app_param_shape);
}

int32_t app_fn_main(void) {
    app_struct_shape_t app_local_base = app_fn_shape_construct(2);
    app_struct_square_t app_local_square = app_fn_square_construct(3, 4);
    zlang_std_io_write_i32(app_fn_describe(&app_local_base));
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_i32(app_fn_describe(&app_local_square.zlang_base_));
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_i32(app_fn_shape_label(&app_local_base));
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_i32(app_fn_shape_label(&app_local_square.zlang_base_));
    zlang_std_io_write_string(zlang_gen_literal_1);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
