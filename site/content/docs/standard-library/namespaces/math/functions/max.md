---
title: max
description: Select the greater of two numeric values.
domain: math
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_math_max_i32
  - zlang_std_math_max_u32
  - zlang_std_math_max_i64
  - zlang_std_math_max_u64
  - zlang_std_math_max_f32
  - zlang_std_math_max_f64
activation_macros:
  - ZLANG_STD_MATH_MAX_I32
  - ZLANG_STD_MATH_MAX_U32
  - ZLANG_STD_MATH_MAX_I64
  - ZLANG_STD_MATH_MAX_U64
  - ZLANG_STD_MATH_MAX_F32
  - ZLANG_STD_MATH_MAX_F64
---

# max

## Summary

`max` selects the greater of two scalar values.

## Import

```zlang
import { max } from "@std/math";
```

## Signature

```zlang
function max(clone integer left, clone integer right): integer
```

| Registry signature | C symbol | Activation macro |
| --- | --- | --- |
| `function max(clone integer left, clone integer right): integer` | `zlang_std_math_max_i32` | `ZLANG_STD_MATH_MAX_I32` |
| `function max(clone unsigned left, clone unsigned right): unsigned` | `zlang_std_math_max_u32` | `ZLANG_STD_MATH_MAX_U32` |
| `function max(clone int64 left, clone int64 right): int64` | `zlang_std_math_max_i64` | `ZLANG_STD_MATH_MAX_I64` |
| `function max(clone uint64 left, clone uint64 right): uint64` | `zlang_std_math_max_u64` | `ZLANG_STD_MATH_MAX_U64` |
| `function max(clone float left, clone float right): float` | `zlang_std_math_max_f32` | `ZLANG_STD_MATH_MAX_F32` |
| `function max(clone double left, clone double right): double` | `zlang_std_math_max_f64` | `ZLANG_STD_MATH_MAX_F64` |

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `left` | one of `integer`, `unsigned`, `int64`, `uint64`, `float`, `double` | clone | First candidate |
| `right` | same type as `left` | clone | Second candidate |

Both arguments must have the same type. No capacity is required.

## Result

Returns `left` when `left > right`, otherwise `right`. The return type matches the argument type and the registry return type.

For `float` and `double`, the helper uses an ordered comparison. When `left` is NaN, the comparison is false and `right` is returned. When `right` is NaN, the comparison is false and the NaN is returned. Signed zeros compare equal, so the second argument is returned.

## Bounds and ownership

`max` takes two scalars by clone. It does not allocate, mutate caller storage, or read through a pointer. No declared capacity is involved.

## Errors and traps

No overload traps. The floating helpers accept NaN and infinities without a validity check, and IEEE-754 ordering decides the result.

## State and concurrency

The registry marks `max` as not stateful and opaque. The emitter passes both arguments directly and adds no hidden context parameter. The standard headers own no mutable globals. The same two arguments always produce the same result, and repeated and concurrent calls share no mutable state.

## Generated C

Importing `max` activates `ZLANG_STD_MATH_MAX_I32` from the first registry entry with that name. The call activates the macro for the selected overload. The Example produces this excerpt:

```c
#define ZLANG_STD_MATH_MAX_I32
...
zlang_std_math_max_i32(2, 3);
```


## Example

```zlang
import { write } from "@std/io";
import { max } from "@std/math";

function main(): integer {
    write(max(2, 3));
    write("\n");
    return 0;
}
```

## See also

- [min](/docs/standard-library/namespaces/math/functions/min)
- [abs](/docs/standard-library/namespaces/math/functions/abs)
- [clamp](/docs/standard-library/namespaces/math/functions/clamp)
- [Catalog](/docs/standard-library/namespaces/math/functions)
