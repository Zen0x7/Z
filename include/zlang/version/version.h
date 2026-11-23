#ifndef ZLANG_VERSION_VERSION_H
#define ZLANG_VERSION_VERSION_H

/** @file
 *  @brief Compile-time version macros and the runtime version accessor.
 */

#include "zlang/detail/inline.h"

#include <stdint.h>

/** Major version of the library, defined at compile time. */
#define LIBZLANG_VERSION_MAJOR 0
/** Minor version of the library, defined at compile time. */
#define LIBZLANG_VERSION_MINOR 0
/** Patch version of the library, defined at compile time. */
#define LIBZLANG_VERSION_PATCH 0

/** Version numbers reported by the library. */
typedef struct zlang_version_t {
    uint32_t major_;
    uint32_t minor_;
    uint32_t patch_;
} zlang_version_t;

/** Return the version the library was compiled with. */
ZLANG_STATIC_INLINE zlang_version_t zlang_get_version(void) {
    zlang_version_t _version;
    _version.major_ = (uint32_t)LIBZLANG_VERSION_MAJOR;
    _version.minor_ = (uint32_t)LIBZLANG_VERSION_MINOR;
    _version.patch_ = (uint32_t)LIBZLANG_VERSION_PATCH;
    return _version;
}

#endif /* ZLANG_VERSION_VERSION_H */
