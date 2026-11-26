#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_I32

#define ZLANG_STD_IO_WRITE_BOOL

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated user types */
typedef struct app_struct_pair_int32_boolean_t {
    int32_t first_;
    bool second_;
} app_struct_pair_int32_boolean_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = " ";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };

/* generated function prototypes */

int32_t app_fn_main(void);
int32_t app_fn_identity_int32(int32_t app_param_value);

int32_t app_fn_main(void) {
    app_struct_pair_int32_boolean_t app_local_pair = { ZLANG_ZERO };
    app_local_pair.first_ = 7;
    app_local_pair.second_ = true;
    int32_t app_local_value = app_fn_identity_int32(app_local_pair.first_);
    zlang_std_io_write_i32(app_local_value);
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_bool(app_local_pair.second_);

    return 0;
}

int32_t app_fn_identity_int32(int32_t app_param_value) {
    return app_param_value;
}

int main(void) {
    return (int)app_fn_main();
}
