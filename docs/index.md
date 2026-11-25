# Z / libzlang reference

Z contains two related pieces:

- `libzlang`, a header-only C99 runtime library used by generated programs.
- `zlang-compile`, a source-to-source compiler that translates Zlang into portable C.

The CMake targets are `zlang::zlang` for the runtime, `zlang::compiler_core` for the compiler
library and `zlang-compile` for the command line driver.

## Start here

- [Compiler architecture](architecture.md) describes the pipeline and ownership rules.
- [Zlang grammar](grammar.md) lists the current syntax and diagnostics.
- [Standard library design](standard_library.md) describes the planned domain catalog and import model.
- The sections below list the runtime headers, compiler headers and `zlang-compile` behavior.

## Public API

Include the umbrella header to reach every public module:

```c
#include <zlang/zlang.h>
```

The generated API reference is published at <https://z.iantorres.cl>.

| Header | Purpose |
| --- | --- |
| `zlang/version/version.h` | Compile-time version macros and `zlang_get_version()` |
| `zlang/testing/testing.h` | Header-only test runner used by the project's own tests |
| `zlang/runtime/runtime.h` | Bounded strings, checked arithmetic, append, concatenation and trap helpers |

Each public module keeps its header at `include/zlang/<module>/<module>.h`. Private helpers live
under `include/zlang/detail/`.

## Compiler API

The compiler library is internal to the repository but documented here because the emitted C is
part of the product contract.

| Header | Purpose |
| --- | --- |
| `zlang/compiler/lexer.h` | Tokenizer with caller-owned scratch storage |
| `zlang/compiler/parser.h` | Recursive-descent parser for Zlang |
| `zlang/compiler/ast.h` | Flat AST representation using indices |
| `zlang/compiler/diagnostics.h` | Stable diagnostic codes and source locations |
| `zlang/compiler/semantics.h` | Type and binding checks |
| `zlang/compiler/planner.h` | Compile-time storage placement |
| `zlang/compiler/emitter.h` | C99 emitter |

## CLI

```sh
zlang-compile input.zlang --emit-c                       # write input.c
zlang-compile input.zlang                                # write input.c and build a native executable
zlang-compile input.zlang --emit-c --disable-comments    # omit source comments from input.c
```

Diagnostics use the canonical form:

```text
input.zlang:12:5: error[ZLANG3001]: sequence 'names' has no declared capacity
```

## Building

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
cmake --build build --target zlang_coverage
```

The canonical language level is C99. CI also compiles C11, C17 and C23, with sanitizers and on
multiple architectures.
