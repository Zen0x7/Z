#ifndef ZLANG_STANDARD_IO_IO_H
#define ZLANG_STANDARD_IO_IO_H

#include "zlang/detail/inline.h"
#include "zlang/runtime/runtime.h"

#if defined(ZLANG_STD_IO_WRITE_STRING)
ZLANG_STATIC_INLINE bool zlang_std_io_write_string(zlang_string_t value) {
    return zlang_print(value);
}
#endif

#if defined(ZLANG_STD_IO_WRITE_BOOL)
ZLANG_STATIC_INLINE bool zlang_std_io_write_bool(bool value) { return zlang_print_bool(value); }
#endif

#if defined(ZLANG_STD_IO_WRITE_I32)
ZLANG_STATIC_INLINE bool zlang_std_io_write_i32(int32_t value) { return zlang_print_i32(value); }
#endif

#if defined(ZLANG_STD_IO_WRITE_U32)
ZLANG_STATIC_INLINE bool zlang_std_io_write_u32(uint32_t value) { return zlang_print_u32(value); }
#endif

#if defined(ZLANG_STD_IO_WRITE_I64)
ZLANG_STATIC_INLINE bool zlang_std_io_write_i64(int64_t value) { return zlang_print_i64(value); }
#endif

#if defined(ZLANG_STD_IO_WRITE_U64)
ZLANG_STATIC_INLINE bool zlang_std_io_write_u64(uint64_t value) { return zlang_print_u64(value); }
#endif

#if defined(ZLANG_STD_IO_WRITE_F32)
ZLANG_STATIC_INLINE bool zlang_std_io_write_f32(float value) { return zlang_print_f32(value); }
#endif

#if defined(ZLANG_STD_IO_WRITE_F64)
ZLANG_STATIC_INLINE bool zlang_std_io_write_f64(double value) { return zlang_print_f64(value); }
#endif

#if defined(ZLANG_STD_IO_READ) || defined(ZLANG_STD_IO_READ_LINE)
ZLANG_STATIC_INLINE int32_t zlang_std_io_read_impl(zlang_string_t *view, char *storage,
                                                   uint32_t capacity, bool keep_terminator) {
    if (view == NULL || storage == NULL || capacity == 0U) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    uint32_t _length_k = 0U;
    int _character_k = getchar();
    while (_character_k != EOF && _character_k != '\n') {
        if (_length_k >= capacity) {
            zlang_trap(ZLANG_TRAP_CAPACITY);
        }
        storage[_length_k++] = (char)_character_k;
        _character_k = getchar();
    }
    if (_character_k == EOF && _length_k == 0U) {
        view->data_ = storage;
        view->length_ = 0U;
        return -1;
    }
    if (keep_terminator && _character_k == '\n') {
        if (_length_k >= capacity) {
            zlang_trap(ZLANG_TRAP_CAPACITY);
        }
        storage[_length_k++] = '\n';
    }
    storage[_length_k] = '\0';
    view->data_ = storage;
    view->length_ = _length_k;
    return (int32_t)_length_k;
}
#endif

#if defined(ZLANG_STD_IO_READ)
ZLANG_STATIC_INLINE int32_t zlang_std_io_read(zlang_string_t *view, char *storage,
                                              uint32_t capacity) {
    return zlang_std_io_read_impl(view, storage, capacity, true);
}
#endif

#if defined(ZLANG_STD_IO_READ_LINE)
ZLANG_STATIC_INLINE int32_t zlang_std_io_read_line(zlang_string_t *view, char *storage,
                                                   uint32_t capacity) {
    return zlang_std_io_read_impl(view, storage, capacity, false);
}
#endif

#endif /* ZLANG_STANDARD_IO_IO_H */
