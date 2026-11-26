#ifndef ZLANG_STANDARD_IO_STREAM_H
#define ZLANG_STANDARD_IO_STREAM_H

#include "zlang/detail/inline.h"
#include "zlang/runtime/contract.h"
#include "zlang/standard/status/status.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/** @file
 * @brief Stable C99 stream contract for every native Zlang IO facility.
 *
 * A stream is a trivially copyable borrowed handle. It owns no storage and performs no allocation.
 * Backends provide read and write operations through a constant operations table. The stdio,
 * memory and future socket, pipe or descriptor backends all use this same contract, so Zlang
 * signatures never need to change when a new backend appears.
 */

typedef int32_t (*zlang_stream_read_fn_t)(void *state, char *storage, uint32_t capacity,
                                          uint32_t offset, uint32_t count, bool keep_terminator,
                                          zlang_std_status_t *status);

typedef int32_t (*zlang_stream_write_fn_t)(void *state, const zlang_string_t *value,
                                           zlang_std_status_t *status);

typedef struct zlang_stream_ops_t {
    zlang_stream_read_fn_t read_;
    zlang_stream_write_fn_t write_;
} zlang_stream_ops_t;

typedef struct zlang_stream_t {
    const zlang_stream_ops_t *ops_;
    void *state_;
} zlang_stream_t;

typedef struct zlang_memory_stream_t {
    char *data_;
    uint32_t capacity_;
    uint32_t length_;
    uint32_t position_;
} zlang_memory_stream_t;

ZLANG_STATIC_INLINE zlang_stream_t zlang_stream_from_ops(const zlang_stream_ops_t *ops,
                                                         void *state) {
    zlang_stream_t _stream_k = {ops, state};
    return _stream_k;
}

ZLANG_STATIC_INLINE int32_t zlang_stream_read(const zlang_stream_t stream, char *storage,
                                              uint32_t capacity, uint32_t offset, uint32_t count,
                                              bool keep_terminator, zlang_std_status_t *status) {
    if (storage == NULL || capacity == 0U || offset > capacity || count > capacity - offset) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    const zlang_stream_read_fn_t _read_k = stream.ops_ != NULL ? stream.ops_->read_ : NULL;
    *status = zlang_std_status_success_;
    if (_read_k != NULL) {
        return _read_k(stream.state_, storage, capacity, offset, count, keep_terminator, status);
    }
    zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    return 0;
}

ZLANG_STATIC_INLINE int32_t zlang_stream_write(const zlang_stream_t stream,
                                               const zlang_string_t *value,
                                               zlang_std_status_t *status) {
    if (value == NULL || (value->data_ == NULL && value->length_ != 0U)) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    const zlang_stream_write_fn_t _write_k = stream.ops_ != NULL ? stream.ops_->write_ : NULL;
    *status = zlang_std_status_success_;
    if (_write_k != NULL) {
        return _write_k(stream.state_, value, status);
    }
    zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    return 0;
}

ZLANG_STATIC_INLINE int32_t zlang_stream_stdio_read(void *state, char *storage, uint32_t capacity,
                                                    uint32_t offset, uint32_t count,
                                                    bool keep_terminator,
                                                    zlang_std_status_t *status) {
    (void)capacity;
    FILE *const _file_k = (FILE *)state;
    if (_file_k == NULL) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
        return 0;
    }
    uint32_t _written_k = 0U;
    int _character_k = getc(_file_k);
    while (_character_k != EOF && _character_k != '\n' && _written_k < count) {
        storage[offset + _written_k++] = (char)_character_k;
        _character_k = getc(_file_k);
    }
    const bool _newline_k = _character_k == '\n';
    if (_newline_k && keep_terminator && _written_k < count) {
        storage[offset + _written_k++] = '\n';
    }
    storage[offset + _written_k] = '\0';
    if (ferror(_file_k)) {
        *status = zlang_std_status_io_error_;
    } else if (_written_k == 0U && !_newline_k) {
        *status = zlang_std_status_end_of_input_;
    }
    return (int32_t)_written_k;
}

