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

`generate_int64` reports one signed 64-bit value between two inclusive bounds in `result_type<int64, status_code>`.

## Import

```zlang
import { generate_int64 } from "@std/random";
```

## Signature

```zlang
function generate_int64(clone int64 from, clone int64 to): result_type<int64, status_code>
```

Both parameters are passed by value.

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `from` | `int64` | clone | Lower bound of the inclusive range |
| `to` | `int64` | clone | Upper bound of the inclusive range |

`from` must not be greater than `to`. The full signed 64-bit range is allowed.

## Result

`value`/`get()` holds one `int64` in `[from, to]`. Both bounds are reachable. `code` is a `status_code` set to `STATUS_SUCCESS` for valid bounds, and `what` is a bounded read-only view of the fixed status message. The C helper returns `zlang_std_result_i64_status_t`, not a bare `int64_t`. Do not expect a specific value or sequence.

## Bounds and ownership

Both bounds are values. The generator changes the injected runtime context and does not allocate.

## Errors and traps

`from > to` traps with `ZLANG_TRAP_INVALID_ARGUMENT` (code 4), and generated C terminates through the runtime trap sink. A trapped call returns no result. There is no recoverable error status: every returned `code` is `STATUS_SUCCESS`.

## State and concurrency

The registry marks `generate_int64` as stateful and opaque. Each call advances one shared runtime context; two calls with the same bounds are legal and advance that context in order. The helper advances the injected pseudo-random state; it does not read an OS entropy source. The emitter passes the context as a hidden argument to generated functions that call the generator, including a Zlang-declared `main`. No mutable global holds the state. The API gives no thread-safety guarantee for sharing one context.

## Generated C

`import { generate_int64 } from "@std/random";` activates `ZLANG_STD_RANDOM_GENERATE_INT64`:

```c
#define ZLANG_STD_RANDOM_GENERATE_INT64
```

The emitted preamble also includes `<zlang/runtime/runtime.h>`. That header includes `zlang/standard/random/state.h` and `zlang/standard/random/generate_int64.h`. The header defines `zlang_std_random_generate_int64(zlang_runtime_context_t *context, int64_t from, int64_t to)` as a `ZLANG_STATIC_INLINE` wrapper returning `zlang_std_result_i64_status_t`; the injected context comes first.

Inside the emitted `app_fn_main`, this excerpt shows the local declaration and the call:

```c
int32_t app_fn_main(zlang_runtime_context_t *zlang_context) {
    zlang_std_result_i64_status_t app_local_result =
        zlang_std_random_generate_int64(zlang_context, 1LL, 6LL);
```

`app_fn_main` is the emitted C function for the Zlang `main`. `app_local_result` is the emitted name for the Zlang local `result`; its `value_` field holds the generated value, `code_` holds the status, and `what_` is the fixed message view. `zlang_context` is the hidden parameter that carries the random state into that function. The generated entry point creates the state and passes it:

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
import { generate_int64 } from "@std/random";

function main(): int32 {
    result_type<int64, status_code> result = generate_int64(1i64, 6i64);
    write(result.get());
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
