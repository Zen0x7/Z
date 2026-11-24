# Zlang Roadmap

This file is append-only. Each version gets one section, appended at the bottom. Earlier sections are a historical record: do not rewrite them when plans change. If a decision is superseded, append a new entry that says so.

Status labels: `Implemented`, `In progress`, `Planned`, `Deferred`.

## 0.1 — Foundation and language baseline

Status: Implemented.

### Purpose

Zlang is a source-to-source language for bounded, deterministic applications. A Zlang program is compiled to valid, readable C that calls a small, audited C API (`zlang_*`). The generated C is an intermediate representation: it is not a runtime, and it is not code the application author is expected to maintain.

The promise is simple: **no magic**. `--emit-c` shows exactly which C will be compiled. Without that flag, `zlang-compile` goes directly from `.zlang` source to a native binary.

### Locked decisions

- **Transpilation, not interpretation.** There is no VM, no opcode dispatch, no runtime class or type registration, and no garbage collector.
- **Generated C is transport.** Every Zlang operation lowers to a call into the Zlang C API. Control flow (`if`, loops) is emitted as native C from compiler IR nodes. There is no callback-based `zlang_for_each` and no indirect dispatch.
- **Compile-time memory analysis.** Every size is known before the binary exists. Dynamic allocation is illegal. Unbounded values require an explicit limit; otherwise the compiler rejects the program and reports the source location of the declaration that needs it.
- **Realtime means memory discipline.** The goal is a bounded footprint and zero dynamic allocation, not hard realtime syscall or WCET guarantees.
- **Limit violations trap.** When a declared capacity is exceeded, the binary traps with a fixed code. It never grows a buffer and never builds an error message at runtime.
- **Diagnostics are first-class.** Syntax, type, and resource/allocation errors are separate categories and are reported in Zlang terms, not as C compiler output.
- **The C API is thin and auditable.** It is small, readable, and verified by the existing CI: sanitizers, coverage, Valgrind, and the multi-platform matrix.
- **Distribution is twofold.** A local `zlang-compile` binary compiles `.zlang` files, and a provided GitHub Action does the same in CI and publishes the resulting artifact.

### Non-goals

- A general-purpose interpreted language.
- Hidden heap growth, reference counting, or garbage collection.
- Syntax as the product. Syntax is deferred until the semantic model is settled.
- Runtime reflection or dynamic dispatch as a default mechanism.
- Hard realtime timing guarantees beyond predictable memory behavior.

### Architecture

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
   `zlang-compile` with `--emit-c` and direct native compilation. Packaging, release artifacts, and the Zlang GitHub Action were intentionally deferred to version 1.0.

8. **Hardening**
   Lexer, parser, and emitter fuzz harnesses. Trap paths covered in child processes. Diagnostics and supported limits documented in `SPEC.md`, including the Annex D conformance matrix.

### Definition of done

A bounded `.zlang` program compiles to native C-backed code; `--emit-c` shows the generated C; the binary performs zero dynamic allocation; an unbounded program is rejected with a source location; and the whole path runs through the existing CI matrix.

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
