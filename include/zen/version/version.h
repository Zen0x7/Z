#ifndef ZEN_VERSION_VERSION_H
#define ZEN_VERSION_VERSION_H

#include "zen/detail/inline.h"

#include <stdint.h>

#define LIBZEN_VERSION_MAJOR 0
#define LIBZEN_VERSION_MINOR 0
#define LIBZEN_VERSION_PATCH 0

typedef struct zen_version_t {
    uint32_t major_;
    uint32_t minor_;
    uint32_t patch_;
} zen_version_t;

ZEN_STATIC_INLINE zen_version_t zen_get_version(void) {
    zen_version_t _version;
    _version.major_ = (uint32_t)LIBZEN_VERSION_MAJOR;
    _version.minor_ = (uint32_t)LIBZEN_VERSION_MINOR;
    _version.patch_ = (uint32_t)LIBZEN_VERSION_PATCH;
    return _version;
}

#endif /* ZEN_VERSION_VERSION_H */
