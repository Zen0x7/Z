#ifndef ZLANG_STANDARD_STRINGS_STRINGS_H
#define ZLANG_STANDARD_STRINGS_STRINGS_H

#include "zlang/detail/inline.h"
#include "zlang/standard/result/result.h"

#if defined(ZLANG_STD_STRINGS_COMPARE)
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_strings_compare(zlang_string_t left,
                                                                            zlang_string_t right) {
    return zlang_std_result_i32(zlang_std_status_success_, zlang_string_equal_compare(left, right));
}
#endif

#if defined(ZLANG_STD_STRINGS_CONTAINS)
ZLANG_STATIC_INLINE zlang_std_result_bool_status_t zlang_std_strings_contains(zlang_string_t value,
                                                                              zlang_string_t part) {
    if ((value.data_ == NULL && value.length_ != 0U) ||
        (part.data_ == NULL && part.length_ != 0U)) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (part.length_ == 0U) {
        return zlang_std_result_bool(zlang_std_status_success_, true);
    }
    if (part.length_ > value.length_) {
        return zlang_std_result_bool(zlang_std_status_success_, false);
    }
    for (uint32_t _index = 0U; _index + part.length_ <= value.length_; ++_index) {
        bool _found_k = true;
        for (uint32_t _offset = 0U; _offset < part.length_; ++_offset) {
            if (value.data_[_index + _offset] != part.data_[_offset]) {
                _found_k = false;
                break;
            }
        }
        if (_found_k) {
            return zlang_std_result_bool(zlang_std_status_success_, true);
        }
    }
    return zlang_std_result_bool(zlang_std_status_success_, false);
}
#endif

#if defined(ZLANG_STD_STRINGS_STARTS_WITH)
ZLANG_STATIC_INLINE zlang_std_result_bool_status_t
zlang_std_strings_starts_with(zlang_string_t value, zlang_string_t prefix) {
    if ((value.data_ == NULL && value.length_ != 0U) ||
        (prefix.data_ == NULL && prefix.length_ != 0U)) {
        zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    }
    if (prefix.length_ > value.length_) {
        return zlang_std_result_bool(zlang_std_status_success_, false);
    }
    for (uint32_t _index = 0U; _index < prefix.length_; ++_index) {
        if (value.data_[_index] != prefix.data_[_index]) {
            return zlang_std_result_bool(zlang_std_status_success_, false);
        }
    }
    return zlang_std_result_bool(zlang_std_status_success_, true);
}
#endif

#endif /* ZLANG_STANDARD_STRINGS_STRINGS_H */
