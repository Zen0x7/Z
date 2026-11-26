---
title: Overview
description: Catalog of functions exported by @std/io.
domain: io
kind: catalog
status: implemented
navigation:
  title: Overview
---

# Functions

`@std/io` registers one read function, one line reader and eight `write` overloads. The compiler picks the overload from the argument type.

## Catalog

| Name | Signature | Parameter mode | C symbol | Macro |
| --- | --- | --- | --- | --- |
| [`read`](/docs/standard-library/namespaces/io/functions/read) | `function read(reference string value): int32` | reference | `zlang_std_io_read` | `ZLANG_STD_IO_READ` |
| [`read_line`](/docs/standard-library/namespaces/io/functions/read_line) | `function read_line(reference string value): int32` | reference | `zlang_std_io_read_line` | `ZLANG_STD_IO_READ_LINE` |
| [`write`](/docs/standard-library/namespaces/io/functions/write) | `function write(clone string value): string` | clone | `zlang_std_io_write_string` | `ZLANG_STD_IO_WRITE_STRING` |
| `write` | `function write(clone boolean value): boolean` | clone | `zlang_std_io_write_bool` | `ZLANG_STD_IO_WRITE_BOOL` |
| `write` | `function write(clone int32 value): int32` | clone | `zlang_std_io_write_i32` | `ZLANG_STD_IO_WRITE_I32` |
| `write` | `function write(clone uint32 value): uint32` | clone | `zlang_std_io_write_u32` | `ZLANG_STD_IO_WRITE_U32` |
| `write` | `function write(clone int64 value): int64` | clone | `zlang_std_io_write_i64` | `ZLANG_STD_IO_WRITE_I64` |
| `write` | `function write(clone uint64 value): uint64` | clone | `zlang_std_io_write_u64` | `ZLANG_STD_IO_WRITE_U64` |
| `write` | `function write(clone float value): float` | clone | `zlang_std_io_write_f32` | `ZLANG_STD_IO_WRITE_F32` |
| `write` | `function write(clone double value): double` | clone | `zlang_std_io_write_f64` | `ZLANG_STD_IO_WRITE_F64` |

The registry gives each `write` overload the same return type as its argument. Every generated C wrapper returns `bool` from the runtime print helper. Binding the `string` overload's result to a `string` emits C that does not compile; numeric bindings compile but receive a converted `bool`. Use `write` as an expression statement and ignore the result.

At an import statement the emitter activates the first registered overload, `ZLANG_STD_IO_WRITE_STRING`, before adding the resolved overload's macro at each call.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/io/functions) — this catalog.
- [read](/docs/standard-library/namespaces/io/functions/read)
- [read_line](/docs/standard-library/namespaces/io/functions/read_line)
- [write](/docs/standard-library/namespaces/io/functions/write)
