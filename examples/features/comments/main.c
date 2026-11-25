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
// File-level traceability comment.
typedef struct app_struct_point_t {
    // Field comment.
    int32_t x_;
} app_struct_point_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = "\n";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };

/* generated function prototypes */

int32_t app_fn_one(void);
int32_t app_fn_main(void);

// Function comment.
int32_t app_fn_one(void) {
    // Body comment.
    return 1;
}

int32_t app_fn_main(void) {
    zlang_std_io_write_i32(app_fn_one());
    zlang_std_io_write_string(zlang_gen_literal_0);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
