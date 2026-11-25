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

`abs` returns the magnitude of one scalar value.

## Import

```zlang
import { abs } from "@std/math";
```

## Signature

```zlang
function abs(clone integer value): integer
```

| Registry signature | C symbol | Activation macro |
| --- | --- | --- |
| `function abs(clone integer value): integer` | `zlang_std_math_abs_i32` | `ZLANG_STD_MATH_ABS_I32` |
| `function abs(clone unsigned value): unsigned` | `zlang_std_math_abs_u32` | `ZLANG_STD_MATH_ABS_U32` |
| `function abs(clone int64 value): int64` | `zlang_std_math_abs_i64` | `ZLANG_STD_MATH_ABS_I64` |
| `function abs(clone uint64 value): uint64` | `zlang_std_math_abs_u64` | `ZLANG_STD_MATH_ABS_U64` |
| `function abs(clone float value): float` | `zlang_std_math_abs_f32` | `ZLANG_STD_MATH_ABS_F32` |
| `function abs(clone double value): double` | `zlang_std_math_abs_f64` | `ZLANG_STD_MATH_ABS_F64` |

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `value` | one of `integer`, `unsigned`, `int64`, `uint64`, `float`, `double` | clone | Value whose magnitude is returned |

`abs` takes one scalar argument. No capacity is required.

## Result

Returns a value of the argument's type. Unsigned values are returned unchanged. Signed values are negated when negative. For the `float` and `double` overloads, the helper uses an ordered comparison against zero: NaN is returned as NaN, negative zero is returned unchanged, and an infinity keeps its magnitude. The C helper returns the same numeric C type that the registry lists for each overload.

## Bounds and ownership

`abs` takes scalars by clone. It does not allocate, mutate caller storage, or read through a pointer. No declared capacity is involved.

## Errors and traps

`zlang_std_math_abs_i32` traps with `ZLANG_TRAP_OVERFLOW` (code `1`) when `value` is `INT32_MIN`. `zlang_std_math_abs_i64` traps with the same code when `value` is `INT64_MIN`. This check prevents negating a signed minimum value, which has no representable positive result.

The unsigned, `float` and `double` helpers contain no trap. Floating-point helpers do not reject NaN or infinity. Floating arithmetic follows IEEE-754, so division by zero in the surrounding program produces infinity or NaN instead of a Zlang trap.

## State and concurrency

The registry marks `abs` as not stateful and opaque. The emitter passes the argument directly and adds no hidden context parameter. The standard headers own no mutable globals. The same argument always produces the same result, and concurrent calls share no mutable state.

## Generated C

Importing `abs` activates `ZLANG_STD_MATH_ABS_I32` from the first registry entry with that name. The call activates the macro for the selected overload. The example below produces this excerpt:

```c
#define ZLANG_STD_MATH_ABS_I32
...
zlang_std_math_abs_i32(zlang_neg_i32(7));
```

A negative literal is first lowered through the runtime's checked negation helper, `zlang_neg_i32`.

The header guards do not match the registry macros. `math.h` defines all `abs` helpers, including the unsigned ones, only when `ZLANG_STD_MATH_ABS_ACTIVE` is defined. That guard lists `ZLANG_STD_MATH_ABS_I32`, `ZLANG_STD_MATH_ABS_I64`, `ZLANG_STD_MATH_ABS_F32` and `ZLANG_STD_MATH_ABS_F64`; it omits `ZLANG_STD_MATH_ABS_U32` and `ZLANG_STD_MATH_ABS_U64`. Generated C imports activate `ZLANG_STD_MATH_ABS_I32` first, so the unsigned helpers are present in generated programs. A hand-written translation unit that defines only an unsigned `abs` macro does not declare them.

## Example

```zlang
import { write } from "@std/io";
import { abs } from "@std/math";

function main(): integer {
    write(abs(-7));
    write("\n");
    return 0;
}
```

## See also

- [clamp](/docs/standard-library/namespaces/math/functions/clamp)
- [min](/docs/standard-library/namespaces/math/functions/min)
- [max](/docs/standard-library/namespaces/math/functions/max)
- [Catalog](/docs/standard-library/namespaces/math/functions)
