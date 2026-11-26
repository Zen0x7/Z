#ifndef ZLANG_STANDARD_IO_IO_H
#define ZLANG_STANDARD_IO_IO_H

#include "zlang/detail/inline.h"
#include "zlang/runtime/runtime.h"
#include "zlang/standard/io/stream.h"
#include "zlang/standard/result/result.h"

#include <stdint.h>

/** @file
 * @brief Native IO helpers for Zlang.
 *
 * Every operation returns `Result<int32, Status>` with the transferred byte count in `value`.
 * Stream and string arguments are borrowed views; no buffer bytes are copied.
 */

#if defined(ZLANG_STD_IO_WRITE_STRING) || defined(ZLANG_STD_IO_WRITE_BOOL) ||                      \
    defined(ZLANG_STD_IO_WRITE_I32) || defined(ZLANG_STD_IO_WRITE_U32) ||                          \
    defined(ZLANG_STD_IO_WRITE_I64) || defined(ZLANG_STD_IO_WRITE_U64) ||                          \
    defined(ZLANG_STD_IO_WRITE_F32) || defined(ZLANG_STD_IO_WRITE_F64)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_stream(const zlang_stream_t stream, const zlang_string_t *value) {
    if (value == NULL) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    FILE *const _file_k = zlang_std_io_stream_file(stream);
    if (_file_k == NULL || (value->data_ == NULL && value->length_ != 0U)) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value->length_ == 0U) {
        return zlang_std_result_i32(ZLANG_STD_STATUS_SUCCESS, 0);
    }
    const size_t _written_k = fwrite(value->data_, 1U, value->length_, _file_k);
    const zlang_std_status_t _code_k =
        _written_k == value->length_ ? ZLANG_STD_STATUS_SUCCESS : ZLANG_STD_STATUS_IO_ERROR;
    return zlang_std_result_i32(_code_k, (int32_t)_written_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_STRING)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_string(const zlang_string_t value) {
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &value);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_BOOL)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_io_write_bool(const bool value) {
    const zlang_string_t _view_k = {value ? "true" : "false", value ? 4U : 5U};
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &_view_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_I32)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_io_write_i32(const int32_t value) {
    char _digits_k[16];
    size_t _position_k = sizeof(_digits_k) - 1U;
    uint32_t _magnitude_k = value < 0 ? (uint32_t)(-(int64_t)value) : (uint32_t)value;
    _digits_k[_position_k] = '\0';
    do {
        _digits_k[--_position_k] = (char)('0' + (_magnitude_k % 10U));
        _magnitude_k /= 10U;
    } while (_magnitude_k != 0U);
    if (value < 0) {
        _digits_k[--_position_k] = '-';
    }
    const zlang_string_t _view_k = {_digits_k + _position_k,
                                    (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &_view_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_U32)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_io_write_u32(const uint32_t value) {
    char _digits_k[16];
    size_t _position_k = sizeof(_digits_k) - 1U;
    uint32_t _magnitude_k = value;
    _digits_k[_position_k] = '\0';
    do {
        _digits_k[--_position_k] = (char)('0' + (_magnitude_k % 10U));
        _magnitude_k /= 10U;
    } while (_magnitude_k != 0U);
    const zlang_string_t _view_k = {_digits_k + _position_k,
                                    (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &_view_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_I64)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_io_write_i64(const int64_t value) {
    char _digits_k[24];
    size_t _position_k = sizeof(_digits_k) - 1U;
    uint64_t _magnitude_k = value < 0 ? (uint64_t)(-(value + 1)) + 1U : (uint64_t)value;
    _digits_k[_position_k] = '\0';
    do {
        _digits_k[--_position_k] = (char)('0' + (_magnitude_k % 10U));
        _magnitude_k /= 10U;
    } while (_magnitude_k != 0U);
    if (value < 0) {
        _digits_k[--_position_k] = '-';
    }
    const zlang_string_t _view_k = {_digits_k + _position_k,
                                    (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &_view_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_U64)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_io_write_u64(const uint64_t value) {
    char _digits_k[24];
    size_t _position_k = sizeof(_digits_k) - 1U;
    uint64_t _magnitude_k = value;
    _digits_k[_position_k] = '\0';
    do {
        _digits_k[--_position_k] = (char)('0' + (_magnitude_k % 10U));
        _magnitude_k /= 10U;
    } while (_magnitude_k != 0U);
    const zlang_string_t _view_k = {_digits_k + _position_k,
                                    (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &_view_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_F32)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_io_write_f32(const float value) {
    char _digits_k[64];
    const int _written_k = snprintf(_digits_k, sizeof(_digits_k), "%.9g", (double)value);
    if (_written_k < 0 || (size_t)_written_k >= sizeof(_digits_k)) {
        return zlang_std_result_i32(ZLANG_STD_STATUS_IO_ERROR, 0);
    }
    const zlang_string_t _view_k = {_digits_k, (uint32_t)_written_k};
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &_view_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_F64)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_io_write_f64(const double value) {
    char _digits_k[64];
    const int _written_k = snprintf(_digits_k, sizeof(_digits_k), "%.17g", value);
    if (_written_k < 0 || (size_t)_written_k >= sizeof(_digits_k)) {
        return zlang_std_result_i32(ZLANG_STD_STATUS_IO_ERROR, 0);
    }
    const zlang_string_t _view_k = {_digits_k, (uint32_t)_written_k};
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &_view_k);
}
#endif

#if defined(ZLANG_STD_IO_READ) || defined(ZLANG_STD_IO_READ_LINE)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_read_impl(zlang_stream_t stream, zlang_string_t *view, char *storage,
                       uint32_t capacity, uint32_t offset, uint32_t count, bool keep_terminator) {
    if (view == NULL || storage == NULL || capacity == 0U || offset > capacity ||
        count > capacity - offset) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    FILE *const _file_k = zlang_std_io_stream_file(stream);
    /* cppcheck-suppress nullPointerRedundantCheck */
    if (_file_k == NULL) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    uint32_t _written_k = 0U;
    /* cppcheck-suppress nullPointerRedundantCheck */
    int _character_k = getc(_file_k);
    while (_character_k != EOF && _character_k != '\n' && _written_k < count) {
        storage[offset + _written_k++] = (char)_character_k;
        _character_k = getc(_file_k);
    }
    const bool _failed_k = ferror(_file_k);
    const bool _newline_k = _character_k == '\n';
    if (_newline_k && keep_terminator && _written_k < count) {
        storage[offset + _written_k++] = '\n';
    }
    const uint32_t _end_k = offset + _written_k;
    storage[_end_k] = '\0';
    view->data_ = storage;
    if (view->length_ < _end_k) {
        view->length_ = _end_k;
    }
    if (_failed_k) {
        return zlang_std_result_i32(ZLANG_STD_STATUS_IO_ERROR, (int32_t)_written_k);
    }
    if (_written_k == 0U && !_newline_k) {
        return zlang_std_result_i32(ZLANG_STD_STATUS_END_OF_INPUT, 0);
    }
    return zlang_std_result_i32(ZLANG_STD_STATUS_SUCCESS, (int32_t)_written_k);
}
#endif

#if defined(ZLANG_STD_IO_READ)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_io_read(zlang_string_t *view,
                                                                    char *storage,
                                                                    uint32_t capacity) {
    return zlang_std_io_read_impl(zlang_std_io_stream_stdin(), view, storage, capacity, 0U,
                                  capacity, true);
}
#endif

#if defined(ZLANG_STD_IO_READ_LINE)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_io_read_line(zlang_string_t *view,
                                                                         char *storage,
                                                                         uint32_t capacity) {
    return zlang_std_io_read_impl(zlang_std_io_stream_stdin(), view, storage, capacity, 0U,
                                  capacity, false);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_STRING)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_string_stream(const zlang_stream_t stream, const zlang_string_t value) {
    return zlang_std_io_write_stream(stream, &value);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_BOOL)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_bool_stream(const zlang_stream_t stream, const bool value) {
    const zlang_string_t _view_k = {value ? "true" : "false", value ? 4U : 5U};
    return zlang_std_io_write_stream(stream, &_view_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_I32)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_i32_stream(const zlang_stream_t stream, const int32_t value) {
    char _digits_k[16];
    size_t _position_k = sizeof(_digits_k) - 1U;
    uint32_t _magnitude_k = value < 0 ? (uint32_t)(-(int64_t)value) : (uint32_t)value;
    _digits_k[_position_k] = '\0';
    do {
        _digits_k[--_position_k] = (char)('0' + (_magnitude_k % 10U));
        _magnitude_k /= 10U;
    } while (_magnitude_k != 0U);
    if (value < 0) {
        _digits_k[--_position_k] = '-';
    }
    const zlang_string_t _view_k = {_digits_k + _position_k,
                                    (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(stream, &_view_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_U32)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_u32_stream(const zlang_stream_t stream, const uint32_t value) {
    char _digits_k[16];
    size_t _position_k = sizeof(_digits_k) - 1U;
    uint32_t _magnitude_k = value;
    _digits_k[_position_k] = '\0';
    do {
        _digits_k[--_position_k] = (char)('0' + (_magnitude_k % 10U));
        _magnitude_k /= 10U;
    } while (_magnitude_k != 0U);
    const zlang_string_t _view_k = {_digits_k + _position_k,
                                    (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(stream, &_view_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_I64)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_i64_stream(const zlang_stream_t stream, const int64_t value) {
    char _digits_k[24];
    size_t _position_k = sizeof(_digits_k) - 1U;
    uint64_t _magnitude_k = value < 0 ? (uint64_t)(-(value + 1)) + 1U : (uint64_t)value;
    _digits_k[_position_k] = '\0';
    do {
        _digits_k[--_position_k] = (char)('0' + (_magnitude_k % 10U));
        _magnitude_k /= 10U;
    } while (_magnitude_k != 0U);
    if (value < 0) {
        _digits_k[--_position_k] = '-';
    }
    const zlang_string_t _view_k = {_digits_k + _position_k,
                                    (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(stream, &_view_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_U64)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_u64_stream(const zlang_stream_t stream, const uint64_t value) {
    char _digits_k[24];
    size_t _position_k = sizeof(_digits_k) - 1U;
    uint64_t _magnitude_k = value;
    _digits_k[_position_k] = '\0';
    do {
        _digits_k[--_position_k] = (char)('0' + (_magnitude_k % 10U));
        _magnitude_k /= 10U;
    } while (_magnitude_k != 0U);
    const zlang_string_t _view_k = {_digits_k + _position_k,
                                    (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(stream, &_view_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_F32)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_f32_stream(const zlang_stream_t stream, const float value) {
    char _digits_k[64];
    const int _written_k = snprintf(_digits_k, sizeof(_digits_k), "%.9g", (double)value);
    if (_written_k < 0 || (size_t)_written_k >= sizeof(_digits_k)) {
        return zlang_std_result_i32(ZLANG_STD_STATUS_IO_ERROR, 0);
    }
    const zlang_string_t _view_k = {_digits_k, (uint32_t)_written_k};
    return zlang_std_io_write_stream(stream, &_view_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_F64)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_f64_stream(const zlang_stream_t stream, const double value) {
    char _digits_k[64];
    const int _written_k = snprintf(_digits_k, sizeof(_digits_k), "%.17g", value);
    if (_written_k < 0 || (size_t)_written_k >= sizeof(_digits_k)) {
        return zlang_std_result_i32(ZLANG_STD_STATUS_IO_ERROR, 0);
    }
    const zlang_string_t _view_k = {_digits_k, (uint32_t)_written_k};
    return zlang_std_io_write_stream(stream, &_view_k);
}
#endif

#if defined(ZLANG_STD_IO_READ)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_read_stream(const zlang_stream_t stream, zlang_string_t *view, char *storage,
                         uint32_t capacity, uint32_t offset, uint32_t count) {
    return zlang_std_io_read_impl(stream, view, storage, capacity, offset, count, true);
}
#endif

#if defined(ZLANG_STD_IO_READ_LINE)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_read_line_stream(const zlang_stream_t stream, zlang_string_t *view, char *storage,
                              uint32_t capacity, uint32_t offset, uint32_t count) {
    return zlang_std_io_read_impl(stream, view, storage, capacity, offset, count, false);
}
#endif

#endif /* ZLANG_STANDARD_IO_IO_H */
