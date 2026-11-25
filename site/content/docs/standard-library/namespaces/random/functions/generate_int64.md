---
title: generate_int64
description: Generate an int64 inside an inclusive signed 64-bit range.
domain: random
kind: function
status: implemented
stateful: true
opaque: true
canonical_symbols:
  - zlang_std_random_generate_int64
activation_macros:
  - ZLANG_STD_RANDOM_GENERATE_INT64
---

# generate_int64

## Summary

`generate_int64` returns one signed 64-bit value between two inclusive bounds.

## Import

```zlang
import { generate_int64 } from "@std/random";
```

## Signature

```zlang
function generate_int64(clone int64 from, clone int64 to): int64
```

Both parameters are passed by value.

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `from` | `int64` | clone | Lower bound of the inclusive range |
| `to` | `int64` | clone | Upper bound of the inclusive range |

`from` must not be greater than `to`. The full signed 64-bit range is allowed.

## Result

Returns one `int64` in `[from, to]`. Both bounds are reachable. Do not expect a specific value or sequence.

## Bounds and ownership

Both bounds are values. No caller-owned storage is read or written, and the runtime does not allocate.

## Errors and traps

`from > to` traps with `ZLANG_TRAP_INVALID_ARGUMENT` (code 4). No status value reaches Zlang, and generated C terminates through the runtime trap sink.

## State and concurrency

The registry marks `generate_int64` as stateful and opaque. Each call advances one shared runtime context; two calls with the same bounds are legal and advance that context in order. The helper advances the injected pseudo-random state; it does not read an OS entropy source. The emitter passes the context as a hidden argument to generated functions that call the generator, including a Zlang-declared `main`. No mutable global holds the state. The API gives no thread-safety guarantee for sharing one context.

## Generated C

The import activates the macro before the runtime include:

```c
#define ZLANG_STD_RANDOM_GENERATE_INT64
#include <zlang/runtime/runtime.h>
```

`runtime.h` then includes the shared random state header and `<zlang/standard/random/generate_int64.h>`. The C helper takes the context first:

```c
int64_t zlang_std_random_generate_int64(zlang_runtime_context_t *context, int64_t from, int64_t to);
```

In the example's generated `app_fn_main`, the call is:

```c
int64_t app_local_value = zlang_std_random_generate_int64(zlang_context, 1LL, 6LL);
```

Zlang functions receive the hidden context as `zlang_context`. Generated `main` creates `_zlang_context` and passes `&_zlang_context`.

## Example

```zlang
import { write } from "@std/io";
import { generate_int64 } from "@std/random";

function main(): integer {
    int64 value = generate_int64(1i64, 6i64);
    write(value);
    write("\n");
    return 0;
}
```

## See also

- [generate_uint64](/docs/standard-library/namespaces/random/functions/generate_uint64)
- [generate_int32](/docs/standard-library/namespaces/random/functions/generate_int32)
- [Functions](/docs/standard-library/namespaces/random/functions)
- [Randomness overview](/docs/standard-library/namespaces/random)
- [Generated C](/docs/standard-library/generated-c)
