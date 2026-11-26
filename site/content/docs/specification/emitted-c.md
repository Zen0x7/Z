---
title: Translation to C
description: The normative C form for Zlang types, expressions, statements and runtime calls.
navigation:
  title: Translation to C
---

# Translation to C

This clause is normative. Each row states the exact C form for a Zlang construct. Whitespace may differ; tokens may not.

## Prologue

Every generated C file begins with:

```c
#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>
```

Imports add their activation macros before these includes.

## Type mapping

| Zlang | Emitted C |
| --- | --- |
| `int32` | `int32_t` |
| `uint32` | `uint32_t` |
| `int64` | `int64_t` |
| `uint64` | `uint64_t` |
| `float` | `float` |
| `double` | `double` |
| `boolean` | `bool` |
| `string` | `zlang_string_t` |
| `arguments` | `zlang_arguments_t` |
| `array[N] T` | generated typedef with a `ZLANG_*_CAPACITY` macro |
| `struct S` | generated C struct typedef |
| `enum E` | generated named constants |
| generic application | one concrete C99 type or function per application |

## Construct mapping

| Zlang | Emitted C |
| --- | --- |
| `int32 var = 10;` | `int32_t var = 10;` |
| `uint32 n = 2u;` | `uint32_t n = 2U;` |
| `boolean ready = true;` | `bool ready = true;` |
| `string s = "Hi";` | `zlang_string_t s = { "Hi", 2U };` |
| `a + b`, `a - b`, `a * b` | checked arithmetic helper, then the plain operator |
| `a += b` and other compound assignments | checked operation followed by assignment |
| `length(values)` | native capacity expression as `uint32_t` |
| `a == b` for strings | `zlang_string_equal(a, b)` |
| scalar comparisons and logic | native C operators with short-circuit behavior |
| `"a" + "b"` | folded string literal |
| `prefix + suffix` with capacity | `zlang_string_concat` into caller-owned storage |
| `write(s)`, `write(i)`, `write(u)`, `write(b)` | bounded standard write helper |
| `values[i]` | bounded typed getter or direct access with a proven index |
| `text[i]` | `zlang_string_byte(text, i)` |
| `values[i] = x` | checked index statement, then a direct store |
| `foreach`, `if`, `while`, `repeat`, `for` | native C `for`, `if`, `while` and `for` |
| method call | direct C call, or vtable slot call through a base receiver |
| trait call | direct C call into the using struct |
| `function f(...)` | C function with the `app_fn_` prefix |
| `return x;` | `return x;` |
| overflow, bounds, capacity or division failure | `zlang_trap(<ZLANG_TRAP_*>)` |

## Generated layout

Generated C appears in this order: activation macros, includes, generated value constants, generated capacity and extent macros, generated array typedefs, generated struct and enum types, generated string literals, function prototypes, user functions and `main`.

Local variables use the `app_local_` prefix. Parameters use `app_param_`. Struct members use a trailing `_`. User functions use `app_fn_`, generated types use `app_`, compiler temporaries use `zlang_gen_`, generated macros use `ZLANG_`, and runtime calls use `zlang_`.

A reference to an array element is resolved once per context. The first field access materializes a checked pointer, and later accesses to the same element reuse it until an index dependency changes or the scope ends. Repeated index reads across an `if` chain and its branches are materialized once. Reads that live only under short-circuit operators remain inline.

With `--tto`, the compiler folds constant expressions and literal array accesses, and proves ranges for canonical loops. Proven indices may become direct `data_[]` accesses. An unproven index keeps its bounds check. Optimization never changes evaluation order or trap timing.

## Runtime API

Generated code may call the `zlang_*` helpers declared in `<zlang/runtime/runtime.h>`. They cover checked arithmetic, bounded indexing, string bytes, bounded concatenation, standard output and traps. They contain no allocation, formatting or dynamic dispatch. Any additional `zlang_*` call requires an amendment to this specification.

Generated C must not contain:

- a function pointer table except the internal vtables required by inheritance;
- a closure or callback passed to a runtime helper;
- a string literal containing a source variable name;
- `malloc`, `calloc`, `realloc`, `free`, `strdup` or an equivalent service;
- a runtime type lookup by name.

## Worked example

Source:

```zlang
import { write } from "@std/io";

struct Pair<T, U> {
    T first;
    U second;
}

function identity<T>(clone T value): T {
    return value;
}

function main(): int32 {
    Pair<int32, boolean> pair;
    pair.first = 7;
    pair.second = true;

    int32 value = identity<int32>(pair.first);
    write(value);
    write(" ");
    write(pair.second);
    return 0;
}
```

The golden C in `examples/features/generics/main.c` declares one specialization of `Pair` and one specialization of `identity`, then calls `zlang_std_io_write_i32`, `zlang_std_io_write_string` and `zlang_std_io_write_bool` in source order. No generic value or runtime dispatch survives.
