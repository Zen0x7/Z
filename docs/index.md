# Z / libzen API reference

Z is a header-only C99 library. The CMake target is `zen::zen`; include the umbrella header to
reach every public module:

```c
#include <zen/zen.h>
```

## Public modules

- `zen/version/version.h` — compile-time version macros and `zen_get_version()`.
- `zen/testing/testing.h` — the header-only test runner used by the project's own test modules.

Each module keeps its header at `include/zen/<module>/<module>.h`. Private helpers live under
`include/zen/detail/` and are not part of the public API.

## Building

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

The canonical language level is C99. CI also compiles C11, C17, and C23, with sanitizers and on
multiple architectures.
