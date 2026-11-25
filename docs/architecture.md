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
feature of Zlang; explicit capacities and compile-time initializers remain part of the
program's fixed memory plan.

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


## Planned 0.20 architecture additions

The standard library will be domain-based rather than a set of keywords. Source code imports a
function with `import { random_int32 } from "algorithms";`. Function names identify their result
type so future variants such as `random_bytes`, `random_char` and `random_float` fit the same model.
The compiler records the import in semantic analysis and the emitter activates the matching runtime
domain macro before including `<zlang/runtime/runtime.h>`. Domain implementations live under
`include/zlang/standard/<domain>/<function>.h`, and the base runtime includes them only when their
activation macro is defined.

Standard headers use two include phases. The declaration phase emits C prototypes for every
activated function; the implementation phase emits the `static inline` bodies. This makes calls
between standard functions independent of include order. Activation is transitive and comes from
standard library metadata: importing a function activates its dependencies as well. Circular calls
are legal because every prototype is visible before any implementation body is compiled.

Each function is registered once through `zlang_std_function(...)` in its domain registry. That
registration carries the Zlang-facing signature, the C symbol, lifecycle status (`DRAFT`,
`UNSTABLE`, `STABLE`), safety status (`SAFE`, `UNSAFE`) and dependencies. Generation produces the
activation macros, declaration headers, semantic signature table and documentation entries. The
standard header contains only the implementation body. This keeps authoring local and makes
validation mechanical: duplicate names, duplicate C symbols, missing dependencies, missing headers
and signature drift are configure-time errors.

Stateful standard functions never own mutable globals and never hide mutable `static` state in a
header. If a stateful deterministic PRNG is selected, runtime state is explicit. The emitter adds a
hidden state parameter only to generated C functions that transitively call stateful standard
functions. Zlang source signatures do not expose this parameter; prototypes, definitions and call
sites are updated together so the generated C remains consistent. A future global-state feature
must expose state through an explicit runtime context type.

`--tto` will enable a fixed-point optimizer. It uses a caller-owned context, a worklist and
generation counters so a rewrite that enables or invalidates another fact can requeue affected AST
nodes. It stops only at a fixed point or at a capacity limit. Opaque operations such as
`random_int32` remain runtime calls; their arguments may be folded but their result is not folded,
cached or reordered.
