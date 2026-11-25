---
title: Emitted C
description: Type mapping, construct mapping and generated names.
navigation:
  title: Emitted C
---

# Emitted C

Every generated C file includes the runtime headers. Imports add activation macros before the includes; the Fibonacci golden starts with:

```c
#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_I32

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>
```

## Type mapping

| Zlang | Emitted C |
| --- | --- |
| `integer` | `int32_t` |
| `unsigned` | `uint32_t` |
| `int64` | `int64_t` |
| `uint64` | `uint64_t` |
| `float` | `float` |
| `double` | `double` |
| `boolean` | `bool` |
| `string` | `zlang_string_t` |
| `array[N] T name` | generated typedef with a capacity macro |

## Construct mapping

- Arithmetic calls checked helpers.
- Logical operators use native C short-circuit behavior.
- `foreach` becomes a native `for` loop.
- Indexed access uses bounds checks.
- `write` becomes the selected `@std/io` runtime call.
- Errors use fixed numeric codes.

## Provenance

| Prefix | Meaning |
| --- | --- |
| `app_*` | translation of user source |
| `zlang_gen_*` | compiler support derived from source |
| `zlang_std_*` | standard library function |
| `zlang_*` | public runtime API |
| `ZLANG_*` | constants and macros |

## Example

The current Fibonacci golden materializes the two reads before adding them:

```c
        int32_t zlang_gen_read_3 = app_local_fib.data_[(uint32_t)zlang_sub_i32(app_local_index, 1)];
        int32_t zlang_gen_read_4 = app_local_fib.data_[(uint32_t)zlang_sub_i32(app_local_index, 2)];
        app_local_fib.data_[(uint32_t)app_local_index] = zlang_add_i32(zlang_gen_read_3, zlang_gen_read_4);
```

The full generated file is committed next to the example.
