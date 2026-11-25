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
   integer values[] = {1, 2, 3};
   ```

   The compiler counts the initializers and emits a sequence with capacity 3. An unbounded declaration without an initializer remains a `ZLANG3001` error. Support explicit capacity with a partial initializer:

   ```zlang
   integer values[5] = {1, 2, 3};
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
- For `integer values[5] = {1, 2, 3};`, set `capacity_ = 5` and `length_ = 3`.
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
- `2A`: `integer values[5] = {1, 2, 3};` means `capacity_ = 5` and `length_ = 3`. An explicit capacity that is never mutated emits a warning so accidental oversized declarations are visible.
- `3A`: string concatenation infers capacity when every operand is a literal. When any operand is a variable, the declaration must use explicit capacity syntax such as `string[32]`. This leaves room for later interpolation.
- `4A`: `integer values[] = {};` is rejected with `ZLANG3001`.
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
and unsigned `repeat` counts, validate `return` against the enclosing function type, and ensure
nested statements are analyzed and emitted exactly once. The full language tour in `README.md` is
compiled as a conformance case.

## 0.4 — Arithmetic, explicit sequences and readable output

Status: Locked.

- Trap `ZLANG_TRAP_DIVISION_BY_ZERO = 5` for division and remainder by zero.
- Add binary `-`, unary `-`, `*`, `/` and `%` with checked runtime helpers.
- Add compound assignment for arithmetic operators.
- Add `length(sequence)`, emitted as the native `length_` field.
- Add scalar `print` for integer, unsigned and boolean values using fixed-stack helpers.
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
- Extend `foreach` to `foreach (index, item in sequence)`, exposing the index as `unsigned`.
- Add local `const` declarations with mandatory initializer and assignment rejection.
- Allow read-only string indexing that yields one byte as `unsigned` with a bounds trap.
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

- Add `record Name { field* }` with scalar or enum fields.
- Add `enum Name { VALUE, ... }` with automatically assigned values.
- Allow record and enum names as declaration types and sequence element types.
- Add field reads and field assignments, including checked assignment to fields of sequence
  elements.
- Emit named C typedefs for records and enums and named C constants for enumerators.
- Keep modules, imports, classes, inheritance and dynamic dispatch out of scope.

## 0.6 — Completion

Status: Implemented.

Records, enums, field reads and field assignments are implemented and covered by conformance
tests. Record and enum declarations emit named C typedefs, enumerators emit named C constants, and
field assignment to sequence elements uses a checked runtime getter with a direct store. Repeated
index reads inside `if / else if` chains are emitted once into a local temporary.

## Emitted comments

Status: Implemented.

Line and block comments are preserved in generated C for source traceability. They remain outside
parsing and do not affect semantics, symbol binding or code generation decisions. A comment is
emitted next to the source construct it documents: declarations, functions, record fields and enum
items. Consecutive `//` lines stay together as a block, block comments keep their internal line
breaks, and `--disable-comments` omits all source comments from generated C.

## 0.7 — Function types and comment control

Status: Locked.

- Allow record and enum names as function parameter and return types.
- Emit generated C typedefs for those parameter and return types.
- Allow record declarations initialized from a function call or another record value of the same
  type.
- Attach generated comments to the source construct they document instead of hoisting them into
  `main`.
- Add `--disable-comments` to the compiler driver.

## 0.7 — Completion

Status: Implemented.

Functions may now take and return user-defined types, and record variables may be initialized from
values of the same record type. Zero-argument functions emit `(void)`, and unused parameters emit
`(void)` guards. Source comments attach to declarations, functions, record fields and enum items,
with consecutive `//` lines emitted as one block.

## 0.8 — Emitted C symbol roles

Status: Locked.

- Emit local source symbols with a leading `_`.
- Emit function parameters with their source names and no prefix.
- Emit record members with a trailing `_`.
- Keep user functions with `_u_`, compiler temporaries with `_zlang_`, generated types with
  `zlang_` and macros with `ZLANG_`.

## 0.8 — Completion

Status: Implemented.

Emitted C now makes the role of each source-derived name visible: `_x` is a local, `x` is a
parameter, `x_` is a record member, `_u_f` is a user function and `_zlang_*` is a compiler
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
- Keep references parameter-only: no storage in records or sequences and no returns.

## 0.12 — Completion

Status: Implemented.

Parameters now declare their binding explicitly. `clone` mutates only a local copy, while
`reference` mutates caller-owned storage through a compiler-managed pointer. The tour demonstrates
both, and reference arguments are checked as lvalues by semantic analysis.

## 0.13 — Reference intent warning

Status: Locked.

- Warn when `const reference` is used for a small value type.
- Warn when a mutable `reference` to a small value type is never mutated.
- Do not warn for mutated references or references to records.
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
  C type. A program using only 1D `integer` arrays gets only the 1D `int32_t` helpers.
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
  array integer grid = {
      {1, 2, 3},
      {4, 5, 6}
  };
  ```
  The initializer shape contributes rank and extents to inference.
- Define multidimensional iteration. `foreach (row, col in grid)` visits every index tuple in
  row-major order; the index variables are unsigned and bounds are compile-time known from the array
  shape.
- Allow array fields in records. A record field may be `array[A][B] T`, emitted as an embedded
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
- Arrays work as record fields without losing bounds checks.
- `const` arrays reject writes and whole-array copies require identical shape.

The implementation supports nested array initializers, multidimensional `foreach` in row-major
order, arrays as record fields with bounds-checked access, `const` arrays, whole-array copy for
identical shapes, and one generated C function per inferred array shape.

## 0.18.1 — Completion

Patch release after the 0.18 array model. Generated C now derives array extents from the accessed
symbol instead of the active function specialization, array fields use literal extents in typed
helper calls, string capacity macros are emitted without arrays, inferred array function calls
resolve their return type without an emitter symbol, scalar `reference` parameters are read and
written through a dereference, `foreach` iterables count as reads, dynamic `integer` indices are
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

## 0.21 — Design note

Zlang 0.21 adds user modules and an explicit program entry point. A compilation still produces one
C file with one C `main`; modules are assembled into that single translation unit rather than linked
as separate programs.

Every top-level declaration in a module is public. No `export` keyword is needed. Importing a
declaration binds a reference to the canonical entity: functions, variables, records and enums are
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
function main(integer argc, arguments argv): integer {
    return 0;
}
```

`main` is an ABI-shaped declaration, not an ordinary user function. Parameter qualifiers are omitted
because the binding modes are fixed by the signature: `argc` is an implicitly cloned `integer`
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
