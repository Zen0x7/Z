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
typedef struct zlang_source_t {
    int32_t data_[ZLANG_SOURCE_CAPACITY];
} zlang_source_t;

typedef struct zlang_target_t {
    int32_t data_[ZLANG_TARGET_CAPACITY];
} zlang_target_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char _zlang_literal_0_data[] = " ";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char _zlang_literal_1_data[] = "\n";
static const zlang_string_t _zlang_literal_1 = { _zlang_literal_1_data, ZLANG_LITERAL_1_LENGTH };

int main(void) {
    const zlang_source_t _source = { { 1, 2, 3 } };
    zlang_target_t _target = { { 0, 0, 0 } };
    memcpy(&_target, &_source, sizeof(_target));

    for (uint32_t _zlang_foreach_2 = ZLANG_ZERO; _zlang_foreach_2 < 3; ++_zlang_foreach_2) {
        int32_t _value = _target.data_[_zlang_foreach_2];
        zlang_print_i32(_value);
        zlang_print(_zlang_literal_0);
    }

    zlang_print(_zlang_literal_1);

    return ZLANG_ZERO;
}
