# Zlang Roadmap

## Purpose

Zlang is a source-to-source language for bounded, deterministic applications. A Zlang program is compiled to valid, readable C that calls a small, audited C API (`zlang_*`). The generated C is an intermediate representation: it is not a runtime, and it is not code the application author is expected to maintain.

The promise is simple: **no magic**. `--emit-c` shows exactly which C will be compiled. Without that flag, `zlang-compile` goes directly from `.zlang` source to a native binary.

## Locked decisions

- **Transpilation, not interpretation.** There is no VM, no opcode dispatch, no runtime class or type registration, and no garbage collector.
- **Generated C is transport.** Every Zlang operation lowers to a call into the Zlang C API. Control flow (`if`, loops) is emitted as native C from compiler IR nodes. There is no callback-based `zlang_for_each` and no indirect dispatch.
- **Compile-time memory analysis.** Every size is known before the binary exists. Dynamic allocation is illegal. Unbounded values require an explicit limit; otherwise the compiler rejects the program and reports the source location of the declaration that needs it.
- **Realtime means memory discipline.** The goal is a bounded footprint and zero dynamic allocation, not hard realtime syscall or WCET guarantees.
- **Limit violations trap.** When a declared capacity is exceeded, the binary traps with a fixed code. It never grows a buffer and never builds an error message at runtime.
- **Diagnostics are first-class.** Syntax, type, and resource/allocation errors are separate categories and are reported in Zlang terms, not as C compiler output.
- **The C API is thin and auditable.** It is small, readable, and verified by the existing CI: sanitizers, coverage, Valgrind, and the multi-platform matrix.
- **Distribution is twofold.** A local `zlang-compile` binary compiles `.zlang` files, and a provided GitHub Action does the same in CI and publishes the resulting artifact.

## Non-goals

- A general-purpose interpreted language.
- Hidden heap growth, reference counting, or garbage collection.
- Syntax as the product. Syntax is deferred until the semantic model is settled.
- Runtime reflection or dynamic dispatch as a default mechanism.
- Hard realtime timing guarantees beyond predictable memory behavior.

## Architecture

```text
main.zlang
    -> zlang-compile
        -> front end: lexer, parser, AST, source locations
        -> semantic analysis: scopes, types, explicit limits
        -> memory planner: sizes, offsets, stack vs static regions
        -> C emitter: zlang_* calls plus native control flow
    -> main.c                 (shown by --emit-c)
    -> gcc/clang + libzlang
    -> main / main.exe
```

The C emitter has one rule: an operation is lowered to a call into the Zlang API, while control flow stays as ordinary C. A loop exists as a `for_each` node in the IR so the compiler can validate its range, but the emitted C is a plain `for` with a checked bound. This keeps the contract enforceable without giving up optimizer visibility.

## Phases

1. **Foundation** *(in place)*
   CMake project, strict warning policy, CI matrix for amd64 and arm64, sanitizer modes, coverage with Codecov, static analysis, install/export checks, fuzz smoke target, and a module-based test layout.

2. **Language front end** *(implemented)*
   Lexer, parser, AST, and source locations. Diagnostics for syntax errors expressed in Zlang terms. Outcome: the first minimal `.zlang` programs parse cleanly.

3. **Semantic model** *(implemented)*
   Scopes, symbol table, type checking, explicit capacity declarations, and range analysis. Outcome: unbounded allocation is rejected at the declaration site with a `file:line:column` diagnostic.

4. **Memory planner** *(implemented)*
   Compute sizes, offsets, and lifetimes. Decide stack placement for small bounded values and static regions for larger ones. Outcome: an allocation plan for every value, with no runtime decision left.

5. **C emitter and `--emit-c`** *(implemented)*
   Lower the checked IR to valid C: `zlang_*` calls for operations, native `for`/`if` for control flow. Outcome: readable emitted C that can be inspected before compilation.

6. **Zlang C API** *(implemented)*
   Implement bounded strings, vectors, arenas, print, and trap helpers as thin C functions. Outcome: an API that is fully covered by the existing sanitizer, Valgrind, and coverage jobs.

7. **Tooling** *(CLI implemented; GitHub Action deferred to 1.0)*
   `zlang-compile` is implemented with `--emit-c` and direct native compilation. Packaging, release artifacts, and the Zlang GitHub Action are intentionally deferred to version 1.0.

8. **Hardening** *(implemented for 0.1)*
   Lexer, parser, and emitter fuzz harnesses run in CI. Trap paths are covered in child processes. Diagnostics and supported limits are documented in `SPEC.md`, including the Annex D conformance matrix.

## Open design questions

Resolved in 0.1: capacity syntax, the initial `zlang_*` API, unbounded-allocation rejection, and the fixed trap codes are all defined in `SPEC.md`.

Deferred to 1.0: modules, classes, string concatenation, a third mapped-memory tier, incremental compilation, packaging/release, and the Zlang GitHub Action.

## Definition of done for v0.1

A bounded `.zlang` program compiles to native C-backed code; `--emit-c` shows the generated C; the binary performs zero dynamic allocation; an unbounded program is rejected with a source location; and the whole path runs through the existing CI matrix.
