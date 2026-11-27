---
title: Types and values
description: Primitive types, strings, arrays, user types, generics and native results.
navigation:
  title: Types and values
---

# Types and values

Zlang has scalar primitives, fixed arrays, user-defined types, named interface contracts and a closed family of native results.

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

Conversions to signed integer types truncate floating-point input toward zero. They trap on non-finite input and when the result is out of range. Floating arithmetic follows IEEE-754, so division by zero produces infinity or NaN instead of a Zlang trap. The `%` operator accepts only integral types. Bitwise `&`, `|`, `^` and `~` accept integral operands. Shifts accept an integral left operand and a `uint32` count. Unary negation of `int32` and `int64` traps on the signed minimum value.

## User-defined types

A struct type matches only the same user-defined name, including in parameters, returns and initializers. A struct declares fields, one constructor, instance methods and `static function` methods. It may declare `implements` interfaces and `use` traits inside its body. An interface declares fields and method signatures as a named contract. An enum declares named constants and emits named C constants.

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

`result_type<T, status_code>` is a closed native generic family. Registered standard-library applications use native payloads. Source programs may declare `result_type<T, status_code>` for a user struct `T` only when `T` implements `dump(): string`. `T` is the ideal value and `status_code` is the error code. A function with no payload uses `result_type<void, status_code>`. Every result exposes `code`, `what` and, for non-void payloads, `value`.

| Constant | Meaning |
| --- | --- |
| `STATUS_SUCCESS` | success |
| `STATUS_END_OF_INPUT` | end of input |
| `STATUS_IO_ERROR` | input/output failure |
| `STATUS_INVALID_ARGUMENT` | invalid argument |
| `STATUS_CAPACITY` | capacity exceeded |

The compiler maps native applications to the layouts in `<zlang/standard/result/result.h>` and emits a concrete C99 typedef for user payload applications. `what` is a read-only view of a fixed literal message. No allocation or runtime formatting is performed unless `dump()` is compiled in. Since Zlang 0.37 every registered standard library function returns `result_type<T, status_code>`; `T` is its obvious ideal value, pure and total functions report `STATUS_SUCCESS` for valid input, and programmer and resource errors remain traps.

## Result capabilities

`result_type<T, status_code>` is a stable envelope, not only an error wrapper. Optional
capabilities are selected by compile-time flags:

| Method | Result | Disabled behavior |
| --- | --- | --- |
| `time()` | `uint64` elapsed nanoseconds | returns `0` without reading a clock |
| `dump()` | bounded canonical `string` view | returns the empty view |

`ZLANG_STD_RESULT_TIMING` adds elapsed-time state and instruments registered standard library
calls. `ZLANG_STD_RESULT_DUMP` adds bounded storage for `dump()`. Neither capability allocates.
`dump()` includes `code`, `what`, the payload and elapsed time. A user-defined `T` must implement
`dump(): string` before `result_type<T, status_code>.dump()` is valid.
