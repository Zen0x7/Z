#include <zlang/zlang.h>

int main(void) {
    const zlang_version_t _version_k = zlang_get_version();
    const int _matches_k = _version_k.major_ == LIBZLANG_VERSION_MAJOR &&
                           _version_k.minor_ == LIBZLANG_VERSION_MINOR &&
                           _version_k.patch_ == LIBZLANG_VERSION_PATCH;

    return _matches_k ? 0 : 1;
}
