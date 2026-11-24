# Compiler architecture

## Pipeline

```text
source.zlang
  -> lexer          tokens + scratch-copied identifiers and strings
  -> parser         flat AST with expression and statement indices
  -> semantics      scopes, bindings, type checks and diagnostics
  -> memory planner automatic vs static placement using a byte budget
  -> emitter        portable C99 source
  -> host compiler  native executable
```

`zlang-compile --emit-c` stops after the emitter. The generated C includes
`<zlang/runtime/runtime.h>` and contains no runtime class registry, no VM opcodes and no hidden
allocation.

## Ownership

Every stage uses caller-owned buffers. There are no global variables and no calls to `malloc` in
the compiler core. This keeps the CLI usable in constrained build environments and lets sanitizers
see every byte that crosses a stage boundary.

- `zlang_lexer_t` reads source text and writes tokens plus copied token text into a scratch buffer.
- `zlang_parser_t` writes statements and expressions into flat arrays.
- `zlang_semantic_context_t` writes diagnostics and uses a flat symbol table.
- `zlang_memory_plan_t` records storage decisions for declarations.
- `zlang_emitter_t` appends C text into a caller-provided output buffer.

## Memory model

The planner computes the byte size of each declaration. Values up to `ZLANG_DEFAULT_STACK_LIMIT`
are emitted as C automatic variables. Larger values are emitted at file scope with C `static`
storage duration and internal linkage. Both live in fixed memory; neither uses the heap.

Unbounded sequences are rejected during semantic analysis with `ZLANG3001`. Runtime growth is not a
feature of Zlang through version 0.2; explicit capacities and compile-time initializers remain part
of the program's fixed memory plan.

## Generated C contract

- Primitive types map directly: `integer` to `int32_t`, `unsigned` to `uint32_t`, `boolean` to
  `bool`, and `string` to `zlang_string_t`.
- Arithmetic uses checked helpers; overflow traps with a fixed code.
- Logical operators emit native C operators with short-circuit behavior.
- Sequence access uses explicit bounds checks; append checks capacity.
- `foreach` becomes a native C `for` loop, not a callback.
- Errors use fixed numeric codes; generated code never formats an error string at runtime.

## Diagnostics

Diagnostic codes are stable across releases. Every diagnostic carries a source line and column. The
CLI prints `file:line:column: error[ZLANGxxxx]: message` and exits with a stage-specific code.

## Verification

The test suite covers the public headers, runtime helpers, lexer, parser, semantic rules, planner,
emitter, CLI and exported CMake package. CI adds sanitizers, Valgrind, static analysis, coverage and
multi-architecture builds.
