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

Output uses `write` from `@std/io`. There is no built-in `print`. `read` and `read_line` take a mutable bounded string and report byte counts. Standard stateful functions share one internal runtime context; Zlang source never exposes it.

## Native result parity

`result_type<T, status_code>` applications map to canonical C layouts in `<zlang/standard/result/result.h>`. The compiler registry, the public header and the documentation must agree for every native function. A change to one of the three requires a review of the other two.

Native results introduce no allocation, formatting or reflection. Existing standard functions keep their current signatures; none returns a native result.
