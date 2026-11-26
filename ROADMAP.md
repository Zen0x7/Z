# Zlang Roadmap

This file is append-only. Each version gets one section, appended at the bottom. Earlier sections are a historical record: do not rewrite them when plans change. If a decision is superseded, append a new entry that says so.

Status labels: `Implemented`, `In progress`, `Planned`, `Deferred`.

## 0.1 — Foundation and language baseline

Status: Implemented.

### Purpose

Zlang is a source-to-source language for bounded, deterministic applications. A Zlang program is compiled to valid, readable C that calls a small, audited C API (`zlang_*`). The generated C is an intermediate representation: it is not a runtime, and it is not code the application author is expected to maintain.

The promise is simple: **no magic**. `--emit-c` shows exactly which C will be compiled. Without that flag, `zc` goes directly from `.z` source to a native binary.

### Locked decisions

- **Transpilation, not interpretation.** There is no VM, no opcode dispatch, no runtime class or type registration, and no garbage collector.
- **Generated C is transport.** Every Zlang operation lowers to a call into the Zlang C API. Control flow (`if`, loops) is emitted as native C from compiler IR nodes. There is no callback-based `zlang_for_each` and no indirect dispatch.
- **Compile-time memory analysis.** Every size is known before the binary exists. Dynamic allocation is illegal. Unbounded values require an explicit limit; otherwise the compiler rejects the program and reports the source location of the declaration that needs it.
- **Realtime means memory discipline.** The goal is a bounded footprint and zero dynamic allocation, not hard realtime syscall or WCET guarantees.
- **Limit violations trap.** When a declared capacity is exceeded, the binary traps with a fixed code. It never grows a buffer and never builds an error message at runtime.
- **Diagnostics are first-class.** Syntax, type, and resource/allocation errors are separate categories and are reported in Zlang terms, not as C compiler output.
- **The C API is thin and auditable.** It is small, readable, and verified by the existing CI: sanitizers, coverage, Valgrind, and the multi-platform matrix.
- **Distribution is twofold.** A local `zc` binary compiles `.z` files, and a provided GitHub Action does the same in CI and publishes the resulting artifact.

### Non-goals

- A general-purpose interpreted language.
- Hidden heap growth, reference counting, or garbage collection.
- Syntax as the product. Syntax is deferred until the semantic model is settled.
- Runtime reflection or dynamic dispatch as a default mechanism.
- Hard realtime timing guarantees beyond predictable memory behavior.

### Architecture

```text
main.z
    -> zc
        -> front end: lexer, parser, AST, source locations
        -> semantic analysis: scopes, types, explicit limits
        -> memory planner: sizes, offsets, stack vs static regions
        -> C emitter: zlang_* calls plus native control flow
    -> main.c                 (shown by --emit-c)
    -> gcc/clang + libzlang
    -> main / main.exe
```

The C emitter has one rule: an operation is lowered to a call into the Zlang API, while control flow stays as ordinary C. A loop exists as a `for_each` node in the IR so the compiler can validate its range, but the emitted C is a plain `for` with a checked bound. This keeps the contract enforceable without giving up optimizer visibility.

### Delivered scope

1. **Foundation**
   CMake project, strict warning policy, CI matrix for amd64 and arm64, sanitizer modes, coverage with Codecov, static analysis, install/export checks, fuzz smoke target, and a module-based test layout.

2. **Language front end**
   Lexer, parser, AST, and source locations. Diagnostics for syntax errors expressed in Zlang terms.

3. **Semantic model**
   Scopes, symbol table, type checking, explicit capacity declarations, and range analysis. Unbounded allocation is rejected at the declaration site with a `file:line:column` diagnostic.

4. **Memory planner**
   Sizes, offsets, and lifetimes. Stack placement for small bounded values and static regions for larger ones.

5. **C emitter and `--emit-c`**
   Checked IR lowered to valid C: `zlang_*` calls for operations, native `for`/`if` for control flow.

6. **Zlang C API**
   Bounded strings, sequences, print, trap, and checked arithmetic helpers as thin C functions.

7. **Tooling**
   `zc` with `--emit-c` and direct native compilation. Packaging, release artifacts, and the Zlang GitHub Action were intentionally deferred to version 1.0.

8. **Hardening**
   Lexer, parser, and emitter fuzz harnesses. Trap paths covered in child processes. Diagnostics and supported limits documented in `SPEC.md`, including the Annex D conformance matrix.

### Definition of done

A bounded `.z` program compiles to native C-backed code; `--emit-c` shows the generated C; the binary performs zero dynamic allocation; an unbounded program is rejected with a source location; and the whole path runs through the existing CI matrix.

## 0.2 — Practical language ergonomics

Status: Planned.

### Goal

Make 0.1 programs pleasant to write without changing the core promise: no runtime allocation, no hidden dispatch, and no magic in the generated C.

### Scope

1. **Comments**
   Support `//` line comments and `/* ... */` block comments in the lexer. Comments are discarded before parsing and do not affect the AST.

2. **Lexical scopes**
   Introduce explicit block scopes in semantic analysis and unique C identifier generation in the emitter. Nested declarations must not collide in generated C.

3. **Sequence initializers with inferred capacity**
   Support:

   ```zlang
   int32 values[] = {1, 2, 3};
   ```

   The compiler counts the initializers and emits a sequence with capacity 3. An unbounded declaration without an initializer remains a `ZLANG3001` error. Support explicit capacity with a partial initializer:

   ```zlang
   int32 values[5] = {1, 2, 3};
   ```

4. **Index assignment**
   Make `values[0] = 10;` emit valid C with an explicit bounds check followed by a direct store. Remove the current conditional-expression form, which is not an lvalue.

5. **String concatenation**
   Support compile-time concatenation of string literals:

   ```zlang
   string message = "temp_" + "_01";
   ```

   Support concatenation involving variables only through an explicit capacity:

   ```zlang
   string[32] label = prefix + "sensor";
   ```

   The compiler reserves a fixed backing buffer and calls a bounded runtime helper. Overflow traps instead of growing a buffer.

6. **Function calls**
   Lower calls to user-defined functions to direct C calls. The call graph stays static and visible in `--emit-c`.

7. **Ordering and inequality operators**
   Add `<`, `>`, `<=`, `>=`, and `!=`, emitted as native C comparisons.

### Proposed decisions pending confirmation

- Allow shadowing in child blocks and rename generated C identifiers automatically.
- For `int32 values[5] = {1, 2, 3};`, set `capacity_ = 5` and `length_ = 3`.
- Require `string[N]` when concatenation involves a variable; infer capacity only when every operand is a literal.

These are recommendations, not locked decisions. Once confirmed, append the resolution to this section or to a later version section. Do not rewrite this entry.

### Non-goals for 0.2

- `module`, `import`, and `export`.
- Classes, `self`, and methods.
- Heap-backed strings or vectors.
- Packaging, releases, and the Zlang GitHub Action.

### Definition of done

