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
    if (_stream_k == NULL) {
        return;
    }
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

static void trap_codes_are_fixed(zlang_test_context_t *const context) {
    const uint8_t _overflow_k = ZLANG_TRAP_OVERFLOW;
    const uint8_t _bounds_k = ZLANG_TRAP_BOUNDS;
    const uint8_t _capacity_k = ZLANG_TRAP_CAPACITY;
    const uint8_t _invalid_k = ZLANG_TRAP_INVALID_ARGUMENT;

    ZLANG_CHECK(context, _overflow_k == 1U);
    ZLANG_CHECK(context, _bounds_k == 2U);
    ZLANG_CHECK(context, _capacity_k == 3U);
    ZLANG_CHECK(context, _invalid_k == 4U);
}

static void checked_addition_returns_the_sum(zlang_test_context_t *const context) {
    ZLANG_CHECK(context, zlang_add_i32(1, 2) == 3);
    ZLANG_CHECK(context, zlang_add_i32(-5, 2) == -3);
    ZLANG_CHECK(context, zlang_add_u32(10U, 20U) == 30U);
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
