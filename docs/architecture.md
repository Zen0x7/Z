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

The standard library will be domain-based rather than a set of keywords. The catalog, naming rules
and classification dimensions live in [`standard_library.md`](standard_library.md). Source code
imports a function with `import { generate_int32 } from "random";`. The compiler records the import
in semantic analysis and the emitter activates the matching function macro before including
`<zlang/runtime/runtime.h>`. Domain implementations live under
`include/zlang/standard/<domain>/<function>.h`, and the base runtime includes them only when their
activation macro is defined.

Standard headers use two include phases. The declaration phase emits C prototypes for every
activated function; the implementation phase emits the `static inline` bodies. This makes calls
between standard functions independent of include order. Activation is transitive and comes from
standard library metadata: importing `algorithms.shuffle_int32` activates
`random.generate_int32` as well. Circular calls are legal because every prototype is visible before
any implementation body is compiled.

Each function is registered once through `zlang_std_function(...)` in its domain registry. That
registration carries the Zlang-facing signature, the C symbol, lifecycle status, safety status,
evaluation, determinism, resource model, error model, portability, optimizer contract and
dependencies. Generation produces the activation macros, declaration headers, semantic signature
table and documentation entries. The standard header contains only the implementation body. This
keeps authoring local and makes validation mechanical.

Generated C uses provenance prefixes so readers can tell what belongs to the source program and
what belongs to the language. `app_*` names translate user declarations; `zlang_gen_*` names are
compiler support derived from source; `zlang_std_*` names are standard functions; `zlang_*` names
are the public runtime API; `ZLANG_*` names are language constants.

Each standard function has one test directory under `tests/stdlib/<domain>/<function>/`. The
directory is a micro CMake project with direct C cases and Zlang integration programs. Direct
cases test runtime boundaries and state. Integration programs test imports, dependency activation,
golden C and stdout under the strict warning set.

Stateful standard functions never own mutable globals and never hide mutable `static` state in a
header. If a stateful deterministic PRNG is selected, runtime state is explicit. The emitter adds a
hidden state parameter only to generated C functions that transitively call stateful standard
functions. Zlang source signatures do not expose this parameter; prototypes, definitions and call
sites are updated together so the generated C remains consistent. A future global-state feature
must expose state through an explicit runtime context type.

`--tto` will enable a fixed-point optimizer. It uses a caller-owned context, a worklist and
generation counters so a rewrite that enables or invalidates another fact can requeue affected AST
nodes. It stops only at a fixed point or at a capacity limit. Opaque operations such as
`random.generate_int32` remain runtime calls; their arguments may be folded but their result is not
folded, cached or reordered.
