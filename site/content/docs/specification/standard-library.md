---
title: Imports and the standard library
description: Canonical domains, activation macros and native result parity.
navigation:
  title: Standard library
---

# Imports and the standard library

## Import model

```zlang
import { write } from "@std/io";
```

Imports are explicit and name-based. A standard import uses a canonical `@std/<domain>` specifier. The compiler activates only the runtime headers and functions that the imported names need. Each function activates one macro in the generated C.

User modules use relative paths or `@alias/...` paths declared in `z.yaml`. Cyclic imports and paths that escape the module root are rejected. A legacy bare-domain import such as `"random"` is rejected.

## Domains

| Domain | Purpose |
| --- | --- |
| `@std/io` | bounded output and bounded input |
| `@std/math` | scalar numeric functions |
| `@std/random` | deterministic generators |
| `@std/strings` | bounded string views |
| `@std/program_options` | host argument views |

Output uses `write` from `@std/io`. There is no built-in `print`. `read` and `read_line` take a mutable bounded string and report transferred byte counts in a result. Standard stateful functions share one internal runtime context; Zlang source never exposes it.

## Native result parity

`result_type<T, status_code>` applications map to canonical C layouts in `<zlang/standard/result/result.h>`. The compiler registry, the public header and the documentation must agree for every native function. A change to one of the three requires a review of the other two.

Native results introduce no allocation, formatting or reflection. In Zlang 0.37 every registered function in `@std/io`, `@std/math`, `@std/program_options`, `@std/random` and `@std/strings` returns `result_type<T, status_code>`. `T` is the function's obvious ideal value, `code` carries the `status_code`, and `what` is a bounded read-only message view. Pure and total functions return `STATUS_SUCCESS` on valid input. Programmer and resource errors remain traps; a trapped function does not return.

`read` and `read_line` have two overloads. The convenience overload reads from implicit `STDIN` at offset `0` into a caller-owned destination with its declared capacity. The explicit overload takes a borrowed stream and a bounded offset/count window. Both return `result_type<int32, status_code>` whose `value` is the number of bytes transferred. `read` copies raw bytes and keeps a newline when it fits the window; `read_line` stops at a newline and drops the delimiter. Neither overload copies buffer bytes into new storage.

Parameter modes are unchanged: `clone` copies scalars or trivial descriptors (a cloned `string` copies its descriptor, not its bytes, and a cloned `stream` copies a borrowed handle), `reference` is a mutable caller-owned destination, and `const reference` is a borrowed view that is not mutated.
