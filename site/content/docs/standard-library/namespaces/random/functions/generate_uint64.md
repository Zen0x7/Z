---
title: generate_uint64
description: Generate a uint64 inside an inclusive unsigned 64-bit range.
domain: random
kind: function
status: implemented
stateful: true
opaque: true
canonical_symbols:
  - zlang_std_random_generate_uint64
activation_macros:
  - ZLANG_STD_RANDOM_GENERATE_UINT64
---

# generate_uint64

## Summary

`generate_uint64` returns one unsigned 64-bit value between two inclusive bounds.

## Import

```zlang
import { generate_uint64 } from "@std/random";
```

## Signature

```zlang
function generate_uint64(clone uint64 from, clone uint64 to): uint64
```

Both parameters are passed by value.

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `from` | `uint64` | clone | Lower bound of the inclusive range |
| `to` | `uint64` | clone | Upper bound of the inclusive range |

`from` must not be greater than `to`. The full unsigned 64-bit range is allowed.

## Result

Returns one `uint64` in `[from, to]`. Both bounds are reachable. Do not expect a specific value or sequence.

## Bounds and ownership

Both bounds are values. No caller-owned storage is read or written, and the runtime does not allocate.

## Errors and traps

`from > to` traps with `ZLANG_TRAP_INVALID_ARGUMENT` (code 4). No status value reaches Zlang, and generated C terminates through the runtime trap sink.

## State and concurrency

The registry marks `generate_uint64` as stateful and opaque. Each call advances one shared runtime context; two calls with the same bounds are legal and advance that context in order. The emitter passes the context as a hidden argument to generated functions that call the generator, including a Zlang-declared `main`. No mutable global holds the state. The API gives no thread-safety guarantee for sharing one context.

## Generated C

The import activates the macro before the runtime include:

```c
#define ZLANG_STD_RANDOM_GENERATE_UINT64
#include <zlang/runtime/runtime.h>
```

`runtime.h` then includes the shared random state header and `<zlang/standard/random/generate_uint64.h>`. The emitter calls `zlang_std_random_generate_uint64` with the context first. Call shape from the emitter, using the example arguments:

```c
uint64_t app_local_value = zlang_std_random_generate_uint64(zlang_context, 1ULL, 6ULL);
```

The context pointer is `zlang_context` inside a generated function and `&_zlang_context` in the generated `main` wrapper.

## Example

```zlang
import { write } from "@std/io";
import { generate_uint64 } from "@std/random";

function main(): integer {
    uint64 value = generate_uint64(1u64, 6u64);
    write(value);
    write("\n");
    return 0;
}
```

## See also

- [Functions](/docs/standard-library/namespaces/random/functions)
- [Randomness overview](/docs/standard-library/namespaces/random)
- [Generated C](/docs/standard-library/generated-c)
