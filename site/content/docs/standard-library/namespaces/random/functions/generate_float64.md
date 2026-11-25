---
title: generate_float64
description: Generate a double inside a floating-point range.
domain: random
kind: function
status: implemented
stateful: true
opaque: true
canonical_symbols:
  - zlang_std_random_generate_float64
activation_macros:
  - ZLANG_STD_RANDOM_GENERATE_FLOAT64
---

# generate_float64

## Summary

`generate_float64` returns one 64-bit floating-point value from a caller-specified range.

## Import

```zlang
import { generate_float64 } from "@std/random";
```

## Signature

```zlang
function generate_float64(clone double from, clone double to): double
```

Both parameters are passed by value.

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `from` | `double` | clone | Lower bound of the range |
| `to` | `double` | clone | Upper bound of the range |

`from` must not be greater than `to`, and both bounds must be finite.

## Result

Returns one `double` from the range `[from, to)` before floating-point rounding. The implementation takes one runtime draw as a fraction below 1 and computes `from + (to - from) * fraction`. Do not expect a specific value or sequence.

## Bounds and ownership

Both bounds are values. No caller-owned storage is read or written, and the runtime does not allocate.

## Errors and traps

`from > to` or a non-finite bound traps with `ZLANG_TRAP_INVALID_ARGUMENT` (code 4). A non-finite computed range traps with the same code. No status value reaches Zlang, and generated C terminates through the runtime trap sink.

## State and concurrency

The registry marks `generate_float64` as stateful and opaque. Each call advances one shared runtime context; two calls with the same bounds are legal and advance that context in order. The helper advances the injected pseudo-random state; it does not read an OS entropy source. The emitter passes the context as a hidden argument to generated functions that call the generator, including a Zlang-declared `main`. No mutable global holds the state. The API gives no thread-safety guarantee for sharing one context.

## Generated C

The import activates the macro before the runtime include:

```c
#define ZLANG_STD_RANDOM_GENERATE_FLOAT64
#include <zlang/runtime/runtime.h>
```

`runtime.h` then includes the shared random state header and `<zlang/standard/random/generate_float64.h>`. The C helper takes the context first:

```c
double zlang_std_random_generate_float64(zlang_runtime_context_t *context, double from, double to);
```

In the example's generated `app_fn_main`, the call is:

```c
double app_local_value = zlang_std_random_generate_float64(zlang_context, 0.0, 1.0);
```

Zlang functions receive the hidden context as `zlang_context`. Generated `main` creates `_zlang_context` and passes `&_zlang_context`.

## Example

```zlang
import { write } from "@std/io";
import { generate_float64 } from "@std/random";

function main(): integer {
    double value = generate_float64(0.0, 1.0);
    write(value);
    write("\n");
    return 0;
}
```

## See also

- [generate_float32](/docs/standard-library/namespaces/random/functions/generate_float32)
- [generate_int64](/docs/standard-library/namespaces/random/functions/generate_int64)
- [Functions](/docs/standard-library/namespaces/random/functions)
- [Randomness overview](/docs/standard-library/namespaces/random)
- [Generated C](/docs/standard-library/generated-c)
