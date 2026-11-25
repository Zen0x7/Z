#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = "\n";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };

/* generated function prototypes */

int32_t app_fn_add(int32_t app_param_left, int32_t app_param_right);
void app_fn_bump(int32_t *app_param_value);

int32_t app_fn_add(int32_t app_param_left, int32_t app_param_right) {
    return zlang_add_i32(app_param_left, app_param_right);
}

void app_fn_bump(int32_t *app_param_value) {
    (*app_param_value) = zlang_add_i32((*app_param_value), 1);
}

int main(void) {
    int32_t app_local_total = app_fn_add(2, 3);
    app_fn_bump(&app_local_total);
    zlang_print_i32(app_local_total);
    zlang_print(zlang_gen_literal_0);

    return ZLANG_ZERO;
}
