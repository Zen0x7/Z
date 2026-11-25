---
title: Compiler
description: Pipeline, storage plan, generated C and diagnostics.
---

# Compiler

`zc` writes C from Zlang source. By default it also calls a C compiler. With `--emit-c`, it stops after writing the `.c` file.

## Pipeline

```console
source.z
  -> lexer          tokens and copied text
  -> parser         flat AST
  -> semantics      names, types and diagnostics
  -> planner        storage for each declaration
  -> emitter        C99 source
  -> C compiler     native executable
```

Each stage writes into memory owned by its caller. The compiler core keeps no global state and calls no allocator. That makes each pass easy to test on its own.

## Where values live

Before writing C, the planner measures each declaration.

- Small values become local C variables.
- Larger values become file-scope `static` objects with internal linkage.
- Neither kind uses the heap.

`ZLANG_DEFAULT_STACK_LIMIT` sets the local-storage budget. An array without a capacity never reaches the planner. Semantic analysis stops it with `ZLANG3001`.

## Reading the C

The generated file uses ordinary C constructs.

- `integer` becomes `int32_t`, `unsigned` becomes `uint32_t`, `boolean` becomes `bool`, and `string` becomes `zlang_string_t`.
- Arithmetic calls checked helpers. Overflow traps.
- Logical operators keep C short-circuit behavior.
- Array access checks bounds. Append checks capacity.
- `foreach` becomes a native `for` loop.
- Errors use numbers. Generated code does not build error strings at runtime.

```c
#include <zlang/runtime/runtime.h>

int main(void) {
    zlang_print(zlang_gen_literal_0);

    return ZLANG_ZERO;
}
```

Names carry their origin. `app_*` names come from your source. `zlang_gen_*` names are compiler support. `zlang_*` names belong to the runtime. `ZLANG_*` names are constants.

## Diagnostics

A diagnostic gives a file, a line, a column and a stable code.

```console
input.z:12:5: error[ZLANG3001]: sequence 'names' has no declared capacity
```

The code stays the same across releases, so scripts can match on it.

## Verification

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build -j
ctest --test-dir build --output-on-failure -j
```

CI runs C99, C11, C17 and C23, plus sanitizers, static analysis, coverage and several architectures.
