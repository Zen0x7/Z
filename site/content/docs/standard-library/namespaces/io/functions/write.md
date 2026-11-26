---
title: write
description: Write a bounded string or scalar to a stream.
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
  - zlang_std_io_write_string_stream
  - zlang_std_io_write_bool_stream
  - zlang_std_io_write_i32_stream
  - zlang_std_io_write_u32_stream
  - zlang_std_io_write_i64_stream
  - zlang_std_io_write_u64_stream
  - zlang_std_io_write_f32_stream
  - zlang_std_io_write_f64_stream
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

`write` sends one value to a stream and returns the number of bytes written in a canonical `result_type<int32, status_code>`.

## Import

```zlang
import { write } from "@std/io";
```

## Signatures

These declarations are overloads of one function. The one-value forms use implicit `STDOUT`:

```zlang
function write(clone string value): result_type<int32, status_code>
function write(clone boolean value): result_type<int32, status_code>
function write(clone int32 value): result_type<int32, status_code>
function write(clone uint32 value): result_type<int32, status_code>
function write(clone int64 value): result_type<int32, status_code>
function write(clone uint64 value): result_type<int32, status_code>
function write(clone float value): result_type<int32, status_code>
function write(clone double value): result_type<int32, status_code>
```

The two-argument forms take a `clone stream` destination (`STDIN`, `STDOUT`, `STDERR` or another handle that implements the same contract):

```zlang
function write(clone stream target, clone string value): result_type<int32, status_code>
function write(clone stream target, clone boolean value): result_type<int32, status_code>
function write(clone stream target, clone int32 value): result_type<int32, status_code>
```

The remaining scalar overloads follow the same pattern.

A `clone string` copies the bounded view descriptor only; buffer bytes are never copied. A `clone stream` copies a borrowed descriptor, not the device or its state.

## Result

`value` is the transferred byte count, `code` is a `status_code` (`STATUS_SUCCESS` on a complete write, `STATUS_IO_ERROR` when the stream fails), and `what` is a bounded read-only view of the fixed status message. The `message()` accessor reads `what`.

```zlang
result_type<int32, status_code> response = write(STDOUT, 7);
if (response.is_ok()) {
    write(response.value);
} else {
    write(response.message());
}
```

## Bounds and ownership

`write` never allocates, grows a buffer or takes ownership of the stream. Null stream or malformed string views trap with invalid argument.

## Generated C

The emitted call returns a canonical result struct:

```c
zlang_std_result_i32_status_t app_local_response =
    zlang_std_io_write_i32_stream(zlang_std_io_stream_stdout(), 7);
```

`runtime.h` includes `zlang/standard/io/stream.h` for stream handles and `zlang/standard/result/result.h` when a native result is used.

## Example

```zlang
import { write } from "@std/io";

function main(): int32 {
    write("Hello from Zlang");
    write("\n");
    return 0;
}
```

## See also

- [read](/docs/standard-library/namespaces/io/functions/read)
- [Catalog](/docs/standard-library/namespaces/io/functions)
