---
title: write
description: Write a bounded string or scalar to the standard output stream.
domain: io
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_io_write_string
  - zlang_std_io_write_bool
  - zlang_std_io_write_i32
  - zlang_std_io_write_u32
  - zlang_std_io_write_i64
  - zlang_std_io_write_u64
  - zlang_std_io_write_f32
  - zlang_std_io_write_f64
activation_macros:
  - ZLANG_STD_IO_WRITE_STRING
  - ZLANG_STD_IO_WRITE_BOOL
  - ZLANG_STD_IO_WRITE_I32
  - ZLANG_STD_IO_WRITE_U32
  - ZLANG_STD_IO_WRITE_I64
  - ZLANG_STD_IO_WRITE_U64
  - ZLANG_STD_IO_WRITE_F32
  - ZLANG_STD_IO_WRITE_F64
---

# write

## Summary

`write` sends one value to standard output. The compiler selects an overload from the argument type.

## Import

```zlang
import { write } from "@std/io";
```

## Signature

The registry exposes eight overloads:

| Registry signature | C symbol | Activation macro |
| --- | --- | --- |
| `function write(clone string value): string` | `zlang_std_io_write_string` | `ZLANG_STD_IO_WRITE_STRING` |
| `function write(clone boolean value): boolean` | `zlang_std_io_write_bool` | `ZLANG_STD_IO_WRITE_BOOL` |
| `function write(clone integer value): integer` | `zlang_std_io_write_i32` | `ZLANG_STD_IO_WRITE_I32` |
| `function write(clone unsigned value): unsigned` | `zlang_std_io_write_u32` | `ZLANG_STD_IO_WRITE_U32` |
| `function write(clone int64 value): int64` | `zlang_std_io_write_i64` | `ZLANG_STD_IO_WRITE_I64` |
| `function write(clone uint64 value): uint64` | `zlang_std_io_write_u64` | `ZLANG_STD_IO_WRITE_U64` |
| `function write(clone float value): float` | `zlang_std_io_write_f32` | `ZLANG_STD_IO_WRITE_F32` |
| `function write(clone double value): double` | `zlang_std_io_write_f64` | `ZLANG_STD_IO_WRITE_F64` |

Every overload has one parameter passed by value. The registry records the types, modes, C symbols and activation macros; the parameter name `value` comes from the C helper.

## Parameters

| Name | Meaning | Ownership |
| --- | --- | --- |
| `value` | The scalar or bounded string to write | clone |

A string argument must respect its declared capacity. `write` does not grow it.

## Result

The registry gives each overload the same return type as its argument. Every generated C wrapper returns `bool` from the runtime print helper. An expression statement compiles because the emitted call's result is discarded. Binding the `string` overload's result to a `string` initializes a `zlang_string_t` from a `bool`, which C rejects. Numeric bindings compile but receive a converted `bool` instead of the registry type. Do not store or chain the result of `write`.

Treat `write` as an effect and use it for its output.

## Bounds and ownership

- No buffer is allocated by `write`.
- No caller-owned storage is mutated.
- String content is read through `zlang_string_t`.
- The write helper returns `false` for a malformed view or a failed stream; the generated expression statement discards that value.

## Errors and traps

`write` has no Zlang-level status value and emits no trap. The runtime helper returns `false` when a write fails, and generated C discards that result. No error string is formatted at runtime.

## State and concurrency

The registry marks `write` as not stateful and opaque. The standard header owns no mutable globals. Zlang source has no threads; `SPEC.md` requires generated programs to preserve the observable order of `write` calls.

## Generated C

`import { write } from "@std/io";` activates `ZLANG_STD_IO_WRITE_STRING`, the first registered `write` overload. Each call also activates the macro for its resolved overload. A program that calls `write(1)` emits both macros:

```c
#define ZLANG_STD_IO_WRITE_STRING
#define ZLANG_STD_IO_WRITE_I32
#include <zlang/runtime/runtime.h>
```

The call then uses the resolved C symbol:

```c
zlang_std_io_write_i32(1);
```

The wrapper returns `bool`; the emitted expression statement discards that value.

## Example

```zlang
import { write } from "@std/io";

function main(): integer {
    write("Hello from Zlang");
    write("\n");
    return 0;
}
```

## See also

- [read](/docs/standard-library/namespaces/io/functions/read)
- [read_line](/docs/standard-library/namespaces/io/functions/read_line)
- [Catalog](/docs/standard-library/namespaces/io/functions)
- [Inputs and Outputs overview](/docs/standard-library/namespaces/io)
- [Generated C](/docs/standard-library/generated-c)
