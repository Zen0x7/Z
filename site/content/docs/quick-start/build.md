---
title: Build
description: Build options, tests, sanitizers and header checks.
navigation:
  title: Build
---

# Build

Use this page when you need more than the first build. The standard debug build is:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build -j
```

Use `-DCMAKE_C_COMPILER=gcc` if you are testing GCC.

The compiler driver is `build/tools/zc/zc`. The library is header-only; there is no compiled library to install.

## Tests

```bash
ctest --test-dir build --output-on-failure -j13
ctest --test-dir build --output-on-failure -L module:examples
ctest --test-dir build --output-on-failure -R zlang.examples.algorithms.fibonacci
```

Each registered CTest case runs one test function or one golden example. Labels select modules and categories.

## Sanitizers

Use one sanitizer at a time:

```bash
cmake -S . -B build-asan -G Ninja -DZLANG_SANITIZER=address
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

Supported modes are `address`, `undefined`, `thread`, `leak`, `int32`, `cfi` and `hwaddress`. MSAN is intentionally excluded because it requires an instrumented standard library. Do not combine sanitizer modes in one build.

## Coverage

Coverage uses GCC and gcovr:

```bash
cmake -S . -B build-coverage -G Ninja -DCMAKE_C_COMPILER=gcc -DZLANG_COVERAGE=ON
cmake --build build-coverage --target zlang_coverage
```

Do not combine coverage with `ZLANG_SANITIZER`.

## Fuzzers

With Clang:

```bash
cmake -S . -B build-fuzz -G Ninja -DCMAKE_C_COMPILER=clang -DZLANG_BUILD_TESTS=OFF -DZLANG_BUILD_FUZZERS=ON
cmake --build build-fuzz
for target in build-fuzz/fuzz/zlang_*_fuzz; do ./"$target" -max_total_time=30; done
```

## Public headers

```bash
./scripts/check_headers.sh clang gcc
```

This compiles every public header on its own.

## See also

- [Quick start](/docs/quick-start)
- [Examples](/docs/quick-start/examples)
