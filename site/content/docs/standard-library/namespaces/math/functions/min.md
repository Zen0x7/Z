---
title: min
description: Select the lesser of two numeric values.
domain: math
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_math_min_i32
  - zlang_std_math_min_u32
  - zlang_std_math_min_i64
  - zlang_std_math_min_u64
  - zlang_std_math_min_f32
  - zlang_std_math_min_f64
activation_macros:
  - ZLANG_STD_MATH_MIN_I32
  - ZLANG_STD_MATH_MIN_U32
  - ZLANG_STD_MATH_MIN_I64
  - ZLANG_STD_MATH_MIN_U64
  - ZLANG_STD_MATH_MIN_F32
  - ZLANG_STD_MATH_MIN_F64
---

# min

## Summary

`min` selects the lesser of two scalar values and reports it in `result_type<T, status_code>`, with `T` matching the argument type.

## Import

```zlang
import { min } from "@std/math";
```

## Signature

```zlang
function min(clone int32 left, clone int32 right): result_type<int32, status_code>
```

| Registry signature | C symbol | Activation macro |
| --- | --- | --- |
| `function min(clone int32 left, clone int32 right): result_type<int32, status_code>` | `zlang_std_math_min_i32` | `ZLANG_STD_MATH_MIN_I32` |
| `function min(clone uint32 left, clone uint32 right): result_type<uint32, status_code>` | `zlang_std_math_min_u32` | `ZLANG_STD_MATH_MIN_U32` |
| `function min(clone int64 left, clone int64 right): result_type<int64, status_code>` | `zlang_std_math_min_i64` | `ZLANG_STD_MATH_MIN_I64` |
| `function min(clone uint64 left, clone uint64 right): result_type<uint64, status_code>` | `zlang_std_math_min_u64` | `ZLANG_STD_MATH_MIN_U64` |
| `function min(clone float left, clone float right): result_type<float, status_code>` | `zlang_std_math_min_f32` | `ZLANG_STD_MATH_MIN_F32` |
| `function min(clone double left, clone double right): result_type<double, status_code>` | `zlang_std_math_min_f64` | `ZLANG_STD_MATH_MIN_F64` |

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `left` | one of `int32`, `uint32`, `int64`, `uint64`, `float`, `double` | clone | First candidate |
| `right` | same type as `left` | clone | Second candidate |

Both arguments must have the same type. No capacity is required.

## Result

`value`/`get()` holds `left` when `left < right`, otherwise `right`. The result type is `result_type<T, status_code>` with `T` matching the argument type. `code` is a `status_code` that is always `STATUS_SUCCESS` because no overload traps, and `message` is a bounded read-only view of the fixed status message. The C helper returns the canonical `zlang_std_result_*_status_t` struct for the selected overload.

For `float` and `double`, the helper uses an ordered comparison. When `left` is NaN, the comparison is false and the result value is `right`. When `right` is NaN, the comparison is false and the NaN becomes the result value. Signed zeros compare equal, so the second argument becomes the result value.

## Bounds and ownership

`min` takes two scalars by clone. It does not allocate, mutate caller storage, or read through a pointer. No declared capacity is involved.

## Errors and traps

No overload traps, so no recoverable error status exists. The floating helpers accept NaN and infinities without a validity check, and IEEE-754 ordering decides the result value.

## State and concurrency

The registry marks `min` as not stateful and opaque. The emitter passes both arguments directly and adds no hidden context parameter. The standard headers own no mutable globals. The same two arguments always produce the same result, and repeated and concurrent calls share no mutable state.

## Generated C

Importing `min` activates `ZLANG_STD_MATH_MIN_I32` from the first registry entry with that name. The call activates the macro for the selected overload. The Example produces this excerpt:

```c
#define ZLANG_STD_MATH_MIN_I32
...
zlang_std_result_i32_status_t app_local_result = zlang_std_math_min_i32(2, 3);
```

`zlang_std_result_i32_status_t` is the C layout for the `result_type<int32, status_code>` selected by `min(2, 3)`.

## Example

```zlang
import { write } from "@std/io";
import { min } from "@std/math";

function main(): int32 {
    result_type<int32, status_code> result = min(2, 3);
    write(result.get());
    write("\n");
    return 0;
}
```

## See also

- [max](/docs/standard-library/namespaces/math/functions/max)
- [abs](/docs/standard-library/namespaces/math/functions/abs)
- [clamp](/docs/standard-library/namespaces/math/functions/clamp)
- [Catalog](/docs/standard-library/namespaces/math/functions)
