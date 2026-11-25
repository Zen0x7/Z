#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated user types */
// File-level traceability comment.
typedef struct zlang_point_t {
    // Field comment.
    int32_t x_;
} zlang_point_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = "\n";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };

// Function comment.
int32_t _u_one(void) {
    // Body comment.
    return 1;
}

int main(void) {
    zlang_print_i32(_u_one());
    zlang_print(_zlang_literal_0);

    return ZLANG_ZERO;
}
