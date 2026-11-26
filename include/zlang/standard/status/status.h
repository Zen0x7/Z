#ifndef ZLANG_STANDARD_STATUS_STATUS_H
#define ZLANG_STANDARD_STATUS_STATUS_H

#include "zlang/detail/inline.h"
#include "zlang/runtime/contract.h"
#include <stdbool.h>
#include <stdint.h>

/** @file
 * @brief Canonical status codes shared by native Zlang facilities.
 */

typedef enum zlang_std_status_t {
    zlang_std_status_success_ = 0U,
    zlang_std_status_end_of_input_ = 1U,
    zlang_std_status_io_error_ = 2U,
    zlang_std_status_invalid_argument_ = 3U,
    zlang_std_status_capacity_ = 4U
} zlang_std_status_t;

ZLANG_STATIC_INLINE zlang_string_t zlang_std_status_message(const zlang_std_status_t code) {
    switch (code) {
    case zlang_std_status_success_:
        return (zlang_string_t){"success", 7U};
    case zlang_std_status_end_of_input_:
        return (zlang_string_t){"end of input", 12U};
    case zlang_std_status_io_error_:
        return (zlang_string_t){"input/output error", 18U};
    case zlang_std_status_invalid_argument_:
        return (zlang_string_t){"invalid argument", 16U};
    case zlang_std_status_capacity_:
        return (zlang_string_t){"capacity exceeded", 17U};
    default:
        return (zlang_string_t){"unknown status", 14U};
    }
}

ZLANG_STATIC_INLINE bool zlang_std_status_is_ok(const zlang_std_status_t code) {
    return code == zlang_std_status_success_;
}

#endif /* ZLANG_STANDARD_STATUS_STATUS_H */
