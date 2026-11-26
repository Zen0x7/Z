#define ZLANG_TRAP_SINK(code) ((void)(code))
#define ZLANG_STD_RESULT
#include "zlang/runtime/runtime.h"
#include "zlang/testing/testing.h"

#include <stdio.h>
#include <string.h>

#define ZLANG_TEST_CASE(name) static void name(zlang_test_context_t *const context);
#include "test_cases.h"
#undef ZLANG_TEST_CASE

static const zlang_std_status_t status_codes_k[] = {
    zlang_std_status_success_,          zlang_std_status_end_of_input_, zlang_std_status_io_error_,
    zlang_std_status_invalid_argument_, zlang_std_status_capacity_,     (zlang_std_status_t)99};

static const char *const status_messages_k[] = {
    "success",          "end of input",      "input/output error",
    "invalid argument", "capacity exceeded", "unknown status"};

static void result_constructors_carry_code_value_and_message(zlang_test_context_t *const context) {
    const zlang_std_result_void_status_t _void_result_k =
        zlang_std_result_void(zlang_std_status_success_);
    const zlang_std_result_i32_status_t _i32_result_k =
        zlang_std_result_i32(zlang_std_status_capacity_, -7);
    const zlang_std_result_u32_status_t _u32_result_k =
        zlang_std_result_u32(zlang_std_status_invalid_argument_, 9U);
    const zlang_std_result_i64_status_t _i64_result_k =
        zlang_std_result_i64(zlang_std_status_end_of_input_, -11LL);
    const zlang_std_result_u64_status_t _u64_result_k =
        zlang_std_result_u64(zlang_std_status_io_error_, 13ULL);
    const zlang_std_result_bool_status_t _bool_result_k =
        zlang_std_result_bool(zlang_std_status_success_, true);
    const zlang_std_result_f32_status_t _f32_result_k =
        zlang_std_result_f32(zlang_std_status_success_, 1.5F);
    const zlang_std_result_f64_status_t _f64_result_k =
        zlang_std_result_f64(zlang_std_status_success_, 2.5);
    const zlang_string_t _text_k = {"value", 5U};
    const zlang_std_result_string_status_t _string_result_k =
        zlang_std_result_string(zlang_std_status_success_, _text_k);

    ZLANG_CHECK(context, _void_result_k.code_ == zlang_std_status_success_);
    ZLANG_CHECK(context, _void_result_k.what_.length_ == 7U);
    ZLANG_CHECK(context, memcmp(_void_result_k.what_.data_, "success", 7U) == 0);
    ZLANG_CHECK(context, _i32_result_k.code_ == zlang_std_status_capacity_);
    ZLANG_CHECK(context, _i32_result_k.value_ == -7);
    ZLANG_CHECK(context, _i32_result_k.what_.length_ == 17U);
    ZLANG_CHECK(context, _u32_result_k.code_ == zlang_std_status_invalid_argument_);
    ZLANG_CHECK(context, _u32_result_k.value_ == 9U);
    ZLANG_CHECK(context, _i64_result_k.code_ == zlang_std_status_end_of_input_);
    ZLANG_CHECK(context, _i64_result_k.value_ == -11LL);
    ZLANG_CHECK(context, _u64_result_k.code_ == zlang_std_status_io_error_);
    ZLANG_CHECK(context, _u64_result_k.value_ == 13ULL);
    ZLANG_CHECK(context, _bool_result_k.value_);
    ZLANG_CHECK(context, _f32_result_k.value_ > 1.49F && _f32_result_k.value_ < 1.51F);
    ZLANG_CHECK(context, _f64_result_k.value_ > 2.49 && _f64_result_k.value_ < 2.51);
    ZLANG_CHECK(context, _string_result_k.value_.length_ == 5U);
    ZLANG_CHECK(context, memcmp(_string_result_k.value_.data_, "value", 5U) == 0);
    ZLANG_CHECK(context, _string_result_k.what_.length_ == 7U);
}

