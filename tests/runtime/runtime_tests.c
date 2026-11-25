#ifdef ZLANG_COVERAGE
#include <sys/wait.h>
#include <unistd.h>

extern void __gcov_dump(void);
#define ZLANG_TRAP_SINK(code)                                                                      \
    do {                                                                                           \
        __gcov_dump();                                                                             \
        _exit(128 + (int)(code));                                                                  \
    } while (0)
#endif

#include "zlang/runtime/runtime.h"
#include "zlang/testing/testing.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

#define ZLANG_TEST_CASE(name) static void name(zlang_test_context_t *const context);
#include "test_cases.h"
#undef ZLANG_TEST_CASE

static void write_copies_the_exact_declared_byte_count(zlang_test_context_t *const context) {
    const zlang_string_t _message_k = {"hello", 5U};
    char _buffer[16];
    FILE *const _stream_k = tmpfile();

    ZLANG_CHECK(context, _stream_k != NULL);
    const bool _write_ok_k = zlang_write(_stream_k, _message_k);
    const bool _seek_ok_k = fseek(_stream_k, 0L, SEEK_SET) == 0;
    const size_t _read_count_k = fread(_buffer, 1U, sizeof(_buffer), _stream_k);
    const bool _compare_ok_k = memcmp(_buffer, "hello", 5U) == 0;
    (void)fclose(_stream_k);

    ZLANG_CHECK(context, _write_ok_k);
    ZLANG_CHECK(context, _seek_ok_k);
    ZLANG_CHECK(context, _read_count_k == 5U);
    ZLANG_CHECK(context, _compare_ok_k);
}

static void write_handles_empty_and_invalid_arguments(zlang_test_context_t *const context) {
    const zlang_string_t _empty_k = {NULL, 0U};
    const zlang_string_t _message_k = {"hello", 5U};
    FILE *const _stream_k = tmpfile();

    const bool _invalid_ok_k = !zlang_write(NULL, _message_k);
    const bool _empty_ok_k = zlang_write(_stream_k, _empty_k);
    const bool _print_ok_k = zlang_print(_empty_k);
    if (_stream_k != NULL) {
        (void)fclose(_stream_k);
    }

    ZLANG_CHECK(context, _invalid_ok_k);
    ZLANG_CHECK(context, _empty_ok_k);
    ZLANG_CHECK(context, _print_ok_k);
}

static void string_concat_handles_self_concatenation(zlang_test_context_t *const context) {
    char _storage[16] = {'a', '\0'};
    const zlang_string_t _self_k = {_storage, 1U};
    const zlang_string_t _parts_k[] = {_self_k, {"b", 1U}};
    const zlang_string_t _result_k = zlang_string_concat(_storage, 15U, _parts_k, 2U);

    ZLANG_CHECK(context, _result_k.length_ == 2U);
    ZLANG_CHECK(context, strcmp(_result_k.data_, "ab") == 0);
    ZLANG_CHECK(context, _storage[2] == '\0');
}

static void trap_codes_are_fixed(zlang_test_context_t *const context) {
    const uint8_t _overflow_k = ZLANG_TRAP_OVERFLOW;
    const uint8_t _bounds_k = ZLANG_TRAP_BOUNDS;
    const uint8_t _capacity_k = ZLANG_TRAP_CAPACITY;
    const uint8_t _invalid_k = ZLANG_TRAP_INVALID_ARGUMENT;
    const uint8_t _division_k = ZLANG_TRAP_DIVISION_BY_ZERO;

    ZLANG_CHECK(context, _overflow_k == 1U);
    ZLANG_CHECK(context, _bounds_k == 2U);
    ZLANG_CHECK(context, _capacity_k == 3U);
    ZLANG_CHECK(context, _invalid_k == 4U);
    ZLANG_CHECK(context, _division_k == 5U);
}

static void checked_addition_returns_the_sum(zlang_test_context_t *const context) {
    ZLANG_CHECK(context, zlang_add_i32(1, 2) == 3);
    ZLANG_CHECK(context, zlang_add_i32(-5, 2) == -3);
    ZLANG_CHECK(context, zlang_add_u32(10U, 20U) == 30U);
}

