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
typedef struct zlang_prime_t {
    bool data_[ZLANG_PRIME_CAPACITY];
} zlang_prime_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = " ";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char _zlang_literal_1_data[] = "\n";
static const zlang_string_t _zlang_literal_1 = { _zlang_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int main(void) {
    zlang_prime_t _prime = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

    for (int32_t _index = 0; (_index < 30); _index = zlang_add_i32(_index, 1)) {
        uint32_t _zlang_index_2 = (uint32_t)_index;
        if (_zlang_index_2 < 30) {
            _prime.data_[_zlang_index_2] = true;
        } else {
            zlang_trap(ZLANG_TRAP_BOUNDS);
        }
    }

    uint32_t _zlang_index_3 = 0;
    if (_zlang_index_3 < 30) {
        _prime.data_[_zlang_index_3] = false;
    } else {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }

    uint32_t _zlang_index_4 = 1;
    if (_zlang_index_4 < 30) {
        _prime.data_[_zlang_index_4] = false;
    } else {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }

    for (int32_t _candidate = 2; (_candidate < 30); _candidate = zlang_add_i32(_candidate, 1)) {
        if (zlang_array_get_bool(_prime.data_, (uint32_t)_candidate, 30)) {
            for (int32_t _multiple = zlang_mul_i32(_candidate, _candidate); (_multiple < 30); _multiple = zlang_add_i32(_multiple, _candidate)) {
                uint32_t _zlang_index_5 = (uint32_t)_multiple;
                if (_zlang_index_5 < 30) {
                    _prime.data_[_zlang_index_5] = false;
                } else {
                    zlang_trap(ZLANG_TRAP_BOUNDS);
                }
            }
        }
    }

    for (int32_t _index = 0; (_index < 30); _index = zlang_add_i32(_index, 1)) {
        if (zlang_array_get_bool(_prime.data_, (uint32_t)_index, 30)) {
            zlang_print_i32(_index);
            zlang_print(_zlang_literal_0);
        }
    }

    zlang_print(_zlang_literal_1);

    return ZLANG_ZERO;
}
