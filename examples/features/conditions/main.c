#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_BOOL

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 3U
static const char zlang_gen_literal_0_data[] = "one";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 3U
static const char zlang_gen_literal_1_data[] = "two";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };
#define ZLANG_LITERAL_2_LENGTH 5U
static const char zlang_gen_literal_2_data[] = "other";
static const zlang_string_t zlang_gen_literal_2 = { zlang_gen_literal_2_data, ZLANG_LITERAL_2_LENGTH };
#define ZLANG_LITERAL_3_LENGTH 1U
static const char zlang_gen_literal_3_data[] = " ";
static const zlang_string_t zlang_gen_literal_3 = { zlang_gen_literal_3_data, ZLANG_LITERAL_3_LENGTH };
#define ZLANG_LITERAL_4_LENGTH 1U
static const char zlang_gen_literal_4_data[] = "\n";
static const zlang_string_t zlang_gen_literal_4 = { zlang_gen_literal_4_data, ZLANG_LITERAL_4_LENGTH };

int main(void) {
    int32_t app_local_value = 2;
    bool app_local_ready = true;

    if (app_local_value == 1) {
        zlang_std_io_write_string(zlang_gen_literal_0);
    } else if (app_local_value == 2) {
        zlang_std_io_write_string(zlang_gen_literal_1);
    } else {
        zlang_std_io_write_string(zlang_gen_literal_2);
    }

    zlang_std_io_write_string(zlang_gen_literal_3);
    zlang_std_io_write_bool((app_local_ready && app_local_value == 2));
    zlang_std_io_write_string(zlang_gen_literal_3);
    zlang_std_io_write_bool(((!app_local_ready) || app_local_value == 2));
    zlang_std_io_write_string(zlang_gen_literal_3);
    zlang_std_io_write_bool((app_local_value < 3 && app_local_value >= 2));
    zlang_std_io_write_string(zlang_gen_literal_3);
    zlang_std_io_write_bool((app_local_value != 2 || (!(app_local_value > 10))));
    zlang_std_io_write_string(zlang_gen_literal_4);

    return ZLANG_ZERO;
}
