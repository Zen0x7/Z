---
title: Types
description: Primitive types, strings, arrays and numeric conversions.
navigation:
  title: Types
---

# Types

Zlang has eight numeric and scalar primitives, plus structs and enums.

| Zlang | Meaning | Emitted C |
| --- | --- | --- |
| `integer` | signed 32-bit | `int32_t` |
| `unsigned` | unsigned 32-bit | `uint32_t` |
| `int64` | signed 64-bit | `int64_t` |
| `uint64` | unsigned 64-bit | `uint64_t` |
| `float` | IEEE-754 binary32 | `float` |
| `double` | IEEE-754 binary64 | `double` |
| `boolean` | `true` or `false` | `bool` |
| `string` | bounded UTF-8 view | `zlang_string_t` |

There is no implicit conversion between numeric types. Use explicit conversion calls:

```zlang
integer a = to_integer(value);
unsigned b = to_unsigned(value);
int64 c = to_int64(value);
uint64 d = to_uint64(value);
float e = to_float(value);
double f = to_double(value);
```

Conversion to an integer traps when the value is out of range. Floating arithmetic follows IEEE-754, so division by zero produces infinity or NaN instead of a Zlang trap.

## Strings and arrays

A string is a non-owning view over bytes. It never allocates. A string variable with capacity reserves fixed caller-owned storage.

An array is fixed-shape. Its capacity is part of its type. Arrays with different extents are different types.

```zlang
array[4] integer values;
array[8] string[32] names;
```

See [Declarations](/docs/language/declarations) for declaration syntax.
