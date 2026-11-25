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
#define ZLANG_PRIME_CAPACITY 30U
#define ZLANG_PRIME_LENGTH ZLANG_PRIME_EXTENT_0
#define ZLANG_PRIME_EXTENT_0 30U


/* generated sequence types */
typedef struct app_array_prime_t {
    bool data_[ZLANG_PRIME_CAPACITY];
} app_array_prime_t;

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
    app_array_prime_t app_local_prime = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

    for (int32_t app_local_index = 0; (app_local_index < 30); app_local_index = zlang_add_i32(app_local_index, 1)) {
        uint32_t zlang_gen_index_2 = (uint32_t)app_local_index;
        if (zlang_gen_index_2 < 30) {
            app_local_prime.data_[zlang_gen_index_2] = true;
        } else {
            zlang_trap(ZLANG_TRAP_BOUNDS);
        }
    }

    app_local_prime.data_[0] = false;
    app_local_prime.data_[1] = false;

    for (int32_t app_local_candidate = 2; (app_local_candidate < 30); app_local_candidate = zlang_add_i32(app_local_candidate, 1)) {
        if (zlang_array_get_bool(app_local_prime.data_, (uint32_t)app_local_candidate, 30)) {
            for (int32_t app_local_multiple = zlang_mul_i32(app_local_candidate, app_local_candidate); (app_local_multiple < 30); app_local_multiple = zlang_add_i32(app_local_multiple, app_local_candidate)) {
                uint32_t zlang_gen_index_3 = (uint32_t)app_local_multiple;
                if (zlang_gen_index_3 < 30) {
                    app_local_prime.data_[zlang_gen_index_3] = false;
                } else {
                    zlang_trap(ZLANG_TRAP_BOUNDS);
                }
            }
        }
    }

    for (int32_t app_local_index = 0; (app_local_index < 30); app_local_index = zlang_add_i32(app_local_index, 1)) {
        if (zlang_array_get_bool(app_local_prime.data_, (uint32_t)app_local_index, 30)) {
            zlang_std_io_write_i32(app_local_index);
            zlang_std_io_write_string(zlang_gen_literal_0);
        }
    }

    zlang_std_io_write_string(zlang_gen_literal_1);

    return 0;
}

int main(void) {
    return (int)app_fn_main();
}
