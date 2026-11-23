#include <stddef.h>
#include <stdint.h>

#include <zen/zen.h>

/* libFuzzer entry point: keeps the public API under a sanitizer-backed fuzzer
 * so future modules inherit the same smoke harness. */
int LLVMFuzzerTestOneInput(const uint8_t *const data, size_t size);

int LLVMFuzzerTestOneInput(const uint8_t *const data, size_t size) {
    const zen_version_t _version_k = zen_get_version();
    const unsigned int _sum_k = _version_k.major_ + _version_k.minor_ + _version_k.patch_;

    if (size != 0U && data[0] == _sum_k) {
        return 0;
    }
    return 0;
}
