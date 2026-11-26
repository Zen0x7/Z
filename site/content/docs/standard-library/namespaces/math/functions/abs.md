---
title: abs
description: Return the magnitude of a numeric value, trapping on signed minimum values.
domain: math
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_math_abs_i32
  - zlang_std_math_abs_u32
  - zlang_std_math_abs_i64
  - zlang_std_math_abs_u64
  - zlang_std_math_abs_f32
  - zlang_std_math_abs_f64
activation_macros:
  - ZLANG_STD_MATH_ABS_I32
  - ZLANG_STD_MATH_ABS_U32
  - ZLANG_STD_MATH_ABS_I64
  - ZLANG_STD_MATH_ABS_U64
  - ZLANG_STD_MATH_ABS_F32
  - ZLANG_STD_MATH_ABS_F64
---

# abs

## Summary

`abs` returns the magnitude of one scalar value in `result_type<T, status_code>`, with `T` matching the argument type.

## Import

```zlang
import { abs } from "@std/math";
```

## Signature

```zlang
function abs(clone int32 value): result_type<int32, status_code>
```

| Registry signature | C symbol | Activation macro |
| --- | --- | --- |
| `function abs(clone int32 value): result_type<int32, status_code>` | `zlang_std_math_abs_i32` | `ZLANG_STD_MATH_ABS_I32` |
| `function abs(clone uint32 value): result_type<uint32, status_code>` | `zlang_std_math_abs_u32` | `ZLANG_STD_MATH_ABS_U32` |
| `function abs(clone int64 value): result_type<int64, status_code>` | `zlang_std_math_abs_i64` | `ZLANG_STD_MATH_ABS_I64` |
| `function abs(clone uint64 value): result_type<uint64, status_code>` | `zlang_std_math_abs_u64` | `ZLANG_STD_MATH_ABS_U64` |
| `function abs(clone float value): result_type<float, status_code>` | `zlang_std_math_abs_f32` | `ZLANG_STD_MATH_ABS_F32` |
| `function abs(clone double value): result_type<double, status_code>` | `zlang_std_math_abs_f64` | `ZLANG_STD_MATH_ABS_F64` |

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `value` | one of `int32`, `uint32`, `int64`, `uint64`, `float`, `double` | clone | Value whose magnitude is returned |

`abs` takes one scalar argument. No capacity is required.

## Result

`value`/`get()` holds the magnitude, with `T` in `result_type<T, status_code>` matching the argument type. `code` is a `status_code` that is `STATUS_SUCCESS` when the helper completes, and `message` is a bounded read-only view of the fixed status message. `uint32` values are unchanged. Signed values are negated when negative. For the `float` and `double` overloads, the helper uses an ordered comparison against zero: NaN appears as NaN, negative zero is unchanged, and an infinity keeps its magnitude. The C helper returns the canonical `zlang_std_result_*_status_t` struct for the selected overload, not a bare numeric type.

## Bounds and ownership

`abs` takes scalars by clone. It does not allocate, mutate caller storage, or read through a pointer. No declared capacity is involved.

## Errors and traps

`zlang_std_math_abs_i32` traps with `ZLANG_TRAP_OVERFLOW` (code `1`) when `value` is `INT32_MIN`. `zlang_std_math_abs_i64` traps with the same code when `value` is `INT64_MIN`. This check prevents negating a signed minimum value, which has no representable positive result. The trap is not a recoverable status; `code` is `STATUS_SUCCESS` only when the helper completes.

The uint32, `float` and `double` helpers contain no trap. Floating-point helpers do not reject NaN or infinity. Floating arithmetic follows IEEE-754, so division by zero in the surrounding program produces infinity or NaN instead of a Zlang trap.

## State and concurrency

The registry marks `abs` as not stateful and opaque. The emitter passes the argument directly and adds no hidden context parameter. The standard headers own no mutable globals. The same argument always produces the same result, and concurrent calls share no mutable state.

## Generated C

Importing `abs` activates `ZLANG_STD_MATH_ABS_I32` from the first registry entry with that name. The call activates the macro for the selected overload. The Example produces this excerpt:

```c
#define ZLANG_STD_MATH_ABS_I32
...
zlang_std_result_i32_status_t app_local_result = zlang_std_math_abs_i32(zlang_neg_i32(7));
```

`zlang_std_result_i32_status_t` is the C layout for the `result_type<int32, status_code>` selected by `abs(-7)`.

The header guards do not match the registry macros. `math.h` defines all `abs` helpers, including the `uint32` ones, only when `ZLANG_STD_MATH_ABS_ACTIVE` is defined. That guard lists `ZLANG_STD_MATH_ABS_I32`, `ZLANG_STD_MATH_ABS_I64`, `ZLANG_STD_MATH_ABS_F32` and `ZLANG_STD_MATH_ABS_F64`; it omits `ZLANG_STD_MATH_ABS_U32` and `ZLANG_STD_MATH_ABS_U64`. Generated C imports activate `ZLANG_STD_MATH_ABS_I32` first, so the `uint32` helpers are present in generated programs. A hand-written translation unit that defines only a `uint32` `abs` macro does not declare them.

## Example

```zlang
import { write } from "@std/io";
import { abs } from "@std/math";

function main(): int32 {
    result_type<int32, status_code> result = abs(-7);
    write(result.get());
    write("\n");
    return 0;
}
```

## See also

- [clamp](/docs/standard-library/namespaces/math/functions/clamp)
- [min](/docs/standard-library/namespaces/math/functions/min)
- [max](/docs/standard-library/namespaces/math/functions/max)
- [Catalog](/docs/standard-library/namespaces/math/functions)