The examples from the 0.2 scope compile and run on the existing CI matrix. Generated C stays readable and direct. Inferred capacities are visible in `--emit-c`. New diagnostics include source locations. Coverage remains at 100% for public headers, and the fuzz harnesses cover the new lexer and parser paths.

## 0.2 — Locked decisions

Status: Locked.

- `1B`: shadowing is rejected with `ZLANG2005 shadowed declaration`.
- `2A`: `int32 values[5] = {1, 2, 3};` means `capacity_ = 5` and `length_ = 3`. An explicit capacity that is never mutated emits a warning so accidental oversized declarations are visible.
- `3A`: string concatenation infers capacity when every operand is a literal. When any operand is a variable, the declaration must use explicit capacity syntax such as `string[32]`. This leaves room for later interpolation.
- `4A`: `int32 values[] = {};` is rejected with `ZLANG3001`.
- `5A`: implementation order is comments, scopes, initializers, index assignment, strings, calls, operators.

## 0.2 — Completion

Status: Implemented.

The 0.2 scope is implemented on top of the locked decisions above. Comments, scopes with unique
generated C identifiers, sequence initializers with inferred capacity, checked index assignment,
bounded string concatenation, direct function calls and ordering and inequality operators are
covered by conformance tests. `ZLANG2005`, `ZLANG2006`, `ZLANG3002` and `ZLANG3003` are wired
through semantic analysis and the command line driver.

## 0.2 — Conditional and logical addendum

Status: Implemented.

Add `else if` chains, boolean `!`, `&&` and `||`, and parenthesized expression grouping. These
features reuse the existing AST and diagnostic set, emit native C control flow and logical
operators, and are covered by new conformance cases in `tests/spec`.

## 0.3 — Hardening and loop control

Status: Implemented.

Add `break` and `continue` with loop-context diagnostics, enforce boolean `if`/`while` conditions
and uint32 `repeat` counts, validate `return` against the enclosing function type, and ensure
nested statements are analyzed and emitted exactly once. The full language tour in `README.md` is
compiled as a conformance case.

## 0.4 — Arithmetic, explicit sequences and readable output

Status: Locked.

- Trap `ZLANG_TRAP_DIVISION_BY_ZERO = 5` for division and remainder by zero.
- Add binary `-`, unary `-`, `*`, `/` and `%` with checked runtime helpers.
- Add compound assignment for arithmetic operators.
- Add `length(sequence)`, emitted as the native `length_` field.
- Add scalar `print` for int32, uint32 and boolean values using fixed-stack helpers.
- Use the leading `sequence[N]` modifier as the only sequence syntax. The postfix form
  `type name[N]` is removed.
- A bounded scalar string is `string[M] name`; a bounded string element is written as
  `sequence[N] string[M] name`.

## 0.4 — Completion

Status: Implemented.

Arithmetic, compound assignment, `length`, scalar printing and the leading `sequence[N]` syntax are
implemented and covered by conformance tests. String sequences use bounded per-element storage.
Trap code `5` reports division by zero. The Doxygen site takes its version from the CMake version
single source of truth.

## 0.5 — Real control and data

Status: Locked.

- Add a native `for (init; condition; update)` statement with scalar initializer, boolean
  condition and expression update.
- Extend `foreach` to `foreach (index, item in sequence)`, exposing the index as `uint32`.
- Add local `const` declarations with mandatory initializer and assignment rejection.
- Allow read-only string indexing that yields one byte as `uint32` with a bounds trap.
- Loops may nest freely, including `for` inside `for` and `foreach` inside `foreach`.

## 0.5 — Completion

Status: Implemented.

Native `for`, indexed `foreach`, local `const` and read-only string indexing are implemented.
Loops nest freely and the full language tour in `README.md` is compiled as a conformance test.
Semantic errors found inside expressions now fail analysis consistently.

## Emitted C layout — Completion

Status: Implemented.

Generated C now separates capacity macros, sequence typedefs, named string literals, user functions
and `main`. Source symbols use `_u_`, compiler temporaries use `_zlang_`, generated types use
`zlang_` and traps use the public `ZLANG_TRAP_*` constants.

## Emitted C layout — Follow-up

Status: Implemented.

Generated sequence types and capacity macros are unique across sibling scopes. Sequence and string
capacities are referenced through their generated macros, loop temporaries use `_zlang_repeat_N`,
and repeated string literals are deduplicated.

## Emitted C readability — Follow-up

Status: Implemented.

Index reads now lower to bounded typed getters instead of nested ternaries. Comparisons are emitted
without redundant outer parentheses, and `(void)` guards are emitted only for declarations that
are actually unused in the generated C.

## Emitted C readability — Statement spacing

Status: Implemented.

The emitter inserts blank lines around structured statements and before the final `return`, so the
body of `main` reads as separated logical blocks instead of a dense statement list.

## Emitted C readability — Named values

Status: Implemented.

Generated numeric constants now use `ZLANG_ZERO`, `ZLANG_ONE` and `ZLANG_TWO`. Sequence initial
lengths and string literal lengths emit named macros instead of bare numeric literals. Source
literals keep the values written by the programmer.

## 0.6 — Records and enums

Status: Locked.

- Add `struct Name { field* }` with scalar or enum fields.
- Add `enum Name { VALUE, ... }` with automatically assigned values.
- Allow struct and enum names as declaration types and sequence element types.
- Add field reads and field assignments, including checked assignment to fields of sequence
  elements.
- Emit named C typedefs for structs and enums and named C constants for enumerators.
- Keep modules, imports, classes, inheritance and dynamic dispatch out of scope.

## 0.6 — Completion

Status: Implemented.

Records, enums, field reads and field assignments are implemented and covered by conformance
tests. Struct and enum declarations emit named C typedefs, enumerators emit named C constants, and
field assignment to sequence elements uses a checked runtime getter with a direct store. Repeated
index reads inside `if / else if` chains are emitted once into a local temporary.

## Emitted comments

Status: Implemented.

Line and block comments are preserved in generated C for source traceability. They remain outside
parsing and do not affect semantics, symbol binding or code generation decisions. A comment is
emitted next to the source construct it documents: declarations, functions, struct fields and enum
items. Consecutive `//` lines stay together as a block, block comments keep their internal line
breaks, and `--disable-comments` omits all source comments from generated C.

## 0.7 — Function types and comment control

Status: Locked.

- Allow struct and enum names as function parameter and return types.
- Emit generated C typedefs for those parameter and return types.
- Allow struct declarations initialized from a function call or another struct value of the same
  type.
- Attach generated comments to the source construct they document instead of hoisting them into
  `main`.
- Add `--disable-comments` to the compiler driver.

## 0.7 — Completion

Status: Implemented.

Functions may now take and return user-defined types, and struct variables may be initialized from
values of the same struct type. Zero-argument functions emit `(void)`, and unused parameters emit
`(void)` guards. Source comments attach to declarations, functions, struct fields and enum items,
with consecutive `//` lines emitted as one block.

