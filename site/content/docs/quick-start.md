---
title: Quick start
description: Build zc, write a program and run it.
---

# Quick start

Build the compiler:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build -j
ctest --test-dir build --output-on-failure -j
```

Write a file named `hello.z`:

```zlang
print("Hello from Zlang");
print("\n");
```

Run it:

```bash
./build/tools/zc/zc hello.z
./hello
```

```console
Hello from Zlang
```

The first command writes `hello.c` and calls your C compiler. The second command runs the program.

## Keep only the C

```bash
./build/tools/zc/zc hello.z --emit-c
```

This writes `hello.c` and stops. Add `--disable-comments` if you do not want source comments in the generated file.

## Command forms

```console
zc input.z --emit-c                       # write input.c
zc input.z                                # write input.c and build a binary
zc input.z --emit-c --disable-comments    # omit comments from input.c
```

Errors include the file, line and column:

```console
input.z:12:5: error[ZLANG3001]: sequence 'names' has no declared capacity
```

The repository keeps worked programs in `examples/algorithms/` and `examples/features/`. Each folder has `main.z`, the generated `main.c`, the expected output and a README.
