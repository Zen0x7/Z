# Zlang working specification notes

Current version: **0.39**

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
| `time()` | optional result capability | `uint64` nanoseconds |
| `dump()` | optional result capability | bounded canonical `string` view |

There is no implicit numeric conversion. Explicit conversions are `to_int32`, `to_uint32`,
`to_int64`, `to_uint64`, `to_float` and `to_double`. Arithmetic is checked for signed overflow;
signed minimum negation and zero-capacity failures trap. Floating arithmetic follows IEEE-754.

Strings are borrowed views. Variable concatenation requires `string[N]` storage and traps on
capacity overflow. Arrays have fixed capacity, bounds-checked index access, no logical length and
no append. Storage at or below `ZLANG_DEFAULT_STACK_LIMIT` is local; larger storage is `static`.

Structs, enums, interfaces, traits and `use` are compile-time constructs. A struct implements named
interfaces and uses traits. Inside a non-static method, `self` names the implicit receiver; fields are
also available without a receiver. Compound assignment accepts scalar identifiers, fields, `self.field`
and one-dimensional array elements with the same checked arithmetic as plain variables. Single
inheritance emits internal vtables and type tags; trait calls are direct calls. No dynamic allocation
is introduced.

## Generics

A declaration may list up to four type parameters; an application may pass up to four concrete type
arguments. Each application is monomorphized into a concrete C99 type or function. Generic structs
support flat instance methods; generic functions must be top-level and flat. Generic traits,
generic inheritance and variadic type packs remain out of scope.

## Native results and IO

Every registered standard library function returns `result_type<T, status_code>`. `T` is the
function's ideal value, `code` carries a `status_code`, and `what` is a bounded read-only message.
Functions with no recoverable failure return `SUCCESS`. Programmer and resource errors trap.

`result_type` is a stable envelope, not only an error wrapper. Optional capabilities are selected
at compile time. `time()` returns `uint64` elapsed nanoseconds; with `ZLANG_STD_RESULT_TIMING`
disabled it is the constant `0` and no clock read or timing state is emitted. `dump()` returns a
bounded canonical string containing `code`, `what`, the payload and elapsed time; with
`ZLANG_STD_RESULT_DUMP` disabled it returns the empty view. A user-defined `T` must implement
`dump(): string` before `result_type<T, status_code>.dump()` is valid.

Streams are borrowed, trivially copyable handles with a constant operations table. Stdio and
caller-owned memory streams implement the same contract as future sockets, pipes and devices.
Canonical IO forms are `read(source, destination, offset, count)` and
`write(destination, source, offset, count)`; convenience overloads keep implicit stdin/stdout.

Status constants are `STATUS_SUCCESS`, `STATUS_END_OF_INPUT`, `STATUS_IO_ERROR`,
`STATUS_INVALID_ARGUMENT` and `STATUS_CAPACITY`. C symbols and status enum constants remain
stable across the Zlang-facing lowercase type rename.

## 0.39 syntax and interfaces

Zlang 0.39 adds eight syntax forms: `label:` with `jump label;`, `switch`, value-producing
`match`, ternary `?:`, bitwise and shift operators, `do { ... } while (...)`, exclusive `uint32`
range literals `start..end`, and labeled `break`/`continue`.

It replaces field-only trait requirements with named interfaces:

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

Interfaces declare fields and method signatures. Traits declare required interfaces and provide
method bodies. Structs declare implemented interfaces and used traits. A struct satisfies an
interface only when all fields and signatures exist directly or through a used trait. A trait is
valid for a struct only when its required interfaces are implemented by that struct.

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
duplicate switch cases and match patterns,
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
- **0.38** — opt-in result capabilities: `time()` and `dump()` selected by compile-time flags.
- **0.39** — labels, jumps, `switch`, `match`, ternary `?:`, bitwise and shift operators, `do while`,
  range literals, labeled loop jumps, and named interface contracts.
- **0.40** — `auto` type inference, generic interface constraints, bounded views and explicit C interop.
- **0.41** — tagged variants with destructuring `match` and a final `default` arm.

## 0.40 type inference and interop foundations

Zlang 0.40 adds local type inference with `auto`. A declaration with an initializer may replace its
type with `auto`; the compiler resolves the initializer type before memory planning and emission.
A native generic argument may also be `auto`, as in
`result_type<auto, status_code> value = query();`. The emitted C always uses the concrete type. A
declaration without an initializer cannot use `auto`.

Generic parameters may declare interface constraints with
`function describe<T implements { Named, Other }>(clone T value): string`. The compiler checks the
constraints when a generic application is specialized.

A bounded borrowed array window uses `view[] T` and `to_view(source, offset, count)`. The view is a
small descriptor with a borrowed data pointer and a runtime length. It can be passed by `clone` to a
function declaring a `view[] T` parameter. It cannot be stored in a struct or returned from a
function. Accesses are bounds-checked.

`extern function` declares a C function with explicit Zlang parameter modes and a header:

```zlang
extern function abs(clone int32 value): int32 from "stdlib.h";
```

The call lowers directly to the named C function. A bare header name emits an angle-bracket
include; a header containing a path separator emits a quoted include. Project headers and C
sources are declared in `z.yaml` under `native:` and referenced with `@native/<alias>`; zc adds the
configured include dirs and links the sources of used aliases. Extern declarations have no Zlang
body, no variadics, no function pointers and no implicit conversion to `status_code`.

## 0.41 tagged variants

A variant declaration names a closed set of constructors with flat payload fields:

```zlang
variant Shape = Circle(int32 radius)
              | Rect(int32 width, int32 height);
```

`Shape` is a compile-time tagged union. A constructor call such as `Circle(7)` produces a `Shape`
value. `match (value)` can destructure variants with binding patterns:

```zlang
return match (shape) {
    Circle(radius) => radius * radius,
    Rect(width, height) => width * height,
    default => 0
};
```

`match` remains a value expression; arms produce values and contain no `return`. Payload fields are
plain scalar or enum values. The C99 layout uses an internal tag and a union. No heap, vtable or
runtime type lookup is emitted. Duplicate constructors and duplicate match patterns are diagnostics.

Older numbered status sections and duplicated design notes were folded into the site specification
and removed from this working file.