## 0.8 — Emitted C symbol roles

Status: Locked.

- Emit local source symbols with a leading `_`.
- Emit function parameters with their source names and no prefix.
- Emit struct members with a trailing `_`.
- Keep user functions with `_u_`, compiler temporaries with `_zlang_`, generated types with
  `zlang_` and macros with `ZLANG_`.

## 0.8 — Completion

Status: Implemented.

Emitted C now makes the role of each source-derived name visible: `_x` is a local, `x` is a
parameter, `x_` is a struct member, `_u_f` is a user function and `_zlang_*` is a compiler
temporary. The README language tour and its generated C output were regenerated with this naming.
Call arguments are stored by explicit expression indices, so nested field and index expressions no
longer shift later arguments.

## 0.9 — Reserved namespace and flat temporaries

Status: Locked.

- Reserve identifiers beginning with `zlang`, `_zlang` or `ZLANG`.
- Reject reserved identifiers with `ZLANG1004`.
- Emit compiler temporaries directly in the enclosing C block.
- Emit `{}` only when the source program contains an explicit block.

## 0.9 — Completion

Status: Implemented.

The generated namespace is now reserved, so compiler temporaries cannot collide with source names.
Index and field temporaries no longer introduce synthetic scopes; only source-authored blocks emit
braces in the generated C.

## 0.10 — Contextual accessor CSE

Status: Locked.

- Resolve a sequence element reference once per lexical context.
- Materialize its checked pointer at the first field access.
- Reuse the pointer for later accesses to the same element.
- Invalidate cached pointers when an index dependency changes or the scope ends.

## 0.10 — Completion

Status: Implemented.

Field pointer temporaries now live in the emitter context instead of a single statement. Consecutive
accesses to the same sequence element share one checked pointer, and assignments to index
dependencies invalidate it.

## 0.11 — Index value CSE

Status: Locked.

- Include branch bodies when collecting repeated index reads for an `if` chain.
- Reuse one local temporary across the conditions and their branch bodies.
- Keep cached index values alive for the whole statement context.
- Invalidate cached index values on writes and scope exit.

## 0.11 — Completion

Status: Implemented.

An index read used in an `if` condition and again inside a branch no longer calls the getter twice.
The value temporary lives for the whole `if` statement, and writes clear the cached reads.

## 0.12 — Parameter qualifiers

Status: Locked.

- Require `clone` or `reference` on every function parameter.
- Allow optional `const` before the mode.
- Lower `clone` to a C value parameter and `reference` to a C pointer parameter.
- Keep pointer syntax out of Zlang source; field access stays `.` at source level.
- Reject missing qualifiers, const mutation and non-lvalue reference arguments.
- Keep references parameter-only: no storage in structs or sequences and no returns.

## 0.12 — Completion

Status: Implemented.

Parameters now declare their binding explicitly. `clone` mutates only a local copy, while
`reference` mutates caller-owned storage through a compiler-managed pointer. The tour demonstrates
both, and reference arguments are checked as lvalues by semantic analysis.

## 0.13 — Reference intent warning

Status: Locked.

- Warn when `const reference` is used for a small value type.
- Warn when a mutable `reference` to a small value type is never mutated.
- Do not warn for mutated references or references to structs.
- Add diagnostic `ZLANG3004`.

## 0.13 — Completion

Status: Implemented.

Semantic analysis now reports unnecessary references for scalar, string and enum parameters. A
reference that actually mutates caller-owned state remains valid and warning-free.

## 0.14 — Array model — Design note

Status: Locked.

- Use one container name: `array`. Do not introduce `vector`.
- Declare rank in the type, for example `array[N] T`, `array[A][B] T`, `array[A][B][C] T`.
- Use an index tuple for generic access: `{i}`, `{i, j}`, `{i, j, k}`.
- Generate offset helpers per rank, not per variable or declaration.
- Generate typed `get` and `set` helpers per `(rank, generated C type)`, deduplicated by generated
  C type. A program using only 1D `int32` arrays gets only the 1D `int32_t` helpers.
- Reuse one offset helper per rank across different extents by passing extents explicitly.
- Keep all storage caller-owned and fixed at compile time. No heap and no hidden allocator.
- Check every dimension before access and trap with `ZLANG_TRAP_BOUNDS`.
- Keep `array_get` and `array_set` as the generic operations; source indexing and tuple access lower
  to the same helpers.

Decision: `array` is purely fixed-shape. It has no logical length and no append operation. The
programmer owns the write index. Bounds are checked per dimension, and `length(array)` reports the
first extent.

## 0.14 — Completion

Status: Implemented.

`sequence` was replaced by fixed-shape `array`, including multidimensional declarations and
indexing. Generated C no longer stores a mutable `length_` field for arrays. Extents are emitted as
macros, rank-2 and higher access uses generated offset helpers, and typed get/set helpers are emitted
per rank and element type. `array_get` and `array_set` source forms are reserved for the next
refinement; source indexing already lowers to the same checked paths.

## 0.15 — Tuple array access

Status: Locked.

- Add `array_get(array, {i, ...})`.
- Add `array_set(array, {i, ...}, value)`.
- Check tuple rank against the declared array rank.
- Lower both forms to generated typed `get` and `set` helpers.
- Include `array_set` in mutation tracking and CSE invalidation.

## 0.15 — Completion

Status: Implemented.

Source code can use explicit index tuples with `array_get` and `array_set`. Rank mismatches are type
diagnostics, and both operations use the same generated typed helpers as multidimensional indexing.

## 0.16 — Inferred arrays

Status: Locked.

- Allow `array T name;` with rank and extents inferred from initializer, constant indices and
  canonical loop induction ranges.
- Allow `array` as an inferred parameter and return type in function signatures.
- Infer one concrete array shape per inferred function from call arguments or return assignment.
- Emit canonical array typedefs by shape so inferred locals, parameters and returns share one C type.
- Report `ZLANG3001` when rank or extent cannot be proved at compile time.

## 0.16 — Completion

Status: Implemented.

Local inferred arrays and inferred array function signatures are resolved before semantic analysis.
Function signatures use one inferred shape; conflicting specializations are deferred.

## 0.17 — Internal array helpers

Status: Locked.

- Remove `array_get` and `array_set` from Zlang source.
- Keep generated `_zlang_array_get_*` and `_zlang_array_set_*` helpers private to emitted C.
- Lower multidimensional source indexing directly to those helpers.

## 0.17 — Completion

Status: Implemented.

The only array access syntax in Zlang is `array[i]...`. Generated C uses shape-typed helpers for
bounds-checked reads and writes, but users never name them.

## 0.18 — Complete arrays

Status: Implemented.

- Support multiple monomorphizations per inferred array function. A function written with `array`
  parameters or return types may be called with different shapes; each shape gets its own generated
  C function and canonical array typedef.
- Support nested array initializers:
  ```zlang
  array int32 grid = {
      {1, 2, 3},
      {4, 5, 6}
  };
  ```
  The initializer shape contributes rank and extents to inference.