static void status_messages_and_predicate_are_complete(zlang_test_context_t *const context) {
    bool _all_ok_k = true;
    for (size_t _index = 0U; _index < sizeof(status_codes_k) / sizeof(status_codes_k[0]);
         ++_index) {
        const zlang_string_t _message_k = zlang_std_status_message(status_codes_k[_index]);
        const size_t _expected_length_k = strlen(status_messages_k[_index]);
        _all_ok_k = _all_ok_k && _message_k.length_ == _expected_length_k &&
                    memcmp(_message_k.data_, status_messages_k[_index], _expected_length_k) == 0;
    }
    ZLANG_CHECK(context, _all_ok_k);
    ZLANG_CHECK(context, zlang_std_status_is_ok(zlang_std_status_success_));
    ZLANG_CHECK(context, !zlang_std_status_is_ok(zlang_std_status_end_of_input_));
    ZLANG_CHECK(context, !zlang_std_status_is_ok((zlang_std_status_t)99));
}

static void stream_backends_share_read_and_write_semantics(zlang_test_context_t *const context) {
    char _storage_k[16] = {0};
    zlang_memory_stream_t _memory_k = zlang_memory_stream_init(_storage_k, sizeof(_storage_k), 0U);
    const zlang_stream_t _memory_stream_k = zlang_std_io_stream_memory(&_memory_k);
    const zlang_string_t _source_k = {"abc", 3U};
    zlang_std_status_t _status_k = zlang_std_status_success_;

    ZLANG_CHECK(context, zlang_stream_write(_memory_stream_k, &_source_k, &_status_k) == 3);
    ZLANG_CHECK(context, _status_k == zlang_std_status_success_);

    _memory_k.position_ = 0U;
    char _destination_k[16] = {0};
    ZLANG_CHECK(context, zlang_stream_read(_memory_stream_k, _destination_k, sizeof(_destination_k),
                                           0U, 2U, false, &_status_k) == 2);
    ZLANG_CHECK(context, memcmp(_destination_k, "ab", 2U) == 0);
    ZLANG_CHECK(context, zlang_stream_read(_memory_stream_k, _destination_k, sizeof(_destination_k),
                                           0U, 8U, false, &_status_k) == 1);
    ZLANG_CHECK(context, zlang_stream_read(_memory_stream_k, _destination_k, sizeof(_destination_k),
                                           0U, 8U, false, &_status_k) == 0);
    ZLANG_CHECK(context, _status_k == zlang_std_status_end_of_input_);

    char _line_storage_k[] = "ab\ncd";
    zlang_memory_stream_t _line_k = zlang_memory_stream_init(_line_storage_k, 5U, 5U);
    const zlang_stream_t _line_stream_k = zlang_std_io_stream_memory(&_line_k);
    ZLANG_CHECK(context, zlang_stream_read(_line_stream_k, _destination_k, sizeof(_destination_k),
                                           0U, 8U, false, &_status_k) == 2);
    ZLANG_CHECK(context, memcmp(_destination_k, "ab", 2U) == 0);
    _line_k.position_ = 2U;
    ZLANG_CHECK(context, zlang_stream_read(_line_stream_k, _destination_k, sizeof(_destination_k),
                                           0U, 8U, true, &_status_k) == 1);
    ZLANG_CHECK(context, _destination_k[0] == '\n');

    char _small_storage_k[4] = {0};
    zlang_memory_stream_t _small_k = zlang_memory_stream_init(_small_storage_k, 4U, 2U);
    const zlang_string_t _large_k = {"abcdef", 6U};
    ZLANG_CHECK(context, zlang_stream_write(zlang_std_io_stream_memory(&_small_k), &_large_k,
                                            &_status_k) == 0);
    ZLANG_CHECK(context, _status_k == zlang_std_status_capacity_);

    const zlang_stream_t _null_ops_k = zlang_stream_from_ops(NULL, NULL);
    ZLANG_CHECK(context, zlang_stream_read(_null_ops_k, _destination_k, sizeof(_destination_k), 0U,
                                           1U, false, &_status_k) == 0);
    ZLANG_CHECK(context, zlang_stream_write(_null_ops_k, &_source_k, &_status_k) == 0);
    ZLANG_CHECK(context, zlang_stream_read(_null_ops_k, NULL, sizeof(_destination_k), 0U, 1U, false,
                                           &_status_k) == 0);
    ZLANG_CHECK(context, zlang_stream_write(_null_ops_k, NULL, &_status_k) == 0);
    ZLANG_CHECK(context, zlang_stream_memory_read(NULL, _destination_k, sizeof(_destination_k), 0U,
                                                  1U, false, &_status_k) == 0);
    ZLANG_CHECK(context, zlang_stream_memory_write(NULL, &_source_k, &_status_k) == 0);
    ZLANG_CHECK(context, zlang_stream_stdio_read(NULL, _destination_k, sizeof(_destination_k), 0U,
                                                 1U, false, &_status_k) == 0);

    FILE *const _file_k = tmpfile();
    bool _stdio_ok_k = _file_k != NULL;
    if (_file_k != NULL) {
        const zlang_stream_t _stdio_k = zlang_stream_from_ops(&zlang_stream_stdio_ops_k, _file_k);
        const zlang_string_t _line_source_k = {"a\nb", 3U};
        const int32_t _write_k = zlang_stream_write(_stdio_k, &_line_source_k, &_status_k);
        const bool _seek_zero_k = fseek(_file_k, 0L, SEEK_SET) == 0;
        const int32_t _first_k = zlang_stream_read(_stdio_k, _destination_k, sizeof(_destination_k),
                                                   0U, 8U, false, &_status_k);
        const bool _first_content_k = _destination_k[0] == 'a';
        const bool _seek_one_k = fseek(_file_k, 1L, SEEK_SET) == 0;
        const int32_t _newline_k = zlang_stream_read(
            _stdio_k, _destination_k, sizeof(_destination_k), 0U, 8U, true, &_status_k);
        const bool _newline_content_k = _destination_k[0] == '\n';
        const int32_t _last_k = zlang_stream_read(_stdio_k, _destination_k, sizeof(_destination_k),
                                                  0U, 8U, false, &_status_k);
        const bool _last_content_k = _destination_k[0] == 'b';
        const int32_t _end_k = zlang_stream_read(_stdio_k, _destination_k, sizeof(_destination_k),
                                                 0U, 8U, false, &_status_k);
        const bool _end_status_k = _status_k == zlang_std_status_end_of_input_;
        const zlang_string_t _empty_k = {NULL, 0U};
        const int32_t _empty_write_k = zlang_stream_write(_stdio_k, &_empty_k, &_status_k);
        _stdio_ok_k = _write_k == 3 && _seek_zero_k && _first_k == 1 && _first_content_k &&
                      _seek_one_k && _newline_k == 1 && _newline_content_k && _last_k == 1 &&
                      _last_content_k && _end_k == 0 && _end_status_k && _empty_write_k == 0;
        (void)fclose(_file_k);
    }
    ZLANG_CHECK(context, _stdio_ok_k);

    ZLANG_CHECK(context, zlang_std_io_stream_stdin().ops_ != NULL);
    ZLANG_CHECK(context, zlang_std_io_stream_stdout().ops_ != NULL);
    ZLANG_CHECK(context, zlang_std_io_stream_stderr().ops_ != NULL);
}

#define ZLANG_TEST_CASE(name) {#name, name},
static const zlang_test_case_t cases_k[] = {
#include "test_cases.h"
};
#undef ZLANG_TEST_CASE

int main(const int argc, char **const argv) {
    return zlang_test_main(argc, argv, "zlang.stdlib.contract", cases_k,
                           sizeof(cases_k) / sizeof(cases_k[0]));
}