static void arithmetic_helpers_compute(zlang_test_context_t *const context) {
    ZLANG_CHECK(context, zlang_sub_i32(10, 3) == 7);
    ZLANG_CHECK(context, zlang_sub_u32(10U, 3U) == 7U);
    ZLANG_CHECK(context, zlang_mul_i32(-3, 4) == -12);
    ZLANG_CHECK(context, zlang_mul_u32(3U, 4U) == 12U);
    ZLANG_CHECK(context, zlang_div_i32(-12, 4) == -3);
    ZLANG_CHECK(context, zlang_div_u32(12U, 4U) == 3U);
    ZLANG_CHECK(context, zlang_mod_i32(-13, 4) == -1);
    ZLANG_CHECK(context, zlang_mod_u32(13U, 4U) == 1U);
}

static void string_equality_helpers_compare_content(zlang_test_context_t *const context) {
    const zlang_string_t _left_k = {"abc", 3U};
    const zlang_string_t _right_k = {"abd", 3U};

    ZLANG_CHECK(context, zlang_string_equal(_left_k, _left_k));
    ZLANG_CHECK(context, !zlang_string_equal(_left_k, _right_k));
    ZLANG_CHECK(context, zlang_string_equal_compare(_left_k, _right_k) == -1);
    ZLANG_CHECK(context, zlang_string_equal_compare(_right_k, (zlang_string_t){"abc", 3U}) == 1);
}

static void numeric_helpers_compute(zlang_test_context_t *const context) {
    ZLANG_CHECK(context, zlang_add_i64(9000000000LL, 1LL) == 9000000001LL);
    ZLANG_CHECK(context, zlang_add_u64(10ULL, 20ULL) == 30ULL);
    ZLANG_CHECK(context, zlang_sub_i64(10LL, 3LL) == 7LL);
    ZLANG_CHECK(context, zlang_sub_u64(10ULL, 3ULL) == 7ULL);
    ZLANG_CHECK(context, zlang_mul_i64(-3LL, 4LL) == -12LL);
    ZLANG_CHECK(context, zlang_mul_u64(3ULL, 4ULL) == 12ULL);
    ZLANG_CHECK(context, zlang_div_i64(-12LL, 4LL) == -3LL);
    ZLANG_CHECK(context, zlang_div_u64(12ULL, 4ULL) == 3ULL);
    ZLANG_CHECK(context, zlang_mod_i64(-13LL, 4LL) == -1LL);
    ZLANG_CHECK(context, zlang_mod_u64(13ULL, 4ULL) == 1ULL);
}

static void numeric_conversions_compute(zlang_test_context_t *const context) {
    ZLANG_CHECK(context, zlang_to_i32_i64(7LL) == 7);
    ZLANG_CHECK(context, zlang_to_i32_u32(7U) == 7);
    ZLANG_CHECK(context, zlang_to_i32_f64(3.75) == 3);
    ZLANG_CHECK(context, zlang_to_u32_i64(7LL) == 7U);
    ZLANG_CHECK(context, zlang_to_u32_u32(7U) == 7U);
    ZLANG_CHECK(context, zlang_to_u32_f64(3.75) == 3U);
    ZLANG_CHECK(context, zlang_to_i32_u64(7ULL) == 7);
    ZLANG_CHECK(context, zlang_to_u32_u64(7ULL) == 7U);
    ZLANG_CHECK(context, zlang_to_i64_i64(7LL) == 7LL);
    ZLANG_CHECK(context, zlang_to_i64_u64(7ULL) == 7LL);
    ZLANG_CHECK(context, zlang_to_i64_f64(3.75) == 3LL);
    ZLANG_CHECK(context, zlang_to_u64_i64(7LL) == 7ULL);
    ZLANG_CHECK(context, zlang_to_u64_u64(7ULL) == 7ULL);
    ZLANG_CHECK(context, zlang_to_u64_f64(3.75) == 3ULL);
}

