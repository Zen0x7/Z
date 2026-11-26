# Zlang working specification notes

Current version: **0.37**

The normative specification is published at <https://z.iantorres.cl/docs/specification>. This file is
a compact working note for agents: it records the current contract and the decisions that produced
it. When it disagrees with the site, the site wins until this file is updated.

## Contract summary

Zlang is a source-to-source language. `zc` emits C99 that calls the header-only `zlang_*` runtime;
a native C toolchain produces the executable. There is no VM, bytecode, runtime type metadata,
garbage collector or hidden heap allocator. Names, types, capacities and storage locations are
resolved by the compiler.

Generated C:

- includes `<stdint.h>`, `<stdbool.h>` and `<zlang/runtime/runtime.h>`;
- uses native C `if`, `while`, `for`, `break` and `continue`;
- never calls `malloc`, `calloc`, `realloc`, `free`, `strdup` or an equivalent service;
- never passes a callback to a runtime helper;
- preserves source order for observable `write` output;
- makes `--emit-c` output byte-for-byte identical to the C compiled by default.

## Types and values

| Zlang | Meaning | Emitted C |
| --- | --- | --- |
| `int32` | signed 32-bit | `int32_t` |
| `uint32` | unsigned 32-bit | `uint32_t` |
| `int64` | signed 64-bit | `int64_t` |
| `uint64` | unsigned 64-bit | `uint64_t` |
| `float` | IEEE-754 binary32 | `float` |
| `double` | IEEE-754 binary64 | `double` |
| `boolean` | `true` or `false` | `bool` |
| `string` | non-owning UTF-8 byte view | `zlang_string_t` |
| `arguments` | host argument view | `zlang_arguments_t` |
| `stream` | borrowed native stream handle | `zlang_stream_t` |
| `result_type<T, status_code>` | native result layout | `zlang_std_result_*_status_t` |

There is no implicit numeric conversion. Explicit conversions are `to_int32`, `to_uint32`,
`to_int64`, `to_uint64`, `to_float` and `to_double`. Arithmetic is checked for signed overflow;
signed minimum negation and zero-capacity failures trap. Floating arithmetic follows IEEE-754.

Strings are borrowed views. Variable concatenation requires `string[N]` storage and traps on
capacity overflow. Arrays have fixed capacity, bounds-checked index access, no logical length and
no append. Storage at or below `ZLANG_DEFAULT_STACK_LIMIT` is local; larger storage is `static`.

Structs, enums, traits and `uses` are compile-time constructs. Single inheritance emits internal
vtables and type tags; trait calls are direct calls. No dynamic allocation is introduced.

## Generics

A declaration may list up to four type parameters; an application may pass up to four concrete type
arguments. Each application is monomorphized into a concrete C99 type or function. Generic structs
support flat instance methods; generic functions must be top-level and flat. Generic traits,
generic inheritance and variadic type packs remain out of scope.

## Native results and IO

Every registered standard library function returns `result_type<T, status_code>`. `T` is the
function's ideal value, `code` carries a `status_code`, and `what` is a bounded read-only message.
Functions with no recoverable failure return `SUCCESS`. Programmer and resource errors trap.

Streams are borrowed, trivially copyable handles with a constant operations table. Stdio and
caller-owned memory streams implement the same contract as future sockets, pipes and devices.
Canonical IO forms are `read(source, destination, offset, count)` and
`write(destination, source, offset, count)`; convenience overloads keep implicit stdin/stdout.

Status constants are `STATUS_SUCCESS`, `STATUS_END_OF_INPUT`, `STATUS_IO_ERROR`,
`STATUS_INVALID_ARGUMENT` and `STATUS_CAPACITY`. C symbols and status enum constants remain
stable across the Zlang-facing lowercase type rename.

## Traps

`zlang_trap(code)` terminates without formatting, allocating or printing. Codes are:

| Code | Name |
| --- | --- |
| `1` | `ZLANG_TRAP_OVERFLOW` |
| `2` | `ZLANG_TRAP_BOUNDS` |
| `3` | `ZLANG_TRAP_CAPACITY` |
| `4` | `ZLANG_TRAP_INVALID_ARGUMENT` |
| `5` | `ZLANG_TRAP_DIVISION_BY_ZERO` |

## Diagnostics

Diagnostics use `file:line:column: error[ZLANGxxxx]: message`. Classes are `ZLANG1xxx` syntax,
`ZLANG2xxx` type, `ZLANG3xxx` resource. Stable codes cover unexpected tokens, unterminated strings,
invalid characters, unsupported constructs, literal range, duplicate or shadowed declarations,
undefined identifiers, type mismatch, missing or excessive string capacity, initializer length and
unnecessary references. Resource errors point at the declaration that needs a limit. The compiler
may emit several diagnostics but emits no C when any error is present.

## Conformance

The maintained matrix lives in `tests/spec/test_cases.h`. It covers grammar productions, emission,
bounds, arithmetic, conversions, strings, arrays, functions, structs, traits, inheritance, generics,
native results, streams, diagnostics, golden examples and installed-package behavior.

## Decision history

- **0.30** — compile-time generics for structs and functions.
- **0.31** — native result layouts and structured registry metadata.
- **0.32** — IO returns byte-transfer results; explicit stream handles introduced.
- **0.33** — generic methods and result helper methods.
- **0.34** — scalar names and conversions become `int32`/`uint32`/`to_int32`/`to_uint32`.
- **0.35** — stable native stream contract with stdio and memory backends.
- **0.36** — built-in result names become `result_type<T, status_code>` and `status_code`.
- **0.37** — every registered stdlib function returns `result_type<T, status_code>`.

Older numbered status sections and duplicated design notes were folded into the site specification
and removed from this working file.
