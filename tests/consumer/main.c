#include <zen/zen.h>

int main(void) {
    const zen_version_t _version_k = zen_get_version();
    const int _matches_k = _version_k.major_ == LIBZEN_VERSION_MAJOR &&
                           _version_k.minor_ == LIBZEN_VERSION_MINOR &&
                           _version_k.patch_ == LIBZEN_VERSION_PATCH;

    return _matches_k ? 0 : 1;
}