static void checked_index_reads_return_values(zlang_test_context_t *const context) {
    const int32_t _signed_k[] = {7, 9};
    const uint32_t _unsigned_k[] = {3U, 4U};
    const bool _boolean_k[] = {true, false};
    const zlang_string_t _strings_k[] = {{"a", 1U}, {"bc", 2U}};
    const zlang_string_t _text_k = {"abc", 3U};

    ZLANG_CHECK(context, zlang_array_get_i32(_signed_k, 1U, 2U) == 9);
    ZLANG_CHECK(context, zlang_array_get_u32(_unsigned_k, 0U, 2U) == 3U);
    ZLANG_CHECK(context, zlang_array_get_bool(_boolean_k, 0U, 2U));
    ZLANG_CHECK(context, zlang_array_get_string(_strings_k, 1U, 2U).length_ == 2U);
    ZLANG_CHECK(context, zlang_string_byte(_text_k, 1U) == (uint32_t)'b');
}

static void array_append_writes_and_increments_length(zlang_test_context_t *const context) {
    int32_t _data[2] = {0, 0};
    uint32_t _length = 0U;
    const int32_t _first_k = 7;
    const int32_t _second_k = 9;

    zlang_array_append(_data, &_length, 2U, sizeof(_data[0]), &_first_k);
    zlang_array_append(_data, &_length, 2U, sizeof(_data[0]), &_second_k);

    ZLANG_CHECK(context, _length == 2U);
    ZLANG_CHECK(context, _data[0] == 7);
    ZLANG_CHECK(context, _data[1] == 9);
}

static void string_concat_copies_bounded_parts(zlang_test_context_t *const context) {
    char _storage[32] = {0};
    const zlang_string_t _parts_k[] = {{"temp_", 5U}, {"sensor", 6U}};
    const zlang_string_t _result_k = zlang_string_concat(_storage, 31U, _parts_k, 2U);
    const zlang_string_t _empty_k = {NULL, 0U};
    const zlang_string_t _empty_result_k = zlang_string_concat(NULL, 0U, &_empty_k, 1U);

    ZLANG_CHECK(context, _result_k.length_ == 11U);
    ZLANG_CHECK(context, strcmp(_result_k.data_, "temp_sensor") == 0);
    ZLANG_CHECK(context, _storage[11] == '\0');
    ZLANG_CHECK(context, _empty_result_k.length_ == 0U);
    ZLANG_CHECK(context, _empty_result_k.data_ == NULL);
}

#ifdef ZLANG_COVERAGE
static void trap_direct(void) { zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT); }

static void trap_overflow_i32(void) { (void)zlang_add_i32(INT32_MAX, 1); }

static void trap_overflow_u32(void) { (void)zlang_add_u32(UINT32_MAX, 1U); }

static void trap_append_capacity(void) {
    int32_t _data[1] = {0};
    uint32_t _length = 1U;
    const int32_t _value_k = 7;

    zlang_array_append(_data, &_length, 1U, sizeof(_data[0]), &_value_k);
}

static void trap_string_concat_capacity(void) {
    char _storage[4] = {0};
    const zlang_string_t _parts_k[] = {{"abcd", 4U}};

    (void)zlang_string_concat(_storage, 3U, _parts_k, 1U);
}

static void trap_string_concat_invalid(void) {
    char _storage[4] = {0};
    const zlang_string_t _parts_k[] = {{NULL, 1U}};

    (void)zlang_string_concat(_storage, 4U, _parts_k, 1U);
}

static void trap_string_concat_overflow(void) {
    char _storage[4] = {0};
    const zlang_string_t _parts_k[] = {{"x", UINT32_MAX}, {"y", 1U}};

    (void)zlang_string_concat(_storage, 4U, _parts_k, 2U);
}

static void trap_string_concat_null_storage(void) {
    const zlang_string_t _parts_k[] = {{"abcd", 4U}};

    (void)zlang_string_concat(NULL, 4U, _parts_k, 1U);
}

static void trap_division_i32(void) { (void)zlang_div_i32(1, 0); }

static void trap_division_u32(void) { (void)zlang_div_u32(1U, 0U); }

static void trap_remainder_i32(void) { (void)zlang_mod_i32(1, 0); }

static void trap_remainder_u32(void) { (void)zlang_mod_u32(1U, 0U); }

static void trap_array_get_i32(void) {
    const int32_t _data_k[] = {1};
    (void)zlang_array_get_i32(_data_k, 1U, 1U);
}