ZLANG_STATIC_INLINE int32_t zlang_stream_stdio_write(void *state, const zlang_string_t *value,
                                                     zlang_std_status_t *status) {
    FILE *const _file_k = (FILE *)state;
    if (_file_k == NULL) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (value->length_ == 0U) {
        return 0;
    }
    const size_t _written_k = fwrite(value->data_, 1U, value->length_, _file_k);
    if (_written_k != value->length_) {
        *status = zlang_std_status_io_error_;
    }
    return (int32_t)_written_k;
}

static const zlang_stream_ops_t zlang_stream_stdio_ops_k = {zlang_stream_stdio_read,
                                                            zlang_stream_stdio_write};

ZLANG_STATIC_INLINE zlang_stream_t zlang_std_io_stream_stdin(void) {
    return zlang_stream_from_ops(&zlang_stream_stdio_ops_k, stdin);
}

ZLANG_STATIC_INLINE zlang_stream_t zlang_std_io_stream_stdout(void) {
    return zlang_stream_from_ops(&zlang_stream_stdio_ops_k, stdout);
}

ZLANG_STATIC_INLINE zlang_stream_t zlang_std_io_stream_stderr(void) {
    return zlang_stream_from_ops(&zlang_stream_stdio_ops_k, stderr);
}

ZLANG_STATIC_INLINE int32_t zlang_stream_memory_read(void *state, char *storage, uint32_t capacity,
                                                     uint32_t offset, uint32_t count,
                                                     bool keep_terminator,
                                                     zlang_std_status_t *status) {
    zlang_memory_stream_t *const _stream_k = (zlang_memory_stream_t *)state;
    if (_stream_k == NULL || _stream_k->data_ == NULL ||
        _stream_k->position_ > _stream_k->length_) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
        return 0;
    }
    uint32_t _written_k = 0U;
    while (_stream_k->position_ < _stream_k->length_ && _written_k < count) {
        const char _character_k = _stream_k->data_[_stream_k->position_++];
        if (_character_k == '\n') {
            if (keep_terminator) {
                storage[offset + _written_k++] = '\n';
            }
            break;
        }
        storage[offset + _written_k++] = _character_k;
    }
    storage[offset + _written_k] = '\0';
    if (_written_k == 0U && _stream_k->position_ >= _stream_k->length_) {
        *status = zlang_std_status_end_of_input_;
    }
    (void)capacity;
    return (int32_t)_written_k;
}

ZLANG_STATIC_INLINE int32_t zlang_stream_memory_write(void *state, const zlang_string_t *value,
                                                      zlang_std_status_t *status) {
    zlang_memory_stream_t *const _stream_k = (zlang_memory_stream_t *)state;
    if (_stream_k == NULL || _stream_k->data_ == NULL ||
        _stream_k->length_ > _stream_k->capacity_) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
        return 0;
    }
    if (value->length_ > _stream_k->capacity_ - _stream_k->length_) {
        *status = zlang_std_status_capacity_;
        return 0;
    }
    memcpy(_stream_k->data_ + _stream_k->length_, value->data_, value->length_);
    _stream_k->length_ += value->length_;
    return (int32_t)value->length_;
}

static const zlang_stream_ops_t zlang_stream_memory_ops_k = {zlang_stream_memory_read,
                                                             zlang_stream_memory_write};

ZLANG_STATIC_INLINE zlang_memory_stream_t zlang_memory_stream_init(char *storage, uint32_t capacity,
                                                                   uint32_t initial_length) {
    zlang_memory_stream_t _stream_k = {storage, capacity, initial_length, 0U};
    return _stream_k;
}

ZLANG_STATIC_INLINE zlang_stream_t zlang_std_io_stream_memory(zlang_memory_stream_t *state) {
    return zlang_stream_from_ops(&zlang_stream_memory_ops_k, state);
}

#endif /* ZLANG_STANDARD_IO_STREAM_H */
