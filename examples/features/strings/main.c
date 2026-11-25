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
static const char _zlang_literal_0_data[] = "zlang";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 5U
static const char _zlang_literal_1_data[] = " tour";
static const zlang_string_t _zlang_literal_1 = { _zlang_literal_1_data, ZLANG_LITERAL_1_LENGTH };
#define ZLANG_LITERAL_2_LENGTH 3U
static const char _zlang_literal_2_data[] = "abc";
static const zlang_string_t _zlang_literal_2 = { _zlang_literal_2_data, ZLANG_LITERAL_2_LENGTH };
#define ZLANG_LITERAL_3_LENGTH 1U
static const char _zlang_literal_3_data[] = "z";
static const zlang_string_t _zlang_literal_3 = { _zlang_literal_3_data, ZLANG_LITERAL_3_LENGTH };
#define ZLANG_LITERAL_4_LENGTH 1U
static const char _zlang_literal_4_data[] = " ";
static const zlang_string_t _zlang_literal_4 = { _zlang_literal_4_data, ZLANG_LITERAL_4_LENGTH };
#define ZLANG_LITERAL_5_LENGTH 1U
static const char _zlang_literal_5_data[] = "\n";
static const zlang_string_t _zlang_literal_5 = { _zlang_literal_5_data, ZLANG_LITERAL_5_LENGTH };

int main(void) {
    zlang_string_t _folded = _zlang_literal_0;
    char _zlang_bounded_storage_6[ZLANG_BOUNDED_CAPACITY + ZLANG_ONE] = { ZLANG_ZERO };
    zlang_string_t _bounded = zlang_string_concat(_zlang_bounded_storage_6, ZLANG_BOUNDED_CAPACITY, (zlang_string_t[]){ _folded, _zlang_literal_1 }, ZLANG_TWO);
    uint32_t _first = zlang_string_byte(_bounded, 0);
    char _zlang_letter_storage_7[ZLANG_LETTER_CAPACITY + ZLANG_ONE] = { ZLANG_ZERO };
    zlang_string_t _letter = zlang_string_concat(_zlang_letter_storage_7, ZLANG_LETTER_CAPACITY, (zlang_string_t[]){ _zlang_literal_2 }, ZLANG_ONE);
    _letter = _zlang_literal_3;
    zlang_print(_folded);
    zlang_print(_zlang_literal_4);
    zlang_print(_bounded);
    zlang_print(_zlang_literal_4);
    zlang_print_u32(_first);
    zlang_print(_zlang_literal_4);
    zlang_print(_letter);
    zlang_print(_zlang_literal_5);

    return ZLANG_ZERO;
}
