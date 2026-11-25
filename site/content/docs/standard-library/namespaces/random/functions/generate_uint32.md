---
title: generate_uint32
description: Generate an unsigned value inside an inclusive 32-bit range.
domain: random
kind: function
status: implemented
stateful: true
opaque: true
canonical_symbols:
  - zlang_std_random_generate_uint32
activation_macros:
  - ZLANG_STD_RANDOM_GENERATE_UINT32
---

# generate_uint32

## Summary

`generate_uint32` returns one unsigned 32-bit value between two inclusive bounds.

## Import

```zlang
import { generate_uint32 } from "@std/random";
```

## Signature

```zlang
function generate_uint32(clone unsigned from, clone unsigned to): unsigned
```

Both parameters are passed by value.

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `from` | `unsigned` | clone | Lower bound of the inclusive range |
| `to` | `unsigned` | clone | Upper bound of the inclusive range |

`from` must not be greater than `to`. The full `unsigned` range is allowed.

## Result

Returns one `unsigned` in `[from, to]`. Both bounds are reachable. Do not expect a specific value or sequence.

## Bounds and ownership

Both bounds are values. No caller-owned storage is read or written, and the runtime does not allocate.

## Errors and traps

`from > to` traps with `ZLANG_TRAP_INVALID_ARGUMENT` (code 4). No status value reaches Zlang, and generated C terminates through the runtime trap sink.

## State and concurrency

The registry marks `generate_uint32` as stateful and opaque. Each call advances one shared runtime context; two calls with the same bounds are legal and advance that context in order. The helper advances the injected pseudo-random state; it does not read an OS entropy source. The emitter passes the context as a hidden argument to generated functions that call the generator, including a Zlang-declared `main`. No mutable global holds the state. The API gives no thread-safety guarantee for sharing one context.

## Generated C

The import activates the macro before the runtime include:

```c
#define ZLANG_STD_RANDOM_GENERATE_UINT32
#include <zlang/runtime/runtime.h>
```

`runtime.h` then includes the shared random state header and `<zlang/standard/random/generate_uint32.h>`. The C helper takes the context first:

```c
uint32_t zlang_std_random_generate_uint32(zlang_runtime_context_t *context, uint32_t from, uint32_t to);
```

In the example's generated `app_fn_main`, the call is:

```c
uint32_t app_local_value = zlang_std_random_generate_uint32(zlang_context, 1U, 6U);
```

Zlang functions receive the hidden context as `zlang_context`. Generated `main` creates `_zlang_context` and passes `&_zlang_context`.

## Example

```zlang
import { write } from "@std/io";
import { generate_uint32 } from "@std/random";

function main(): integer {
    unsigned value = generate_uint32(1u, 6u);
    write(value);
    write("\n");
    return 0;
}
```

## See also

- [generate_int32](/docs/standard-library/namespaces/random/functions/generate_int32)
- [generate_uint64](/docs/standard-library/namespaces/random/functions/generate_uint64)
- [Functions](/docs/standard-library/namespaces/random/functions)
- [Randomness overview](/docs/standard-library/namespaces/random)
- [Generated C](/docs/standard-library/generated-c)
