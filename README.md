<p align="center"><a href="https://github.com/Zen0x7/Z" target="_blank"><img src="docs/logo.svg" width="400" alt="Zlang"></a></p>

<p align="center">
<a href="https://codecov.io/gh/Zen0x7/Z"><img src="https://codecov.io/gh/Zen0x7/Z/branch/master/graph/badge.svg?token=8TUYTBYEBJ" alt="Coverage"></a>
<a href="https://github.com/Zen0x7/Z/actions/workflows/ci.yml"><img src="https://github.com/Zen0x7/Z/actions/workflows/ci.yml/badge.svg?branch=master" alt="CI"></a>
<a href="https://github.com/Zen0x7/Z/actions/workflows/codeql.yml"><img src="https://github.com/Zen0x7/Z/actions/workflows/codeql.yml/badge.svg?branch=master" alt="CodeQL"></a>
<a href="https://github.com/Zen0x7/Z/actions/workflows/dependabot/dependabot-updates"><img src="https://github.com/Zen0x7/Z/actions/workflows/dependabot/dependabot-updates/badge.svg?branch=master" alt="Dependabot Updates"></a>
</p>

## About Z

> **Note:** This repository contains the Z compiler, its header-only C runtime and the test
> suite. For contributor rules and repository conventions, see [AGENTS.md](AGENTS.md).

Z is a small source-to-source language for bounded, deterministic programs. You write `.zlang`
source; `zlang-compile` emits readable C99; then your normal C toolchain produces the executable.

There is no VM, no bytecode, no garbage collector and no hidden runtime allocator. Types,
capacities and storage decisions are resolved by the compiler. Generated C calls a small audited
`zlang_*` API for checked arithmetic, bounds, capacity and string operations.

## What it is for

Z aims to make the "compile-time decisions, ordinary C output" idea pleasant to use:

- explicit value types: `integer`, `unsigned`, `boolean`, `string`;
- fixed-capacity sequences with compile-time memory planning;
- native C control flow, including `else if` chains;
- bounded string concatenation instead of heap-backed strings;
- direct C calls to user-defined functions;
- fixed numeric traps when a declared limit is violated.

Version 0.2 adds comments, lexical scopes with unique generated identifiers, sequence
initializers, checked index assignment, bounded strings, logical operators and expression
grouping.

## Pipeline

```text
example.zlang
    |
    |  zlang-compile
    v
example.c  ->  C compiler  ->  executable
```

```sh
./build/tools/zlang-compile/zlang-compile example.zlang --emit-c
./build/tools/zlang-compile/zlang-compile example.zlang
```

The first command only writes `example.c`. The second also invokes `$ZLANG_CC`, or `cc` by
default, to build the native binary.

## Quick start

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build -j
ctest --test-dir build --output-on-failure -j
```

## Documentation

- [Live API reference](https://z.iantorres.cl)
- [Language specification](SPEC.md)
- [Roadmap and locked decisions](ROADMAP.md)
- [API reference source](docs/index.md)
- [0.2 grammar additions](docs/grammar_02.md)

The generated Doxygen site is written to `build/docs/html/index.html`.
