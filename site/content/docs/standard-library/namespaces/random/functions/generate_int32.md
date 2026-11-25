---
title: generate_int32
description: Generate an integer inside an inclusive signed 32-bit range.
domain: random
kind: function
status: implemented
stateful: true
opaque: true
canonical_symbols:
  - zlang_std_random_generate_int32
activation_macros:
  - ZLANG_STD_RANDOM_GENERATE_INT32
---

# generate_int32

## Summary

`generate_int32` returns one signed 32-bit value between two inclusive bounds.

## Import

```zlang
import { generate_int32 } from "@std/random";
```

## Signature

```zlang
function generate_int32(clone integer from, clone integer to): integer
```

Both parameters are passed by value.

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `from` | `integer` | clone | Lower bound of the inclusive range |
| `to` | `integer` | clone | Upper bound of the inclusive range |

`from` must not be greater than `to`. The full `integer` range is allowed.

## Result

Returns one `integer` in `[from, to]`. Both bounds are reachable. Do not expect a specific value or sequence.

## Bounds and ownership

Both bounds are values. No caller-owned storage is read or written, and the runtime does not allocate.

## Errors and traps

`from > to` traps with `ZLANG_TRAP_INVALID_ARGUMENT` (code 4). No status value reaches Zlang, and generated C terminates through the runtime trap sink.

## State and concurrency

The registry marks `generate_int32` as stateful and opaque. Each call advances one shared runtime context; two calls with the same bounds are legal and advance that context in order. The emitter passes the context as a hidden argument to generated functions that call the generator, including a Zlang-declared `main`. No mutable global holds the state. The API gives no thread-safety guarantee for sharing one context.

## Generated C

The import activates the macro before the runtime include:

```c
#define ZLANG_STD_RANDOM_GENERATE_INT32
#include <zlang/runtime/runtime.h>
```

`runtime.h` then includes the shared random state header and `<zlang/standard/random/generate_int32.h>`. The emitter calls `zlang_std_random_generate_int32` with the context first. The checked-in golden C for a top-level call is:

```c
int32_t app_local_value = zlang_std_random_generate_int32(&_zlang_context, 1, 1);
```

Inside a generated function, including a Zlang-declared `main`, the call uses `zlang_context` instead of `&_zlang_context`.

## Example

```zlang
import { write } from "@std/io";
import { generate_int32 } from "@std/random";

function main(): integer {
    integer value = generate_int32(1, 6);
    write(value);
    write("\n");
    return 0;
}
```

## See also

- [Functions](/docs/standard-library/namespaces/random/functions)
- [Randomness overview](/docs/standard-library/namespaces/random)
- [Generated C](/docs/standard-library/generated-c)
