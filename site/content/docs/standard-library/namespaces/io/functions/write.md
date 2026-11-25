---
title: write
description: Write a bounded string or scalar to the standard output stream.
domain: io
kind: function
status: implemented
stateful: false
opaque: false
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

`write` sends one value to standard output. The compiler selects an overload from the argument type.

## Import

```zlang
import { write } from "@std/io";
```

## Signature

The registry currently exposes these overloads:

| Argument | C symbol | Macro |
| --- | --- | --- |
| `string` | `zlang_std_io_write_string` | `ZLANG_STD_IO_WRITE_STRING` |
| `boolean` | `zlang_std_io_write_bool` | `ZLANG_STD_IO_WRITE_BOOL` |
| `integer` | `zlang_std_io_write_i32` | `ZLANG_STD_IO_WRITE_I32` |
| `unsigned` | `zlang_std_io_write_u32` | `ZLANG_STD_IO_WRITE_U32` |
| `int64` | `zlang_std_io_write_i64` | `ZLANG_STD_IO_WRITE_I64` |
| `uint64` | `zlang_std_io_write_u64` | `ZLANG_STD_IO_WRITE_U64` |
| `float` | `zlang_std_io_write_f32` | `ZLANG_STD_IO_WRITE_F32` |
| `double` | `zlang_std_io_write_f64` | `ZLANG_STD_IO_WRITE_F64` |

Every overload has one parameter. The registry marks it as passed by value.

## Parameters

| Name | Meaning | Ownership |
| --- | --- | --- |
| `value` | The scalar or bounded string to write | clone |

A string argument must respect its declared capacity. `write` does not grow it.

## Result

The registry lists a return type of `string`, while the generated C wrappers return `bool` from the runtime print helpers. These two surfaces do not agree yet. Do not store or chain the result of `write`.

Treat `write` as effectful and use it for its output.

## Bounds and ownership

- No buffer is allocated by `write`.
- No caller-owned storage is mutated.
- String content is read through `zlang_string_t`.
- A malformed bounded string is a runtime problem, not a language-level retry.

## Errors and traps

`write` has no Zlang-level status value. Runtime helpers may trap on invalid bounded values. The generated C does not format error strings at runtime.

## State and concurrency

The registry marks `write` as not stateful and not opaque. Standard headers own no mutable globals. Output order between concurrent writers depends on the host process, so synchronize at the application level if order matters.

## Generated C

Importing `write` activates the macro for the selected overload:

```c
#define ZLANG_STD_IO_WRITE_I32
#include <zlang/runtime/runtime.h>
```

The generated call is deterministic for a given value:

```c
zlang_std_io_write_i32(app_local_value);
```

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

- [Inputs and Outputs overview](/docs/standard-library/namespaces/io)
- [Catalog and overview](/docs/standard-library/namespaces/io)
- [Generated C](/docs/standard-library/generated-c)
