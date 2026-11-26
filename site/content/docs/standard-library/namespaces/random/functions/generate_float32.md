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

Both bounds are values. The generator changes the injected runtime context and does not allocate.

## Errors and traps

`from > to` or a non-finite bound traps with `ZLANG_TRAP_INVALID_ARGUMENT` (code 4). A non-finite computed range traps with the same code. No status value reaches Zlang, and generated C terminates through the runtime trap sink.

## State and concurrency

The registry marks `generate_float32` as stateful and opaque. Each call advances one shared runtime context; two calls with the same bounds are legal and advance that context in order. The helper advances the injected pseudo-random state; it does not read an OS entropy source. The emitter passes the context as a hidden argument to generated functions that call the generator, including a Zlang-declared `main`. No mutable global holds the state. The API gives no thread-safety guarantee for sharing one context.

## Generated C

`import { generate_float32 } from "@std/random";` activates `ZLANG_STD_RANDOM_GENERATE_FLOAT32`:

```c
#define ZLANG_STD_RANDOM_GENERATE_FLOAT32
```

The emitted preamble also includes `<zlang/runtime/runtime.h>`. That header includes `zlang/standard/random/state.h` and `zlang/standard/random/generate_float32.h`. The header defines `zlang_std_random_generate_float32(zlang_runtime_context_t *context, float from, float to)` as a `ZLANG_STATIC_INLINE` wrapper; the injected context comes first.

Inside the emitted `app_fn_main`, this excerpt shows the local declaration and the call:

```c
int32_t app_fn_main(zlang_runtime_context_t *zlang_context) {
    float app_local_value = zlang_std_random_generate_float32(zlang_context, 0.0f, 1.0f);
```

`app_fn_main` is the emitted C function for the Zlang `main`. `app_local_value` is the emitted name for the Zlang local `value`. `zlang_context` is the hidden parameter that carries the random state into that function. The generated entry point creates the state and passes it:

```c
int main(void) {
    zlang_runtime_context_t _zlang_context = { 0x6D2B79F5U };
    return (int)app_fn_main(&_zlang_context);
}
```

`_zlang_context` is the local state object in the emitted `main`, and `zlang_runtime_context_t` is its runtime type.


## Example

```zlang
import { write } from "@std/io";
import { generate_float32 } from "@std/random";

function main(): int32 {
    float value = generate_float32(0.0f, 1.0f);
    write(value);
    write("\n");
    return 0;
}
```

## See also

- [generate_float64](/docs/standard-library/namespaces/random/functions/generate_float64)
- [generate_int32](/docs/standard-library/namespaces/random/functions/generate_int32)
- [Functions](/docs/standard-library/namespaces/random/functions)
- [Randomness overview](/docs/standard-library/namespaces/random)
- [Generated C](/docs/standard-library/generated-c)