- Define multidimensional iteration. `foreach (row, col in grid)` visits every index tuple in
  row-major order; the index variables are uint32 and bounds are compile-time known from the array
  shape.
- Allow array fields in structs. A struct field may be `array[A][B] T`, emitted as an embedded
  canonical array typedef with its own bounds checks.
- Allow `const array[A][B] T` declarations and reject indexed writes to them.
- Allow whole-array assignment and copy for compatible shapes:
  ```zlang
  destination = source;
  ```
  Shape mismatch is a type diagnostic.

Acceptance criteria:

- A generic inferred-array function can be called with two shapes in one program.
- Nested initializers infer the same extents as explicit declarations.
- `foreach` over a 2D array visits all cells exactly once.
- Arrays work as struct fields without losing bounds checks.
- `const` arrays reject writes and whole-array copies require identical shape.

The implementation supports nested array initializers, multidimensional `foreach` in row-major
order, arrays as struct fields with bounds-checked access, `const` arrays, whole-array copy for
identical shapes, and one generated C function per inferred array shape.

## 0.18.1 — Completion

Patch release after the 0.18 array model. Generated C now derives array extents from the accessed
symbol instead of the active function specialization, array fields use literal extents in typed
helper calls, string capacity macros are emitted without arrays, inferred array function calls
resolve their return type without an emitter symbol, scalar `reference` parameters are read and
written through a dereference, `foreach` iterables count as reads, dynamic `int32` indices are
cast to `uint32_t`, and logical negation of a comparison keeps the comparison grouped.

The `examples/` tree is now a golden suite: every example has committed generated C and expected
stdout, grouped under `algorithms/` and `features/` and checked by CTest. Each example is a
self-contained directory with `main.z`, generated `main.c`, `expected.txt` and a `README.md`
explaining the constructs it exercises; `examples/README.md` indexes both categories. The
multidimensional CLI smoke fixture covers rank 3 and rank 4 helper generation. Semantic symbols now
copy array rank and extents so whole-array copy never reads uninitialized symbol state under
valgrind.

## 0.19 — Completion

Zlang 0.19 cleans up emitted C and removes two ordering restrictions. The emitter emits only the
array helpers actually referenced by the generated program, so `_zlang_array_helpers_keep` and its
call in `main` are gone. User functions are registered before emission and a generated prototype
block appears after the declarations, allowing a function body to call a function defined later in
the source. Inferred array specializations get one prototype per generated C function.

The compiler driver still builds native binaries through the configured C compiler. On Windows it
launches the compiler directly with `CreateProcessA`, avoiding `cmd.exe` quote loss for paths such
as `C:/Program Files/...`, and selects MSVC or GNU flags from the compiler frontend so MinGW GCC
receives `-I` / `-o` instead of `/I` / `/Fe`.

## 0.20 — Completion

Zlang 0.19 exposes no user-callable standard functions: the runtime API is used only by generated
C. The next language level adds a domain-based standard library and an optional transpile-time
optimizer. The complete catalog, naming rules and classification dimensions are documented in
[`docs/standard_library.md`](docs/standard_library.md). This release implements imports, the
standard library registry, `random.generate_int32`, the generated C provenance prefixes and the
first transpile-time optimizer with `--tto`.

Imports are intention-first:

```zlang
import { generate_int32, generate_bytes } from "random";
import { tcp_listener, send_bytes } from "networking";
```

Generated C carries explicit provenance prefixes: `app_*` for names translated from user source,
`zlang_gen_*` for compiler support derived from source, `zlang_std_*` for standard library
functions, `zlang_*` for the public runtime API and `ZLANG_*` for language constants. A source
literal's content belongs to the user, but its generated C storage is compiler support and uses
`zlang_gen_*`.

Domains are full, expressive capability names: `random`, `algorithms`, `text`, `encoding`, `math`,
`memory`, `time`, `streams`, `filesystem`, `networking` and `system`. Function names say what they
do: `generate_int32`, `parse_int32`, `encode_hex`, `sort_int32`, `tcp_listener`. Generic names such
as `listener`, abbreviations such as `inet`, and catch-all domains such as `util` are rejected.

The emitter activates only the runtime headers the program uses. Each activated function has one
macro, and `runtime.h` includes standard headers in two phases: prototypes first, `static inline`
bodies second. Dependencies are declared by standard library metadata and activated transitively by
full registry id, such as `algorithms.shuffle_int32` depending on `random.generate_int32`.
Function-call cycles are legal because every prototype is visible before any implementation body is
compiled.

Registration is declarative. Each domain owns a registry file, and each function is registered once
with its signature, C symbol, lifecycle (`DRAFT`, `UNSTABLE`, `STABLE`), safety (`SAFE`, `UNSAFE`),
evaluation, determinism, resource, error, portability and optimizer classification, plus dependency
ids. Configure-time generation writes activation macros, declaration headers, the semantic signature
table and documentation. Maintainers implement only the C body in the matching standard header.

Standard functions do not declare mutable globals and do not use hidden mutable `static` state. If a
stateful deterministic PRNG is chosen, runtime state is explicit. The emitter adds a hidden state
parameter only to generated C functions that transitively call stateful standard functions. Source
signatures remain unchanged; prototypes, definitions and call sites receive the hidden parameter
together. A future global-state feature must expose state through an explicit runtime context type.

### Transpile-time optimization

`--tto` enables transpile-time optimization. Default output remains unoptimized and stable.

The optimizer is a fixed-point rewriter with a caller-owned context, a worklist and generation
counters:

- a rewrite may expose another rewrite, so changed nodes are reconsidered;
- any rewrite that invalidates prior analysis restarts or requeues affected nodes;
- termination requires a finite lattice, monotonic facts and a fixed capacity;
- the optimizer never changes program meaning.

Initial rewrite families:

- constant expression evaluation for literals, arithmetic, comparisons, logical operators and
  `length` when extents are known;
- literal array index folding into direct reads and writes when the index is proven in range;
- out-of-range literal indices keep the declared trap semantics;
- control-flow folding for conditions that are constant after earlier rewrites;
- nested call and nested expression materialization into explicit sequential temporaries, preserving
  evaluation order;
- function evaluation when the function is pure, has constant arguments and contains no opaque
  operation;
- `random.generate_int32` is a barrier: its arguments may be folded, but its result is never folded,
  cached or reordered.

Additional cleanup in 0.20:

- emit value constants only when referenced;
- remove legacy `ZLANG_*_LENGTH` aliases from array output;
- keep parameter names local to their function scope, so separate functions and specializations can
  reuse the same parameter name;
- reduce long generated lines through materialization and multiline signatures.

Acceptance criteria:

- `line[0] = 9` under `--tto` emits a direct store without a bounds `if` for that literal index;
- a later read of the same unmodified cell may become a constant under `--tto`;
- any may-write to that array cell invalidates the known value;
- `generate_int32(1, 6)` is never constant-folded under `--tto`;
- imported and unimported standard functions are distinguished by semantic analysis;
- a standard function may call another standard function regardless of source import order;
- dependencies are activated from metadata, not from user imports;
- every standard function has a micro CMake test project with direct C cases and Zlang integration
  programs;
- generated C compiles under the strict warning set in both default and `--tto` modes.

## 0.21 — Completion

Zlang 0.21 adds user modules and an explicit program entry point. A compilation still produces one
C file with one C `main`; modules are assembled into that single translation unit rather than linked
as separate programs.

Every top-level declaration in a module is public. No `export` keyword is needed. Importing a
declaration binds a reference to the canonical entity: functions, variables, structs and enums are
emitted once and shared by every module that imports them. Two files importing the same variable
refer to the same generated object. Two modules may define the same local name because generated
names are derived from canonical module paths.

Module specifiers use three forms:

```zlang
import { my_function } from "./lib/my_function";
import { my_function } from "@lib/my_function";
import { generate_int32 } from "@std/random";
```

Relative paths resolve from the importing file. `@std` is reserved for standard domains. Other `@`
names are aliases declared in `z.yaml`. `@std/random` is canonical; `from "random"` remains a
compatibility form. Module imports are not transitive by name, but signatures resolve by canonical
entity identity.

The entry point is explicit and receives the host arguments:

```zlang
function main(int32 argc, arguments argv): int32 {
    return 0;
}
```

`main` is an ABI-shaped declaration, not an ordinary user function. Parameter qualifiers are omitted
because the binding modes are fixed by the signature: `argc` is an implicitly cloned `int32`
because it is a trivial scalar, and `argv` is an implicitly referenced, read-only `arguments`
handle because it aliases caller-owned host state. Writing `clone` or `reference` in this signature
is rejected so there is exactly one canonical form. The mandatory qualifier rule remains unchanged
for every other function.

`arguments` is a read-only opaque handle created by the C `main` wrapper from `argc` and `char **argv`.
Zlang does not expose pointer syntax, and generated C still uses `int main(int argc, char **argv)`.

Command line parsing belongs to `@std/program_options`, not to the language core. The first version
provides `argument_count`, `argument_at`, `has_option`, `option_value` and `has_flag`. They return
bounded string views over `argv` and never allocate.

At this stage a user module has no hidden global runtime state of its own. Module variables are
emitted once as module-qualified C objects; they are user-owned module state, not runtime state.
The module graph is resolved before semantic analysis, with cycle detection and deterministic
ordering. Generated names come from canonical paths, never from aliases.

The implementation assembles all loaded modules into a single Zlang source before parsing. Each
module is loaded once; imported names are bound to canonical entity names; module variables are
emitted once as file-scope objects; and enum items imported through an enum declaration are bound
automatically. CLI coverage verifies relative imports, aliases from `z.yaml`, `@std` imports from
dependencies, duplicate local names in separate modules, shared variables, structs, enums,
functions, cyclic import rejection, root escape rejection, canonical generated names and the
explicit `main` entry point.

## 0.22 — Design note

Status: Locked.

Zlang 0.22 completes the numeric primitive set and resolves the deferred runtime-state and
inferred-array questions. It also removes backward compatibility from standard imports: the only
canonical form is `@std/<domain>`. A specifier such as `random` or `program_options` is no longer a
standard import; it is resolved as a user module path and fails if no such module exists. Existing
sources must be migrated, and current documentation and tests use only the canonical spelling.

### Numeric primitives

The scalar set gains four primitives:

| Zlang type | Width and sign | Generated C type |
| --- | --- | --- |
| `int32` | signed 32-bit | `int32_t` |
| `uint32` | uint32 32-bit | `uint32_t` |
| `int64` | signed 64-bit | `int64_t` |
| `uint64` | uint32 64-bit | `uint64_t` |
| `float` | IEEE-754 binary32 | `float` |
| `double` | IEEE-754 binary64 | `double` |

`boolean` and `string` keep their 0.21 definitions. `arguments` remains an opaque entry-point
handle. There is no implicit conversion between any two numeric types. Every widening or narrowing
uses an explicit conversion call named for the target type: `to_int32`, `to_uint32`, `to_int64`,
`to_uint64`, `to_float` and `to_double`. There is no `reinterpret` or other bit-level conversion form
in 0.22. Conversion to an int32 type traps on an out-of-range value; conversion from a
floating-point value truncates toward zero and traps on non-finite input.

Integer literals keep the decimal spelling and the `u` suffix. Literals for 64-bit integers use
the `i64` and `u64` suffixes. Floating-point literals are decimal with either a decimal point or
an exponent; the `f` suffix selects `float`, and no suffix selects `double`. Integer literals do
not acquire a floating-point type implicitly.

Arithmetic follows the C/IEEE-754 model for each emitted type. Integer division by zero keeps the
existing Zlang trap. Floating-point division follows IEEE-754: division by zero produces an
infinity or NaN according to the standard, with no Zlang trap. `%` remains valid only for int32
types. Comparisons between numeric types require both operands to have exactly the same type.
Printing uses bounded runtime helpers with deterministic textual forms; generated programs still
perform no dynamic allocation.

### Standard random functions

`@std/random` exposes `generate_int32`, `generate_uint32`, `generate_int64`, `generate_uint64`,
`generate_float32` and `generate_float64`. Floating-point generators use the existing deterministic
PRNG and produce values in `[from, to)`. Invalid ranges trap with the existing invalid-argument
trap. Results remain opaque to `--tto`: arguments may be folded, but calls are never folded.

### Explicit runtime context

The runtime state becomes one caller-owned C object named `zlang_runtime_context_t`. It contains
the deterministic PRNG state and any future runtime-owned state. The emitter passes it as a hidden
parameter only to generated C functions that transitively need runtime state. Zlang source never
exposes a `context` type, keyword or parameter, and module variables remain user-owned file-scope
objects. This is an internal generated-C contract, not a user-visible language feature.

### Inferred array specializations

Inferred array functions are monomorphized by concrete shape: element type, rank and every extent.
A function called with `array[4] int32` and `array[2][3] int32` emits two C functions with
canonical shape-derived names. Semantic analysis resolves the inferred shape for each call site and
validates the function body for every distinct shape. Two different shapes are not a conflict;
they are two specializations. A conflict is reported only when one call site cannot provide a
unique shape, or when the function body is invalid for one of its inferred shapes.

### Acceptance criteria

- `float`, `double`, `int64` and `uint64` work in declarations, parameters, returns, arrays,
  structs and arithmetic;
- explicit conversions are required and reject implicit numeric coercion;
- floating-point division by zero follows IEEE-754 and does not trap;
- standard random functions remain deterministic, stateful and opaque to `--tto`;
- generated C uses one internal `zlang_runtime_context_t` instead of per-domain hidden state;
- inferred array functions called with different shapes emit one canonical C function per shape;
- `from "random"` is rejected and every current fixture uses `@std/random`;
- at least one golden example demonstrates a multi-module project;
- the full CI matrix, sanitizers, coverage, fuzzers, docs and package checks stay green.

