#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 3U
static const char _zlang_literal_0_data[] = "one";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 3U
static const char _zlang_literal_1_data[] = "two";
static const zlang_string_t _zlang_literal_1 = { _zlang_literal_1_data, ZLANG_LITERAL_1_LENGTH };
#define ZLANG_LITERAL_2_LENGTH 5U
static const char _zlang_literal_2_data[] = "other";
static const zlang_string_t _zlang_literal_2 = { _zlang_literal_2_data, ZLANG_LITERAL_2_LENGTH };
#define ZLANG_LITERAL_3_LENGTH 1U
static const char _zlang_literal_3_data[] = " ";
static const zlang_string_t _zlang_literal_3 = { _zlang_literal_3_data, ZLANG_LITERAL_3_LENGTH };
#define ZLANG_LITERAL_4_LENGTH 1U
static const char _zlang_literal_4_data[] = "\n";
static const zlang_string_t _zlang_literal_4 = { _zlang_literal_4_data, ZLANG_LITERAL_4_LENGTH };

int main(void) {
    int32_t _value = 2;
    bool _ready = true;

    if (_value == 1) {
        zlang_print(_zlang_literal_0);
    } else if (_value == 2) {
        zlang_print(_zlang_literal_1);
    } else {
        zlang_print(_zlang_literal_2);
    }

    zlang_print(_zlang_literal_3);
    zlang_print_bool((_ready && _value == 2));
    zlang_print(_zlang_literal_3);
    zlang_print_bool(((!_ready) || _value == 2));
    zlang_print(_zlang_literal_3);
    zlang_print_bool((_value < 3 && _value >= 2));
    zlang_print(_zlang_literal_3);
    zlang_print_bool((_value != 2 || (!(_value > 10))));
    zlang_print(_zlang_literal_4);

    return ZLANG_ZERO;
}
