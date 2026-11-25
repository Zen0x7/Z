#ifndef ZLANG_STANDARD_PROGRAM_OPTIONS_H
#define ZLANG_STANDARD_PROGRAM_OPTIONS_H

#include "zlang/runtime/runtime.h"

#if defined(ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT)
ZLANG_STATIC_INLINE uint32_t
zlang_std_program_options_argument_count(const zlang_arguments_t *arguments) {
    return arguments == NULL || arguments->argv_ == NULL || arguments->argc_ < 0
               ? 0U
               : (uint32_t)arguments->argc_;
}
#endif

#if defined(ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_AT)
ZLANG_STATIC_INLINE zlang_string_t
zlang_std_program_options_argument_at(const zlang_arguments_t *arguments, uint32_t index) {
    const uint32_t _count_k = zlang_std_program_options_argument_count(arguments);
    if (index >= _count_k) {
        zlang_trap(ZLANG_TRAP_BOUNDS);
        return (zlang_string_t){"", 0U};
    }
    const char *const _value_k = arguments->argv_[index];
    return (zlang_string_t){_value_k, (uint32_t)strlen(_value_k)};
}
#endif

#if defined(ZLANG_STD_PROGRAM_OPTIONS_HAS_OPTION) || defined(ZLANG_STD_PROGRAM_OPTIONS_HAS_FLAG)
ZLANG_STATIC_INLINE bool zlang_std_program_options_has(const zlang_arguments_t *arguments,
                                                       zlang_string_t option) {
    const uint32_t _count_k = zlang_std_program_options_argument_count(arguments);
    for (uint32_t _index_k = 1U; _index_k < _count_k; ++_index_k) {
        const char *const _value_k = arguments->argv_[_index_k];
        if (strlen(_value_k) == option.length_ &&
            memcmp(_value_k, option.data_, option.length_) == 0) {
            return true;
        }
    }
    return false;
}
#endif

#if defined(ZLANG_STD_PROGRAM_OPTIONS_HAS_OPTION)
ZLANG_STATIC_INLINE bool zlang_std_program_options_has_option(const zlang_arguments_t *arguments,
                                                              zlang_string_t option) {
    return zlang_std_program_options_has(arguments, option);
}
#endif

#if defined(ZLANG_STD_PROGRAM_OPTIONS_HAS_FLAG)
ZLANG_STATIC_INLINE bool zlang_std_program_options_has_flag(const zlang_arguments_t *arguments,
                                                            zlang_string_t flag) {
    return zlang_std_program_options_has(arguments, flag);
}
#endif

#if defined(ZLANG_STD_PROGRAM_OPTIONS_OPTION_VALUE)
ZLANG_STATIC_INLINE zlang_string_t
zlang_std_program_options_option_value(const zlang_arguments_t *arguments, zlang_string_t option) {
    const uint32_t _count_k = zlang_std_program_options_argument_count(arguments);
    for (uint32_t _index_k = 1U; _index_k + 1U < _count_k; ++_index_k) {
        const char *const _value_k = arguments->argv_[_index_k];
        if (strlen(_value_k) == option.length_ &&
            memcmp(_value_k, option.data_, option.length_) == 0) {
            const char *const _result_k = arguments->argv_[_index_k + 1U];
            return (zlang_string_t){_result_k, (uint32_t)strlen(_result_k)};
        }
    }
    zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    return (zlang_string_t){"", 0U};
}
#endif

#endif