## 0.22 — Completion

Zlang 0.22 is implemented at language level 0.22. The numeric set contains `int32`, `uint32`,
`int64`, `uint64`, `float`, `double`, `boolean`, `string` and the opaque `arguments` handle. Integer
literals accept `u`, `i64` and `u64` suffixes; floating literals accept a decimal point or exponent
and the `f` suffix. There is no implicit numeric conversion. Conversion calls are named for their
target type and trap on out-of-range or non-finite input where required. Integer arithmetic remains
checked at the emitted C level; floating arithmetic follows IEEE-754, so division by zero produces
an infinity or NaN and does not trap.

`@std/random` now exposes six deterministic generators: `generate_int32`, `generate_uint32`,
`generate_int64`, `generate_uint64`, `generate_float32` and `generate_float64`. Every generator uses
the same PRNG state, embedded in one internal `zlang_runtime_context_t`. The emitter passes that
context only to generated C functions that need it; no `context` type or keyword is visible in Zlang
source. Full-range 64-bit ranges are handled without overflow.

Inferred array functions are monomorphized by concrete element type, rank and extents. The emitter
registers one C function per shape, gives each one a canonical shape-derived name, and selects the
right specialization at each call site. A call whose shape cannot be proved is still rejected with
`ZLANG3001`.

The legacy `from "random"` spelling was removed in the previous change. All current sources use
`@std/random`, and the golden example suite includes a real multi-module project.

## 0.23 — Design note

Status: Locked.

Zlang 0.23 closes the type-discipline gaps exposed by 0.22 and adds the first scalar and string
standard APIs. No new primitive type is introduced.

### Numeric discipline

Every binary arithmetic operator requires two operands of exactly the same numeric type. There are
no hidden promotions in arithmetic or comparisons. `to_*` remains the only conversion form.
Unary negation is checked for `int32` and `int64`, trapping when the operand is the minimum value
of its type; `float` and `double` keep IEEE negation.

### String discipline

String equality and inequality compare content through a bounded runtime helper. Ordering
operators are rejected for strings because a byte-wise order is not a Zlang string contract.
Mixed string/numeric comparisons are rejected.

### Standard APIs

`@std/math` exposes `abs`, `clamp`, `min`, `max` and `sqrt`. The first four are overloaded by
argument type across `int32`, `uint32`, `int64`, `uint64`, `float` and `double`; `sqrt` is
provided for `float` and `double`. Overload resolution uses the argument types recorded by semantic
analysis, and each concrete overload has one activation macro and one generated C symbol.

`@std/strings` exposes `compare`, `contains` and `starts_with`. They are pure, bounded, non-owning
and allocation-free. The catalog uses `@std/strings`; `@std/text` is not a standard domain.

### Acceptance criteria

- mixed numeric arithmetic and comparisons are rejected;
- string equality works by content and string ordering is rejected;
- unary negation of signed minimum values traps;
- all `@std/math` overloads resolve by argument type and generate one C implementation each;
- all `@std/strings` functions work with bounded string views;
- the full CI matrix, sanitizers, coverage, fuzzers, docs and package checks stay green.

## 0.23 — Completion

Zlang 0.23 is implemented at language level 0.23. Binary arithmetic and comparisons reject mixed
operand types, string equality compares content through `zlang_string_equal`, string ordering is a
type diagnostic and unary negation of signed integers traps on the minimum value.

`@std/math` exposes `abs`, `clamp`, `min`, `max` and `sqrt`. Overloads are resolved from semantic
argument types; the emitter activates one macro and emits one C symbol per used overload. Generated
math uses only fixed C operations and bounded iteration, with no `libm` dependency.

`@std/strings` exposes `compare`, `contains` and `starts_with`. The domain is named `strings`; there
is no `@std/text` domain. All three functions operate on bounded non-owning string views.

The golden suite includes the arithmetic golden regenerated for checked unary negation. New spec,
runtime and standard-library modules cover numeric discipline, string equality, string ordering
rejection, checked negation and the new standard APIs.

## 0.24 — Design note

Status: Locked.

Zlang 0.24 moves output out of the language core. `print` is no longer a keyword; it is rejected as
an undeclared identifier. The only source-level output function is `write`, imported from
`@std/io`. There is no compatibility spelling.

`@std/io.write` is overloaded by argument type across `string`, `boolean`, `int32`, `uint32`,
`int64`, `uint64`, `float` and `double`. Each overload has one activation macro and one generated C
symbol. The wrappers call the existing bounded runtime print primitives, so generated programs keep
zero dynamic allocation. Enum values use explicit `to_int32(...)` before `write`.

`@std/streams` remains reserved for future explicit stream handles. `read` is not introduced in this
release because input source, EOF, bounded destination storage and trap semantics need a separate
contract.

## 0.24 — Completion

Zlang 0.24 is implemented at language level 0.24. The `print` keyword and its AST node were
removed. Output is now a standard import:

```zlang
import { write } from "@std/io";

write("hello");
```

`@std/io.write` is resolved by argument type and emits one of eight bounded wrapper calls. Runtime
primitives remain available to generated C, but source programs cannot write without importing the
domain. Enum values are written through explicit `to_int32(...)`. No compatibility path for
`print` exists.

The golden example suite, CLI fixtures, standard-library tests, spec matrix and documentation were
migrated to `write`. Every golden example now declares its own explicit `main` function and its
generated C calls `app_fn_main` from the single C `main`. `read` is deliberately deferred until its
input contract is specified.

## 0.25 — Design note

Status: Locked.

Zlang 0.25 adds bounded input to `@std/io`. Both `read` and `read_line` take one mutable bounded
string destination and return `int32`: `-1` at EOF, otherwise the number of logical bytes read.
Line endings are normalized across platforms: CRLF is one logical newline. `read` keeps that newline
in the destination; `read_line` removes it. If the destination cannot hold the logical line, the
program traps with `ZLANG_TRAP_CAPACITY`. No buffer grows and no allocation occurs.

## 0.25 — Completion

Zlang 0.25 is implemented at language level 0.25. `@std/io` now exposes `read`, `read_line` and
`write`. Input destinations are caller-owned bounded strings, EOF is reported as `-1`, line endings
are normalized across platforms and overflow traps without allocation.

Every golden example now has an explicit Zlang `main`, and all committed C goldens were regenerated
through `zc --tto`. The CLI suite includes a stdin-driven test for `read` and `read_line`.

## 0.26 — Design note

Status: Locked.

Zlang 0.26 adds range-proven index folding to `--tto`. The optimizer recognizes canonical `for`
loops over signed `int32` variables with constant start, constant `<` or `<=` bound and `+= 1`
update. It proves the range of the induction variable and of `variable +/- constant` expressions.
One-dimensional array reads and writes whose whole proven range is inside the declared extent are
emitted as direct C `data_[]` accesses. A non-canonical loop, a mutated induction variable or any
index outside the proven range keeps the bounds check and `ZLANG_TRAP_BOUNDS`.