static void trap_array_get_u32(void) {
    const uint32_t _data_k[] = {1U};
    (void)zlang_array_get_u32(_data_k, 1U, 1U);
}

static void trap_array_get_bool(void) {
    const bool _data_k[] = {true};
    (void)zlang_array_get_bool(_data_k, 1U, 1U);
}

static void trap_array_get_string(void) {
    const zlang_string_t _data_k[] = {{"a", 1U}};
    (void)zlang_array_get_string(_data_k, 1U, 1U);
}

static void trap_string_byte(void) {
    const zlang_string_t _text_k = {"a", 1U};
    (void)zlang_string_byte(_text_k, 1U);
}

static bool trap_exits_with_code(void (*function)(void), uint8_t code) {
    const pid_t _child_k = fork();
    int _status_k = 0;

    if (_child_k == 0) {
        function();
        _exit(0);
    }
    if (_child_k < 0 || waitpid(_child_k, &_status_k, 0) < 0) {
        return false;
    }
    return WIFEXITED(_status_k) && WEXITSTATUS(_status_k) == (128 + (int)code);
}
#endif

static void traps_terminate_with_the_declared_code(zlang_test_context_t *const context) {
#ifdef ZLANG_COVERAGE
    ZLANG_CHECK(context, trap_exits_with_code(trap_direct, ZLANG_TRAP_INVALID_ARGUMENT));
    ZLANG_CHECK(context, trap_exits_with_code(trap_overflow_i32, ZLANG_TRAP_OVERFLOW));
    ZLANG_CHECK(context, trap_exits_with_code(trap_overflow_u32, ZLANG_TRAP_OVERFLOW));
    ZLANG_CHECK(context, trap_exits_with_code(trap_append_capacity, ZLANG_TRAP_CAPACITY));
    ZLANG_CHECK(context, trap_exits_with_code(trap_string_concat_capacity, ZLANG_TRAP_CAPACITY));
    ZLANG_CHECK(context,
                trap_exits_with_code(trap_string_concat_invalid, ZLANG_TRAP_INVALID_ARGUMENT));
    ZLANG_CHECK(context, trap_exits_with_code(trap_string_concat_overflow, ZLANG_TRAP_OVERFLOW));
    ZLANG_CHECK(context,
                trap_exits_with_code(trap_string_concat_null_storage, ZLANG_TRAP_INVALID_ARGUMENT));
    ZLANG_CHECK(context, trap_exits_with_code(trap_division_i32, ZLANG_TRAP_DIVISION_BY_ZERO));
    ZLANG_CHECK(context, trap_exits_with_code(trap_division_u32, ZLANG_TRAP_DIVISION_BY_ZERO));
    ZLANG_CHECK(context, trap_exits_with_code(trap_remainder_i32, ZLANG_TRAP_DIVISION_BY_ZERO));
    ZLANG_CHECK(context, trap_exits_with_code(trap_remainder_u32, ZLANG_TRAP_DIVISION_BY_ZERO));
    ZLANG_CHECK(context, trap_exits_with_code(trap_array_get_i32, ZLANG_TRAP_BOUNDS));
    ZLANG_CHECK(context, trap_exits_with_code(trap_array_get_u32, ZLANG_TRAP_BOUNDS));
    ZLANG_CHECK(context, trap_exits_with_code(trap_array_get_bool, ZLANG_TRAP_BOUNDS));
    ZLANG_CHECK(context, trap_exits_with_code(trap_array_get_string, ZLANG_TRAP_BOUNDS));
    ZLANG_CHECK(context, trap_exits_with_code(trap_string_byte, ZLANG_TRAP_BOUNDS));
#else
    ZLANG_CHECK(context, true);
#endif
}

#define ZLANG_TEST_CASE(name) {#name, name},
static const zlang_test_case_t cases_k[] = {
#include "test_cases.h"
};
#undef ZLANG_TEST_CASE

int main(const int argc, char **const argv) {
    return zlang_test_main(argc, argv, "zlang.runtime", cases_k,
                           sizeof(cases_k) / sizeof(cases_k[0]));
}
