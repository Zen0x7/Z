---
title: generate_float32
description: Generate a float inside a signed 32-bit floating-point range.
domain: random
kind: function
status: implemented
stateful: true
opaque: true
canonical_symbols:
  - zlang_std_random_generate_float32
activation_macros:
  - ZLANG_STD_RANDOM_GENERATE_FLOAT32
---

# generate_float32

## Summary

`generate_float32` returns one 32-bit floating-point value from a caller-specified range.

## Import

```zlang
import { generate_float32 } from "@std/random";
```

## Signature

```zlang
function generate_float32(clone float from, clone float to): float
```

Both parameters are passed by value.

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `from` | `float` | clone | Lower bound of the range |
| `to` | `float` | clone | Upper bound of the range |

`from` must not be greater than `to`, and both bounds must be finite.

## Result

Returns one `float` from the range `[from, to)` before floating-point rounding. The implementation takes one runtime draw as a fraction below 1, computes the range in C `double`, and converts the result to `float`. Do not expect a specific value or sequence.

## Bounds and ownership

Both bounds are values. No caller-owned storage is read or written, and the runtime does not allocate.

## Errors and traps

`from > to` or a non-finite bound traps with `ZLANG_TRAP_INVALID_ARGUMENT` (code 4). A non-finite computed range traps with the same code. No status value reaches Zlang, and generated C terminates through the runtime trap sink.

## State and concurrency

The registry marks `generate_float32` as stateful and opaque. Each call advances one shared runtime context; two calls with the same bounds are legal and advance that context in order. The emitter passes the context as a hidden argument to generated functions that call the generator, including a Zlang-declared `main`. No mutable global holds the state. The API gives no thread-safety guarantee for sharing one context.

## Generated C

The import activates the macro before the runtime include:

```c
#define ZLANG_STD_RANDOM_GENERATE_FLOAT32
#include <zlang/runtime/runtime.h>
```

`runtime.h` then includes the shared random state header and `<zlang/standard/random/generate_float32.h>`. The emitter calls `zlang_std_random_generate_float32` with the context first. Call shape from the emitter, using the example arguments:

```c
float app_local_value = zlang_std_random_generate_float32(zlang_context, 0.0f, 1.0f);
```

The context pointer is `zlang_context` inside a generated function and `&_zlang_context` in the generated `main` wrapper.

## Example

```zlang
import { write } from "@std/io";
import { generate_float32 } from "@std/random";

function main(): integer {
    float value = generate_float32(0.0f, 1.0f);
    write(value);
    write("\n");
    return 0;
}
```

## See also

- [Functions](/docs/standard-library/namespaces/random/functions)
- [Randomness overview](/docs/standard-library/namespaces/random)
- [Generated C](/docs/standard-library/generated-c)
