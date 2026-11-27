# Zlang roadmap

Current version: **0.39**

This file is a compact record of delivered language levels, locked decisions and remaining work. The
published specification at <https://z.iantorres.cl/docs/specification> is normative. Detailed design
history for old releases was folded into this summary and the site pages.

## Guiding rule

A Zlang program is a set of compile-time decisions that produces ordinary C99. Nothing is decided
at runtime that could have been decided at compile time.

## Locked decisions

- Source-to-source compilation to C99; no VM, bytecode, runtime type metadata or garbage collector.
- Header-only `zlang_*` runtime; generated C never allocates on the heap.
- Fixed-shape arrays with compile-time storage and bounds-checked access.
- Native C control flow; no callbacks or hidden runtime dispatch except internal vtables for
  single inheritance.
- Explicit `clone`, `reference` and `const reference` parameter modes.
- Canonical `@std/<domain>` imports and domain-based activation macros.
- Stable diagnostic codes with one-based source locations.
- Compile-time generics with monomorphization; no runtime generic representation.
- Native `result_type<T, status_code>` layouts shared by the compiler registry and public headers.
- Result capabilities are compile-time flags; disabled capabilities have no runtime work.
- Borrowed native streams with one operations table for stdio, memory and future backends.
- Every registered stdlib function returns `result_type<T, status_code>`.

## Delivered scope

| Version | Delivered feature |
| --- | --- |
| 0.1–0.5 | C99 baseline, arrays, loops, functions, bounds checks, readable emitted C |
| 0.6–0.13 | Structs, enums, user types, comments, symbol roles, reserved namespace, parameter modes |
| 0.14–0.21 | Fixed arrays, inference, typed helpers, nested initializers, TTO, zero-argument and forward calls |
| 0.22–0.24 | Wide numeric primitives, conversions, deterministic random, stdlib imports, `write` replaces `print` |
| 0.25–0.29 | Bounded input, content strings, exact operand types, stdlib math/strings, inheritance and traits |
| 0.30 | Compile-time generics on structs and functions |
| 0.31 | Native result layouts and structured stdlib registry metadata |
| 0.32 | IO byte-transfer results and explicit stream handles |
| 0.33 | Generic struct methods and native result helper methods |
| 0.34 | Scalar rename to `int32`/`uint32` and `to_int32`/`to_uint32`; no aliases |
| 0.35 | Stable native stream contract with stdio and memory backends |
| 0.36 | Built-in result rename to `result_type<T, status_code>` and `status_code` |
| 0.37 | Uniform `result_type<T, status_code>` contract across all registered stdlib domains |
| 0.38 | Compile-time result capabilities: `time()` and `dump()` |
| 0.39 | Eight syntax forms and named interface/trait/struct contracts |

## Current shape

The standard library covers `io`, `math`, `program_options`, `random` and `strings`. Each registered
function records its result type, parameter modes and activation macro. Native result layouts live
in `<zlang/standard/result/result.h>`; status codes live in
`<zlang/standard/status/status.h>`; streams live in `<zlang/standard/io/stream.h>`.

Registry entries, public headers, emitted C, conformance tests, golden examples and site pages must
agree. Native parity is checked by matching compiler registry entries, header declarations and
documentation.

Traps remain traps. Null storage, invalid offsets, zero capacity and capacity overflow are
programmer or resource errors; a trapped function does not return a result. Recoverable outcomes
are reported through `status_code`.

## Browser toolchain

## Implemented 0.39 contract

Zlang 0.39 accepts labeled jumps, `switch`, value-producing `match`, ternary `?:`, bitwise and
shift operators, `do while`, exclusive `uint32` range literals and labeled loop jumps. Interfaces
are named field-and-method contracts. Traits require interfaces and provide method bodies. Structs
implement interfaces and use traits.

The playground builds Zlang entirely in the browser:

```text
.z source → zc.wasm → C → clang.wasm → object → lld.wasm → .wasm → WASI host
```

Build scripts in `tools/wasm/` produce artifacts from pinned upstream LLVM, Emscripten and wasi-sdk
sources. Artifacts are static files under `site/public/compiler/`, with notices in
`site/THIRD_PARTY_NOTICES.md`. No source or binary is sent to a server.

## 0.39 — syntax and interface contracts

Zlang 0.39 adds eight syntax features and replaces field-only trait requirements with named
interfaces.

| # | Feature | Syntax |
| --- | --- | --- |
| 1 | jump and labels | `label:` and `jump label;` — delivered |
| 2 | switch | `switch (value) { case literal: ... default: ... }` — delivered |
| 3 | match | `match (value) { literal => expression, _ => expression }` — delivered |
| 4 | ternary | `condition ? then_expression : else_expression` — delivered |
| 5 | bitwise and shifts | `&`, `|`, `^`, `~`, `<<`, `>>` — delivered |
| 6 | do-while | `do { ... } while (condition);` — delivered |
| 7 | range literal | `start..end` for exclusive `uint32` ranges — delivered |
| 8 | labeled loop jumps | `outer: while (...) { break outer; continue outer; }` — delivered |

Interfaces replace ad-hoc trait field requirements:

```zlang
interface MyInterface {
    int32 value;
    function foo(): int32;
}

trait MyTrait requires MyInterface {
    function foo(): int32 {
        return self.value;
    }
}

struct MyStruct implements MyInterface {
    use MyTrait;
}
```

An interface is a named contract with fields and method signatures. A trait requires one or more
interfaces and supplies method bodies. A struct implements interfaces and uses traits. The compiler
checks that every implemented interface field and method exists on the struct, directly or through
a used trait, and that every trait requirement is satisfied by an implemented interface.

## Remaining work

- Extend `dump()` to user-defined `T` only when `T` implements `dump(): string`.
- Add new IO domains only through the existing native `stream` contract.
- Keep generic traits, generic inheritance and variadic type packs out of scope until explicitly
  locked.
- Keep the site specification synchronized with compiler, runtime and stdlib changes. `SPEC.md` and
  this file are working notes, not alternate sources of truth.

## Non-goals

- Runtime reflection or runtime names by string.
- Heap-backed strings, vectors, objects or growable storage.
- C++, Objective-C, threads, atomics, complex arithmetic and variable-length arrays.
- Callback-based iteration and runtime callbacks passed into helpers.
- Compatibility aliases for breaking renames.

Older design notes, duplicated completion sections and the historical pending checklist were
removed after their decisions landed or were superseded by the site specification.
