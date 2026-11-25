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
typedef struct app_struct_counter_t {
    int32_t value_;
} app_struct_counter_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = " ";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

/* generated method prototypes */

app_struct_counter_t app_fn_counter_construct(int32_t app_param_initial);
int32_t app_fn_counter_increment(app_struct_counter_t *self);
app_struct_counter_t app_fn_counter_zero(void);

/* generated function prototypes */

int32_t app_fn_main(void);

/* generated methods */

app_struct_counter_t app_fn_counter_construct(int32_t app_param_initial) {
    app_struct_counter_t self = { ZLANG_ZERO };
    self.value_ = app_param_initial;
    return self;
}

int32_t app_fn_counter_increment(app_struct_counter_t *self) {
    self->value_ = zlang_add_i32(self->value_, 1);

    return self->value_;
}

app_struct_counter_t app_fn_counter_zero(void) {
    return app_fn_counter_construct(0);
}

int32_t app_fn_main(void) {
    app_struct_counter_t app_local_counter = app_fn_counter_construct(1);
    zlang_std_io_write_i32(app_fn_counter_increment(&app_local_counter));
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_i32(app_fn_counter_increment(&app_local_counter));
    zlang_std_io_write_string(zlang_gen_literal_1);
    app_struct_counter_t app_local_empty = app_fn_counter_zero();
    zlang_std_io_write_i32(app_local_empty.value_);
    zlang_std_io_write_string(zlang_gen_literal_1);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
