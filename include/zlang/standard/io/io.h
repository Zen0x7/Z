#ifndef ZLANG_STANDARD_IO_IO_H
#define ZLANG_STANDARD_IO_IO_H

#include "zlang/detail/inline.h"
#include "zlang/runtime/contract.h"
#include "zlang/standard/io/stream.h"
#include "zlang/standard/result/result.h"
#include "zlang/standard/status/status.h"

#include <stdint.h>
#include <stdio.h>

/** @file
 * @brief Native IO helpers built on the stable Zlang stream contract.
 *
 * Every operation returns `result_type<int32, status_code>` with the transferred byte count in
 * `value`. Streams and strings are borrowed views; buffer bytes are never copied except into the
 * caller-owned destination supplied to a read.
 */

#if defined(ZLANG_STD_IO_WRITE_STRING) || defined(ZLANG_STD_IO_WRITE_BOOL) ||                      \
    defined(ZLANG_STD_IO_WRITE_I32) || defined(ZLANG_STD_IO_WRITE_U32) ||                          \
    defined(ZLANG_STD_IO_WRITE_I64) || defined(ZLANG_STD_IO_WRITE_U64) ||                          \
    defined(ZLANG_STD_IO_WRITE_F32) || defined(ZLANG_STD_IO_WRITE_F64)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_stream(const zlang_stream_t destination, const zlang_string_t *source) {
    ZLANG_STD_TIMING_BEGIN();
    zlang_std_status_t _status_k = zlang_std_status_success_;
    const int32_t _written_k = zlang_stream_write(destination, source, &_status_k);
    return ZLANG_STD_TIMED_CALL(i32, zlang_std_result_i32(_status_k, _written_k));
}
#endif

