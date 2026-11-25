---
title: clamp
description: Confine a numeric value to an inclusive range.
domain: math
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_math_clamp_i32
  - zlang_std_math_clamp_u32
  - zlang_std_math_clamp_i64
  - zlang_std_math_clamp_u64
  - zlang_std_math_clamp_f32
  - zlang_std_math_clamp_f64
activation_macros:
  - ZLANG_STD_MATH_CLAMP_I32
  - ZLANG_STD_MATH_CLAMP_U32
  - ZLANG_STD_MATH_CLAMP_I64
  - ZLANG_STD_MATH_CLAMP_U64
  - ZLANG_STD_MATH_CLAMP_F32
  - ZLANG_STD_MATH_CLAMP_F64
---

# clamp

## Summary

`clamp` returns a value confined to an inclusive range.

## Import

```zlang
import { clamp } from "@std/math";
```

## Signature

```zlang
function clamp(clone integer value, clone integer low, clone integer high): integer
```

| Registry signature | C symbol | Activation macro |
| --- | --- | --- |
| `function clamp(clone integer value, clone integer low, clone integer high): integer` | `zlang_std_math_clamp_i32` | `ZLANG_STD_MATH_CLAMP_I32` |
| `function clamp(clone unsigned value, clone unsigned low, clone unsigned high): unsigned` | `zlang_std_math_clamp_u32` | `ZLANG_STD_MATH_CLAMP_U32` |
| `function clamp(clone int64 value, clone int64 low, clone int64 high): int64` | `zlang_std_math_clamp_i64` | `ZLANG_STD_MATH_CLAMP_I64` |
| `function clamp(clone uint64 value, clone uint64 low, clone uint64 high): uint64` | `zlang_std_math_clamp_u64` | `ZLANG_STD_MATH_CLAMP_U64` |
| `function clamp(clone float value, clone float low, clone float high): float` | `zlang_std_math_clamp_f32` | `ZLANG_STD_MATH_CLAMP_F32` |
| `function clamp(clone double value, clone double low, clone double high): double` | `zlang_std_math_clamp_f64` | `ZLANG_STD_MATH_CLAMP_F64` |

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `value` | one of `integer`, `unsigned`, `int64`, `uint64`, `float`, `double` | clone | Value to confine |
| `low` | same type as `value` | clone | Inclusive lower bound |
| `high` | same type as `value` | clone | Inclusive upper bound |

All three arguments must have the same type. No capacity is required.

## Result

Returns `value` when it lies between `low` and `high`. Returns `low` when `value` is smaller, and `high` when `value` is larger. The return type matches the argument type and the registry return type.

For the `float` and `double` overloads, the bounds are finite and ordered. Ordered comparisons decide the result; signed zeros compare equal.

## Bounds and ownership

`clamp` takes three scalars by clone. It does not allocate, mutate caller storage, or read through a pointer. No declared capacity is involved.

## Errors and traps

Every overload traps with `ZLANG_TRAP_INVALID_ARGUMENT` (code `4`) when `low > high`. The `float` and `double` overloads also trap with the same code when any of the three arguments is non-finite, including NaN and either infinity. The integer overloads have no other trap.

## State and concurrency

The registry marks `clamp` as not stateful and opaque. The emitter passes the three arguments directly and adds no hidden context parameter. The standard headers own no mutable globals, so concurrent calls share no mutable state.

## Generated C

Importing `clamp` activates `ZLANG_STD_MATH_CLAMP_I32` from the first registry entry with that name. The call activates the macro for the selected overload. The example below produces this excerpt:

```c
#define ZLANG_STD_MATH_CLAMP_I32
...
zlang_std_math_clamp_i32(7, 1, 5);
```

## Example

```zlang
import { write } from "@std/io";
import { clamp } from "@std/math";

function main(): integer {
    write(clamp(7, 1, 5));
    write("\n");
    return 0;
}
```

## See also

- [abs](/docs/standard-library/namespaces/math/functions/abs)
- [min](/docs/standard-library/namespaces/math/functions/min)
- [max](/docs/standard-library/namespaces/math/functions/max)
- [Catalog](/docs/standard-library/namespaces/math/functions)
