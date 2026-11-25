#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_I32

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated capacities */
#define ZLANG_SOURCE_CAPACITY 3U
#define ZLANG_SOURCE_LENGTH ZLANG_SOURCE_EXTENT_0
#define ZLANG_SOURCE_EXTENT_0 3U
#define ZLANG_TARGET_CAPACITY 3U
#define ZLANG_TARGET_LENGTH ZLANG_TARGET_EXTENT_0
#define ZLANG_TARGET_EXTENT_0 3U


/* generated sequence types */
typedef struct app_array_source_t {
    int32_t data_[ZLANG_SOURCE_CAPACITY];
} app_array_source_t;

typedef struct app_array_target_t {
    int32_t data_[ZLANG_TARGET_CAPACITY];
} app_array_target_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = " ";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

/* generated function prototypes */

int32_t app_fn_main(void);

int32_t app_fn_main(void) {
    const app_array_source_t app_local_source = { { 1, 2, 3 } };
    app_array_target_t app_local_target = { { 0, 0, 0 } };
    memcpy(&app_local_target, &app_local_source, sizeof(app_local_target));

    for (uint32_t zlang_gen_foreach_2 = ZLANG_ZERO; zlang_gen_foreach_2 < 3; ++zlang_gen_foreach_2) {
        int32_t app_local_value = app_local_target.data_[zlang_gen_foreach_2];
        zlang_std_io_write_i32(app_local_value);
        zlang_std_io_write_string(zlang_gen_literal_0);
    }

    zlang_std_io_write_string(zlang_gen_literal_1);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
