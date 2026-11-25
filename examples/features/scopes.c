#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = " ";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char _zlang_literal_1_data[] = "\n";
static const zlang_string_t _zlang_literal_1 = { _zlang_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int main(void) {
    int32_t _outer = 1;

    {
        int32_t _inner = 2;
        zlang_print_i32(_outer);
        zlang_print(_zlang_literal_0);
        zlang_print_i32(_inner);
        zlang_print(_zlang_literal_1);
    }

    zlang_print_i32(_outer);
    zlang_print(_zlang_literal_1);

    return ZLANG_ZERO;
}
