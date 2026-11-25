---
title: Build
description: Build zc and run its tests.
navigation:
  title: Build
---

# Build

Use CMake and Ninja. Clang is the default development compiler.

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build -j
ctest --test-dir build --output-on-failure -j
```

The compiler driver is `build/tools/zc/zc`.

## Sanitizer build

Use one sanitizer at a time:

```bash
cmake -S . -B build-asan -G Ninja -DZLANG_SANITIZER=address
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

Supported modes include `address`, `undefined`, `thread`, `leak`, `integer`, `cfi` and `hwaddress`.

## Check public headers

```bash
./scripts/check_headers.sh clang gcc
```

This compiles each public header on its own.

## See also

- [First program](/docs/quick-start/first-program)
- [Command line](/docs/quick-start/command-line)
