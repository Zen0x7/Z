---
title: sqrt
description: Apply the registered square-root helper to finite, non-negative floating-point input.
domain: math
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_math_sqrt_f32
  - zlang_std_math_sqrt_f64
activation_macros:
  - ZLANG_STD_MATH_SQRT_F32
  - ZLANG_STD_MATH_SQRT_F64
---

# sqrt

## Summary

`sqrt` applies the registered square-root helper to a finite, non-negative floating-point value.

## Import

```zlang
import { sqrt } from "@std/math";
```

## Signature

```zlang
function sqrt(clone float value): float
```

| Registry signature | C symbol | Activation macro |
| --- | --- | --- |
| `function sqrt(clone float value): float` | `zlang_std_math_sqrt_f32` | `ZLANG_STD_MATH_SQRT_F32` |
| `function sqrt(clone double value): double` | `zlang_std_math_sqrt_f64` | `ZLANG_STD_MATH_SQRT_F64` |

There are no integer overloads. Integer input is rejected by overload resolution because Zlang has no implicit numeric conversion.

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `value` | `float` or `double` | clone | Non-negative, finite input |

`float` is IEEE-754 binary32 and `double` is IEEE-754 binary64. No capacity is required.

## Result

Returns a value of the argument's type. For `value` between zero and one inclusive, the helper returns `value` unchanged. For positive fractions, that result is not the mathematical square root; for example, `sqrt(0.25)` returns `0.25`. For values above one, it bisects the interval `[1, value]`: 32 iterations for `float` and 64 iterations for `double`, returning the lower bound. The result is a deterministic approximation, not a call to `sqrtf` or `sqrt` from `<math.h>`. The function does not promise correct rounding.

Negative zero is accepted and returned. No caller storage is written.

## Bounds and ownership

`sqrt` takes one scalar by clone. It does not allocate, mutate caller storage, or read through a pointer. The bisection loop uses local variables only.

## Errors and traps

The helper traps with `ZLANG_TRAP_INVALID_ARGUMENT` (code `4`) when `value < 0` or when `value` is not finite. NaN and both infinities therefore trap. Non-negative finite inputs never trap; IEEE-754 division inside the loop operates on finite operands and produces no Zlang trap.

## State and concurrency

The registry marks `sqrt` as not stateful and opaque. The emitter passes the argument directly and adds no hidden context parameter. The standard headers own no mutable globals, so repeated and concurrent calls share no mutable state.

## Generated C

Importing `sqrt` activates `ZLANG_STD_MATH_SQRT_F32`, the first registry entry with that name. A `double` call activates `ZLANG_STD_MATH_SQRT_F64` as well. The example below produces this excerpt:

```c
#define ZLANG_STD_MATH_SQRT_F32
#define ZLANG_STD_MATH_SQRT_F64
...
zlang_std_math_sqrt_f64(9.0);
```

The registry records one macro per overload. The generated prologue contains `ZLANG_STD_MATH_SQRT_F32` even though the program never calls the `float` overload. This is a mismatch between the registry's per-overload activation model and the emitter's import handling. The call itself uses the resolved macro and symbol.

## Example

```zlang
import { write } from "@std/io";
import { sqrt } from "@std/math";

function main(): integer {
    write(sqrt(9.0));
    write("\n");
    return 0;
}
```

## See also

- [abs](/docs/standard-library/namespaces/math/functions/abs)
- [clamp](/docs/standard-library/namespaces/math/functions/clamp)
- [Catalog](/docs/standard-library/namespaces/math/functions)
