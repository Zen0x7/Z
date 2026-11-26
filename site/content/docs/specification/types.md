---
title: Types and values
description: Primitive types, strings, arrays, user types, generics and native results.
navigation:
  title: Types and values
---

# Types and values

Zlang has scalar primitives, fixed arrays, user-defined types and a closed family of native results.

## Primitive types

| Zlang | Meaning | Emitted C |
| --- | --- | --- |
| `int32` | signed 32-bit | `int32_t` |
| `uint32` | unsigned 32-bit | `uint32_t` |
| `int64` | signed 64-bit | `int64_t` |
| `uint64` | unsigned 64-bit | `uint64_t` |
| `float` | IEEE-754 binary32 | `float` |
| `double` | IEEE-754 binary64 | `double` |
| `boolean` | `true` or `false` | `bool` |
| `string` | non-owning UTF-8 byte view | `zlang_string_t` |
| `arguments` | host argument view | `zlang_arguments_t` |

A string has a pointer to its first byte and a byte length. A string literal is interned by the compiler and emitted in read-only storage. String `==` and `!=` compare content through the runtime helper. String ordering is rejected.

The `arguments` type appears in `main` and in `@std/program_options` functions. It borrows host-owned memory and cannot be constructed in Zlang source.

## Arrays

An array is a fixed-shape container written as `<type>[<capacity>]`. Capacity is part of the type, not a dynamic property.

```zlang
array[4] int32 values;
array[8] string[32] names;
```

The capacity must be a positive int32 literal. A brace initializer may infer the capacity from its element count. An explicit capacity may be combined with a shorter initializer; the capacity stays unchanged and the remaining elements are zero-filled.

An array has no logical length and no append operation. `length(values)` returns its fixed capacity as `uint32`. Every index is checked against that capacity. Arrays with different extents are different types, and arrays do not participate in inheritance.

## Numeric rules

There is no implicit conversion between numeric types, or between numeric types and `boolean`. Binary arithmetic and comparisons require both operands to have exactly the same type. Explicit conversion calls take exactly one numeric operand:

```zlang
int32 a = to_int32(value);
uint32 b = to_uint32(value);
int64 c = to_int64(value);
uint64 d = to_uint64(value);
float e = to_float(value);
double f = to_double(value);
```

Conversions to signed integer types truncate floating-point input toward zero. They trap on non-finite input and when the result is out of range. Floating arithmetic follows IEEE-754, so division by zero produces infinity or NaN instead of a Zlang trap. The `%` operator accepts only integral types. Unary negation of `int32` and `int64` traps on the signed minimum value.

## User-defined types

A struct type matches only the same user-defined name, including in parameters, returns and initializers. A struct declares fields, one constructor, instance methods and `static function` methods. An enum declares named constants and emits named C constants.

A derived struct may be used where its base struct or `reference Base` is expected. The reverse conversion is rejected. See [Functions, methods and traits](/docs/specification/functions) for method and inheritance rules.

## Generic applications

A generic declaration lists type parameters after its name, and an application supplies concrete type arguments:

```zlang
struct Pair<T, U> {
    T first;
    U second;
}

Pair<int32, boolean> pair;
int32 value = identity<int32>(pair.first);
```

A declaration may list up to four type parameters. An application may pass up to four concrete type arguments. Nested type arguments are not part of the language. Each application is monomorphized into a concrete C99 type or function. No runtime generic representation, heap allocation or reflection is emitted.

## Native results

`result_type<T, status_code>` is a closed native generic family. `T` is the ideal value and `status_code` is the error code. A function with no payload uses `result_type<void, status_code>`. Every result exposes `code`, `what` and, for non-void payloads, `value`.

| Constant | Meaning |
| --- | --- |
| `STATUS_SUCCESS` | success |
| `STATUS_END_OF_INPUT` | end of input |
| `STATUS_IO_ERROR` | input/output failure |
| `STATUS_INVALID_ARGUMENT` | invalid argument |
| `STATUS_CAPACITY` | capacity exceeded |

The compiler maps canonical applications to the layouts in `<zlang/standard/result/result.h>`. `what` is a read-only view of a fixed literal message. No runtime formatting or allocation is performed. Standard library functions keep their existing signatures; none returns a native result.