## 0.26 — Completion

Zlang 0.26 is implemented at language level 0.26. With `--tto`, canonical signed `for` loops with
constant bounds and `+= 1` updates prove induction-variable ranges. Safe one-dimensional reads and
writes become direct C `data_[]` accesses; the `fibonacci` golden now contains no
`ZLANG_TRAP_BOUNDS` and no checked `zlang_array_get_i32` calls. Unproven indices keep their original
bounds checks. Golden C files were regenerated through `zc --tto`.

## 0.27 — Design note

Status: Locked.

Zlang 0.27 restores readable unwinding for range-proven index folding. When a proven direct store
uses an arithmetic expression over nested proven reads, the emitter declares one typed temporary per
nested read in evaluation order and emits the final store from those temporaries. This replaces one
long generated line with short sequential lines while preserving the semantics of 0.26.

## 0.27 — Completion

Zlang 0.27 is implemented at language level 0.27. The `fibonacci` golden now emits two
`zlang_gen_read_*` temporaries followed by a short `zlang_add_i32` store. Unproven accesses keep
their bounds checks. Golden C files were regenerated through `zc --tto`.

## 0.28 — Design note

Status: Locked.

Zlang 0.28 replaces `record` with `struct` and adds struct members without introducing classes.
There is no inheritance, no dynamic dispatch, no heap allocation and no `new`. A struct declares
fields, at most one constructor, instance methods and `static function` methods. The source never
writes a receiver: inside instance methods and constructors, fields are referenced directly and the
emitter materializes the generated `self` parameter or local. Instance calls use
`value.method(...)`; static calls use `Struct.method(...)`; construction uses `Struct(...)`. Method
names and field names share one namespace inside a struct.

## 0.28 — Completion

Zlang 0.28 is implemented at language level 0.28. Generated C declares `app_struct_<name>_t` types
and `app_fn_<struct>_<method>` functions, with `self` supplied only for instance methods and
constructors. The golden suite includes `features/struct_methods`.

## 0.29 — Design note

Status: Locked.

Zlang 0.29 adds single inheritance, polymorphism and traits. `struct Derived : Base` embeds the base
as its first C subobject. All instance methods are virtual by default; no `virtual` keyword is
needed. Each polymorphic hierarchy has an internal vtable and each polymorphic subobject has an
internal type tag and vtable pointer. Upcasting is implicit by value or by `reference`; downcasting
is not provided. Arrays store exact element types and do not use inheritance.

A trait is static behavior reuse: `trait Name requires ... { methods }` and `struct S uses Name`.
Trait methods are compiled into the using struct and do not use vtable dispatch. Trait methods cannot
override inherited methods.

The performance cost of tags, vtables and indirect calls is accepted by the user. The generated C
still performs no dynamic allocation.
## 0.29 — Completion

Zlang 0.29 is implemented at language level 0.29. A struct can declare a single base with
`struct Derived : Base`; the base is emitted as the first C subobject and derived values can be
used where a base value or `reference Base` is expected. Downcasting is rejected.

All instance methods are virtual by default. Each polymorphic hierarchy has one generated vtable
type with one slot per virtual method, and each concrete struct gets its own vtable instance. A
hidden type tag and vtable pointer are emitted in the root subobject. Constructors and default
struct initializers set them. Calls through a base receiver are lowered to indirect vtable calls;
calls on a known concrete receiver can still be resolved directly.

Traits use `trait Name requires Type field, ... { methods }` and `struct S uses Name`. Trait methods
are compiled into the using struct as ordinary C functions and are called directly. They do not use
the vtable and cannot override inherited methods. The golden suite includes
`features/inheritance_traits`.

Inheritance has a runtime cost: larger structs, tags, vtable pointers and indirect calls. The
generated program still performs no dynamic allocation and uses no runtime type registry beyond the
internal dispatch tables. The emitter derives tags, vtable types, slots and instances directly from
the AST, so this metadata has no fixed emitter-side capacity and adds no stack storage.

## 0.30 — Design note

Status: Locked.

Zlang 0.30 adds compile-time generics for structs and functions. A generic declaration lists up to
four type parameters, for example `struct Pair<T, U>` or `function identity<T>(clone T value): T`.
Type applications use `Pair<int32, boolean>`. The compiler monomorphizes each used application
into a concrete C99 type or function; there is no heap, no runtime type reflection and no generic
value representation.

The MVP supports concrete type arguments and type parameters already in scope. Generic methods,
generic traits, inheritance over generic structs and variadic type packs are not part of 0.30.
Capacity exhaustion produces a diagnostic instead of a silent failure.

The canonical stdlib result shape is `Result<T, E>`: `T` is the value the function would
ideally return, `E` is the error code, and `what` carries a bounded message. A function with no
payload uses `Result<void, Status>`. Stdlib normalization is scheduled for 0.31; 0.30 only
provides the language mechanism.


## 0.30 — Completion

Zlang 0.30 is implemented at language level 0.30. Structs and functions can declare up to four type
parameters and be applied to up to four concrete type arguments. Applications such as
`Pair<int32, boolean>` and `identity<int32>(1)` are monomorphized into concrete C99 types and
functions; no heap, runtime reflection or generic value representation is emitted.

Generic structs cannot inherit, use traits or declare methods. Generic functions must be top-level
and have flat bodies. Invalid applications, duplicate type parameters and capacity exhaustion are
reported through diagnostics. The canonical result shape for later stdlib normalization is
`Result<T, E>`, with `T` as the ideal value, `E` as the error code and `what` as a bounded message.

## 0.31 — Design note

Status: Locked.

Zlang 0.31 adds native generic facilities for the standard library. The compiler registry and the
public C headers share a closed set of `Result<T, Status>` specializations. `Result<T, E>` keeps `T`
as the ideal value, `E` as the error code and `what` as a bounded read-only message. A function with
no payload uses `Result<void, Status>`.

Native result layouts live in `<zlang/standard/result/result.h>`; the compiler maps canonical Zlang
applications such as `Result<int32, Status>` to those C types. The stdlib registry records
structured return metadata and parameter modes, including `const reference`. No runtime allocation,
formatting or reflection is introduced.

Native stdlib parity must be reviewed whenever a native function is added: its compiler registry
entry, public C header and documentation must agree. A future binary tool should verify registry,
header and documentation parity mechanically. That tooling is planned but is not required for the
facility itself.


## 0.31 — Completion

Zlang 0.31 is implemented at language level 0.31. Native `Result<T, Status>` applications map to
canonical C layouts in `<zlang/standard/result/result.h>`. Results expose `code`, `what` and, for
non-void payloads, `value`. Status constants are available in Zlang source and emit their C enum
constants.

