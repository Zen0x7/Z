#ifndef ZLANG_STANDARD_IO_STREAM_H
#define ZLANG_STANDARD_IO_STREAM_H

#include "zlang/detail/inline.h"

#include <stdio.h>

/** @file
 * @brief Borrowed C stream handle used by Zlang native IO facilities.
 *
 * A stream is a trivially copyable handle. It does not own the underlying stream and performs no
 * allocation. The stdio variants wrap `FILE *`; future facilities can extend the handle without
 * changing Zlang signatures.
 */

typedef enum zlang_stream_kind_t {
    zlang_stream_stdio_ = 0U,
    zlang_stream_invalid_
} zlang_stream_kind_t;

typedef struct zlang_stream_t {
    zlang_stream_kind_t kind_;
    FILE *file_;
} zlang_stream_t;

ZLANG_STATIC_INLINE zlang_stream_t zlang_std_io_stream_stdin(void) {
    zlang_stream_t _stream_k = {zlang_stream_stdio_, stdin};
    return _stream_k;
}

ZLANG_STATIC_INLINE zlang_stream_t zlang_std_io_stream_stdout(void) {
    zlang_stream_t _stream_k = {zlang_stream_stdio_, stdout};
    return _stream_k;
}

ZLANG_STATIC_INLINE zlang_stream_t zlang_std_io_stream_stderr(void) {
    zlang_stream_t _stream_k = {zlang_stream_stdio_, stderr};
    return _stream_k;
}

ZLANG_STATIC_INLINE FILE *zlang_std_io_stream_file(const zlang_stream_t stream) {
    return stream.kind_ == zlang_stream_stdio_ ? stream.file_ : NULL;
}

#endif /* ZLANG_STANDARD_IO_STREAM_H */
