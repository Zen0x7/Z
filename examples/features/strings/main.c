#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_U32

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated capacities */
#define ZLANG_BOUNDED_CAPACITY 32U
#define ZLANG_LETTER_CAPACITY 8U

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 5U
static const char zlang_gen_literal_0_data[] = "zlang";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 5U
static const char zlang_gen_literal_1_data[] = " tour";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };
#define ZLANG_LITERAL_2_LENGTH 3U
static const char zlang_gen_literal_2_data[] = "abc";
static const zlang_string_t zlang_gen_literal_2 = { zlang_gen_literal_2_data, ZLANG_LITERAL_2_LENGTH };
#define ZLANG_LITERAL_3_LENGTH 1U
static const char zlang_gen_literal_3_data[] = "z";
static const zlang_string_t zlang_gen_literal_3 = { zlang_gen_literal_3_data, ZLANG_LITERAL_3_LENGTH };
#define ZLANG_LITERAL_4_LENGTH 1U
static const char zlang_gen_literal_4_data[] = " ";
static const zlang_string_t zlang_gen_literal_4 = { zlang_gen_literal_4_data, ZLANG_LITERAL_4_LENGTH };
#define ZLANG_LITERAL_5_LENGTH 1U
static const char zlang_gen_literal_5_data[] = "\n";
static const zlang_string_t zlang_gen_literal_5 = { zlang_gen_literal_5_data, ZLANG_LITERAL_5_LENGTH };

/* generated function prototypes */

int32_t app_fn_main(void);

int32_t app_fn_main(void) {
    zlang_string_t app_local_folded = zlang_gen_literal_0;
    char zlang_gen_bounded_storage_6[ZLANG_BOUNDED_CAPACITY + ZLANG_ONE] = { ZLANG_ZERO };
    zlang_string_t app_local_bounded = zlang_string_concat(zlang_gen_bounded_storage_6, ZLANG_BOUNDED_CAPACITY, (zlang_string_t[]){ app_local_folded, zlang_gen_literal_1 }, ZLANG_TWO);
    uint32_t app_local_first = zlang_string_byte(app_local_bounded, 0);
    char zlang_gen_letter_storage_7[ZLANG_LETTER_CAPACITY + ZLANG_ONE] = { ZLANG_ZERO };
    zlang_string_t app_local_letter = zlang_string_concat(zlang_gen_letter_storage_7, ZLANG_LETTER_CAPACITY, (zlang_string_t[]){ zlang_gen_literal_2 }, ZLANG_ONE);
    app_local_letter = zlang_gen_literal_3;
    zlang_std_io_write_string(app_local_folded);
    zlang_std_io_write_string(zlang_gen_literal_4);
    zlang_std_io_write_string(app_local_bounded);
    zlang_std_io_write_string(zlang_gen_literal_4);
    zlang_std_io_write_u32(app_local_first);
    zlang_std_io_write_string(zlang_gen_literal_4);
    zlang_std_io_write_string(app_local_letter);
    zlang_std_io_write_string(zlang_gen_literal_5);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