The compiler registry now carries structured return metadata and parameter modes, including
`const reference`, while existing stdlib functions keep their 0.30 signatures. Native result types
activate `ZLANG_STD_RESULT` in generated C. No stdlib function is migrated in 0.31; the facility
exists so 0.32 can migrate IO without mixing compiler and header contracts.

## 0.32 — Revised IO design note

Status: Locked.

IO will be redesigned around explicit stream handles and byte-transfer results. `write` and `read`
both return `Result<int32, Status>`: `value` is the number of bytes transferred and `code` is
`SUCCESS`, `END_OF_INPUT` or `IO_ERROR`. The model matches the `error_code, bytes_transferred`
shape used by Boost.Asio, adapted to synchronous allocation-free Zlang.

Streams are borrowed handles. `stdin`, `stdout` and `stderr` are ordinary stream values, and the
same API must support future stream facilities such as sockets. Convenience overloads may use
implicit stdin or stdout, but the explicit stream overload is canonical.

`read` writes into a bounded destination view at an explicit offset and maximum byte count. `write`
borrows a bounded source view. Neither operation copies buffer bytes; string and stream parameters
are views and the docs must say so. Null storage, invalid offsets, zero capacity and capacity
overflow remain traps. Recoverable stream conditions are returned as `Result<int32, Status>`.

## 0.34 — Design note

Status: Locked.

Zlang 0.34 will rename the built-in scalar types `int32` and `uint32` to their real widths:
`int32` and `uint32`. The generated C already uses `int32_t` and `uint32_t`; this change makes the
source-level names match the emitted representation and aligns them with the existing `int64` and
`uint64` names.

The explicit conversion calls follow the same rule: `to_int32` becomes `to_int32` and
`to_uint32` becomes `to_uint32`. `to_int64`, `to_uint64`, `to_float` and `to_double` are unchanged.

This is a breaking language rename. No aliases or compatibility layer will be added. The change
must update the lexer keyword table, parser type checks, semantic type rules, stdlib registry
signatures, diagnostics docs, tests, goldens and site documentation. Runtime helper names such as
`zlang_print_i32` and stdlib function names such as `generate_int32` remain unchanged unless a
separate decision is made.

## Pending implementation checklist

- **0.31 — Native result facilities**: implemented in the working tree; not committed or pushed.
- **0.32 — IO redesign**:
  - `write` and `read` return `Result<int32, Status>` with transferred bytes in `value`.
  - IO accepts explicit stream handles; `stdin`, `stdout` and `stderr` are ordinary streams.
  - Future sockets must fit the same stream API.
  - `read` takes destination view, offset and maximum byte count.
  - String and stream parameters are borrowed views; no buffer bytes are copied.
  - Recoverable conditions use `Result`; null storage, invalid offsets and capacity failures trap.
  - Registry, header and docs must stay in parity.
- **0.33 — Generic methods**:
  - first target is `Result` helpers: `is_ok()`, `get()` and `message()`;
  - then general methods on generic structs and functions;
  - monomorphized per specialization, with no runtime dispatch;
  - generic traits and generic inheritance remain out of scope.
- **0.34 — Scalar type rename**:
  - `int32` becomes `int32`;
  - `uint32` becomes `uint32`;
  - `to_int32` becomes `to_int32`;
  - `to_uint32` becomes `to_uint32`;
  - no aliases or retrocompatibility;
  - update compiler, stdlib registry, docs, tests, goldens and site.
- **Tooling**:
  - add a future binary verifier for native stdlib parity between compiler registry, public C
    headers and documentation.

## 0.32 — Completion

Zlang 0.32 is implemented at language level 0.32. `write` and `read` return
`Result<int32, Status>` with the transferred byte count in `value`. Convenience overloads keep
implicit stdout and stdin; explicit overloads accept `STDIN`, `STDOUT` and `STDERR` as borrowed
stream handles. Future stream facilities can use the same native `stream` type.

`write` borrows a bounded string view. Because `zlang_string_t` is a trivially copyable descriptor,
the convenience overload clones the descriptor and never copies buffer bytes. `read` writes into a
bounded destination view at an explicit offset and count. Recoverable stream conditions are
reported as `END_OF_INPUT` or `IO_ERROR`; null storage, invalid offsets, zero capacity and capacity
overflow remain traps.

Native IO parity is enforced by matching compiler registry entries, public C headers and docs.

## 0.33 — Completion

Zlang 0.33 is implemented at language level 0.33. Native `Result<T, E>` values expose `is_ok()`,
`get()` and `message()` through direct method syntax. These helpers are lowered to C field reads
and `zlang_std_status_is_ok`; they add no vtable or runtime dispatch.

Generic structs can declare flat instance methods. Each generic application monomorphizes its
methods with the same substituted type names used for fields and signatures, and emits ordinary
`app_fn_<specialization>_<method>` functions. Constructors, static methods, nested control flow in
generic methods, generic traits and generic inheritance remain outside scope.

## 0.34 — Completion

Zlang 0.34 is implemented at language level 0.34. Built-in signed and unsigned 32-bit types are
named `int32` and `uint32`, matching their C representations `int32_t` and `uint32_t` and aligning
with `int64` and `uint64`. Explicit conversions are named `to_int32` and `to_uint32`; the 64-bit and
floating conversion names are unchanged.

This is a breaking rename with no aliases or compatibility layer. Compiler keywords, AST type
names, stdlib signatures, diagnostics fixtures, examples, goldens, tests and documentation were
updated together. Runtime helper names such as `zlang_print_i32` remain unchanged.

## 0.35 — Design note

Status: Locked.

Zlang 0.35 fixes the native stream contract before adding new IO domains. A stream is a trivially
copyable borrowed handle with a constant operations table and a caller-owned state pointer. The
operations are `read(source, destination, offset, count)` and
`write(destination, source, offset, count)`; the convenience overloads keep implicit stdin and
stdout, but the canonical API is explicit on both ends.

The stream type does not expose `FILE *`, sockets, descriptors or hardware pins. Stdio, memory,
POSIX descriptors, Windows handles, sockets, pipes and device backends implement the same native
operations table. New IO APIs consume `stream` instead of introducing parallel read or write
families.

## 0.35 — Completion

Zlang 0.35 is implemented at language level 0.35. `<zlang/standard/io/stream.h>` defines the stable
`zlang_stream_t` handle, the constant `zlang_stream_ops_t` table and generic `zlang_stream_read` and
`zlang_stream_write` helpers. Stdio and caller-owned memory streams implement the contract.
`<zlang/standard/io/io.h>` now uses only the stream contract, and `write` supports an explicit
string window overload: `write(destination, source, offset, count)`.

Status codes live in `<zlang/standard/status/status.h>` and are shared by result values and stream
operations. No mutable global state or heap allocation is introduced. Future socket, pipe and
hardware backends can plug into the same contract without changing Zlang signatures.

The pending checklist earlier in this file is historical: 0.30 through 0.35 are implemented. Future work starts from the stable stream contract and the version sections above.
