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
static const char zlang_gen_literal_0_data[] = "\n";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };

/* generated method prototypes */

app_struct_counter_t app_fn_counter_construct(int32_t app_param_initial);
int32_t app_fn_counter_label(app_struct_counter_t *self);

/* generated function prototypes */

int32_t app_fn_main(void);
int32_t app_fn_show_counter(app_struct_counter_t app_param_item);

/* generated methods */

app_struct_counter_t app_fn_counter_construct(int32_t app_param_initial) {
    app_struct_counter_t self = { ZLANG_ZERO };
    self.value_ = app_param_initial;
    return self;
}

int32_t app_fn_counter_label(app_struct_counter_t *self) {
    return self->value_;
}

int32_t app_fn_main(void) {
    app_struct_counter_t app_local_counter = app_fn_counter_construct(7);
    zlang_std_io_write_i32(app_fn_show_counter(app_local_counter));
    zlang_std_io_write_string(zlang_gen_literal_0);

    return 0;
}

int32_t app_fn_show_counter(app_struct_counter_t app_param_item) {
    return app_fn_counter_label(&app_param_item);
}

int main(void) {
    return (int)app_fn_main();
}
