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

#endif /* ZLANG_STANDARD_IO_IO_H */
