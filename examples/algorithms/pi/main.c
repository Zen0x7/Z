#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = ".";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int main(void) {
    int32_t app_local_scale = 100000;
    int32_t app_local_total = 0;
    int32_t app_local_denominator = 1;
    bool app_local_add_term = true;

    for (int32_t app_local_index = 0; (app_local_index < 200000); app_local_index = zlang_add_i32(app_local_index, 1)) {
        if (app_local_add_term) {
            app_local_total = zlang_add_i32(app_local_total, zlang_div_i32(zlang_mul_i32(4, app_local_scale), app_local_denominator));
        } else {
            app_local_total = zlang_sub_i32(app_local_total, zlang_div_i32(zlang_mul_i32(4, app_local_scale), app_local_denominator));
        }

        app_local_denominator = zlang_add_i32(app_local_denominator, 2);
        app_local_add_term = (!app_local_add_term);
    }

    zlang_print_i32(zlang_div_i32(app_local_total, app_local_scale));
    zlang_print(zlang_gen_literal_0);
    zlang_print_i32(zlang_mod_i32(app_local_total, app_local_scale));
    zlang_print(zlang_gen_literal_1);

    return ZLANG_ZERO;
}
