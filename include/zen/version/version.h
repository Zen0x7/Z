#ifndef ZEN_VERSION_VERSION_H
#define ZEN_VERSION_VERSION_H

/** @file
 *  @brief Compile-time version macros and the runtime version accessor.
 */

#include "zen/detail/inline.h"

#include <stdint.h>

/** Major version of the library, defined at compile time. */
#define LIBZEN_VERSION_MAJOR 0
/** Minor version of the library, defined at compile time. */
#define LIBZEN_VERSION_MINOR 0
/** Patch version of the library, defined at compile time. */
#define LIBZEN_VERSION_PATCH 0

/** Version numbers reported by the library. */
typedef struct zen_version_t {
    uint32_t major_;
    uint32_t minor_;
    uint32_t patch_;
} zen_version_t;

/** Return the version the library was compiled with. */
ZEN_STATIC_INLINE zen_version_t zen_get_version(void) {
    zen_version_t _version;
    _version.major_ = (uint32_t)LIBZEN_VERSION_MAJOR;
    _version.minor_ = (uint32_t)LIBZEN_VERSION_MINOR;
    _version.patch_ = (uint32_t)LIBZEN_VERSION_PATCH;
    return _version;
}

#endif /* ZEN_VERSION_VERSION_H */