#if defined(ZLANG_STD_IO_WRITE_STRING)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_string(const zlang_string_t source) {
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &source);
}

ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_string_stream(const zlang_stream_t destination, const zlang_string_t source) {
    return zlang_std_io_write_stream(destination, &source);
}

ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_string_window(const zlang_stream_t destination, const zlang_string_t source,
                                 const uint32_t offset, const uint32_t count) {
    if (offset > source.length_ || count > source.length_ - offset) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    const zlang_string_t _window_k = {source.data_ + offset, count};
    return zlang_std_io_write_stream(destination, &_window_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_BOOL)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_io_write_bool(const bool value) {
    const zlang_string_t _source_k = {value ? "true" : "false", value ? 4U : 5U};
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &_source_k);
}

ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_bool_stream(const zlang_stream_t destination, const bool value) {
    const zlang_string_t _source_k = {value ? "true" : "false", value ? 4U : 5U};
    return zlang_std_io_write_stream(destination, &_source_k);
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
    const zlang_string_t _source_k = {_digits_k + _position_k,
                                      (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &_source_k);
}

ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_i32_stream(const zlang_stream_t destination, const int32_t value) {
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
    const zlang_string_t _source_k = {_digits_k + _position_k,
                                      (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(destination, &_source_k);
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
    const zlang_string_t _source_k = {_digits_k + _position_k,
                                      (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &_source_k);
}

ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_u32_stream(const zlang_stream_t destination, const uint32_t value) {
    char _digits_k[16];
    size_t _position_k = sizeof(_digits_k) - 1U;
    uint32_t _magnitude_k = value;
    _digits_k[_position_k] = '\0';
    do {
        _digits_k[--_position_k] = (char)('0' + (_magnitude_k % 10U));
        _magnitude_k /= 10U;
    } while (_magnitude_k != 0U);
    const zlang_string_t _source_k = {_digits_k + _position_k,
                                      (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(destination, &_source_k);
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
    const zlang_string_t _source_k = {_digits_k + _position_k,
                                      (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &_source_k);
}

ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_i64_stream(const zlang_stream_t destination, const int64_t value) {
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
    const zlang_string_t _source_k = {_digits_k + _position_k,
                                      (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(destination, &_source_k);
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
    const zlang_string_t _source_k = {_digits_k + _position_k,
                                      (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &_source_k);
}

ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_u64_stream(const zlang_stream_t destination, const uint64_t value) {
    char _digits_k[24];
    size_t _position_k = sizeof(_digits_k) - 1U;
    uint64_t _magnitude_k = value;
    _digits_k[_position_k] = '\0';
    do {
        _digits_k[--_position_k] = (char)('0' + (_magnitude_k % 10U));
        _magnitude_k /= 10U;
    } while (_magnitude_k != 0U);
    const zlang_string_t _source_k = {_digits_k + _position_k,
                                      (uint32_t)(sizeof(_digits_k) - 1U - _position_k)};
    return zlang_std_io_write_stream(destination, &_source_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_F32)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_io_write_f32(const float value) {
    ZLANG_STD_TIMING_BEGIN();
    char _digits_k[64];
    const int _written_k = snprintf(_digits_k, sizeof(_digits_k), "%.9g", (double)value);
    if (_written_k < 0 || (size_t)_written_k >= sizeof(_digits_k)) {
        return ZLANG_STD_TIMED_CALL(i32, zlang_std_result_i32(zlang_std_status_io_error_, 0));
    }
    const zlang_string_t _source_k = {_digits_k, (uint32_t)_written_k};
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &_source_k);
}

ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_f32_stream(const zlang_stream_t destination, const float value) {
    ZLANG_STD_TIMING_BEGIN();
    char _digits_k[64];
    const int _written_k = snprintf(_digits_k, sizeof(_digits_k), "%.9g", (double)value);
    if (_written_k < 0 || (size_t)_written_k >= sizeof(_digits_k)) {
        return ZLANG_STD_TIMED_CALL(i32, zlang_std_result_i32(zlang_std_status_io_error_, 0));
    }
    const zlang_string_t _source_k = {_digits_k, (uint32_t)_written_k};
    return zlang_std_io_write_stream(destination, &_source_k);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_F64)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_io_write_f64(const double value) {
    ZLANG_STD_TIMING_BEGIN();
    char _digits_k[64];
    const int _written_k = snprintf(_digits_k, sizeof(_digits_k), "%.17g", value);
    if (_written_k < 0 || (size_t)_written_k >= sizeof(_digits_k)) {
        return ZLANG_STD_TIMED_CALL(i32, zlang_std_result_i32(zlang_std_status_io_error_, 0));
    }
    const zlang_string_t _source_k = {_digits_k, (uint32_t)_written_k};
    return zlang_std_io_write_stream(zlang_std_io_stream_stdout(), &_source_k);
}

ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_write_f64_stream(const zlang_stream_t destination, const double value) {
    ZLANG_STD_TIMING_BEGIN();
    char _digits_k[64];
    const int _written_k = snprintf(_digits_k, sizeof(_digits_k), "%.17g", value);
    if (_written_k < 0 || (size_t)_written_k >= sizeof(_digits_k)) {
        return ZLANG_STD_TIMED_CALL(i32, zlang_std_result_i32(zlang_std_status_io_error_, 0));
    }
    const zlang_string_t _source_k = {_digits_k, (uint32_t)_written_k};
    return zlang_std_io_write_stream(destination, &_source_k);
}
#endif

#if defined(ZLANG_STD_IO_READ) || defined(ZLANG_STD_IO_READ_LINE)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_read_impl(const zlang_stream_t source, zlang_string_t *view, char *storage,
                       uint32_t capacity, uint32_t offset, uint32_t count, bool keep_terminator) {
    ZLANG_STD_TIMING_BEGIN();
    zlang_std_status_t _status_k = zlang_std_status_success_;
    const int32_t _read_k =
        zlang_stream_read(source, storage, capacity, offset, count, keep_terminator, &_status_k);
    const uint32_t _end_k = offset + (uint32_t)_read_k;
    view->data_ = storage;
    if (view->length_ < _end_k) {
        view->length_ = _end_k;
    }
    return ZLANG_STD_TIMED_CALL(i32, zlang_std_result_i32(_status_k, _read_k));
}
#endif

#if defined(ZLANG_STD_IO_READ)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_io_read(zlang_string_t *view,
                                                                    char *storage,
                                                                    uint32_t capacity) {
    return zlang_std_io_read_impl(zlang_std_io_stream_stdin(), view, storage, capacity, 0U,
                                  capacity, true);
}

ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_read_stream(const zlang_stream_t source, zlang_string_t *view, char *storage,
                         uint32_t capacity, uint32_t offset, uint32_t count) {
    return zlang_std_io_read_impl(source, view, storage, capacity, offset, count, true);
}
#endif

#if defined(ZLANG_STD_IO_READ_LINE)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_io_read_line(zlang_string_t *view,
                                                                         char *storage,
                                                                         uint32_t capacity) {
    return zlang_std_io_read_impl(zlang_std_io_stream_stdin(), view, storage, capacity, 0U,
                                  capacity, false);
}

ZLANG_STATIC_INLINE zlang_std_result_i32_status_t
zlang_std_io_read_line_stream(const zlang_stream_t source, zlang_string_t *view, char *storage,
                              uint32_t capacity, uint32_t offset, uint32_t count) {
    return zlang_std_io_read_impl(source, view, storage, capacity, offset, count, false);
}
#endif

#endif /* ZLANG_STANDARD_IO_IO_H */
