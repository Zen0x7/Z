---
title: read
description: Read bytes into caller-owned bounded storage.
domain: io
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_io_read
  - zlang_std_io_read_stream
activation_macros:
  - ZLANG_STD_IO_READ
---

# read

## Summary

`read` reads bytes into caller-owned bounded storage and returns the transferred byte count in a canonical `Result<int32, Status>`.

## Import

```zlang
import { read } from "@std/io";
```

## Signatures

```zlang
function read(reference string destination): Result<int32, Status>
function read(clone stream source, reference string destination,
              uint32 offset, uint32 count): Result<int32, Status>
```

The first overload reads from `stdin` at offset `0` for the declared capacity. The explicit overload reads from any stream into a bounded window.

## Result

`value` is the number of bytes read. `code` is `STATUS_SUCCESS`, `STATUS_END_OF_INPUT` when no bytes were read at EOF, or `STATUS_IO_ERROR` on stream failure.

## Bounds and ownership

The destination remains owned by the caller. `offset + count` must fit in the declared capacity. Invalid offsets, null storage and capacity overflow trap. No buffer bytes are copied beyond the destination write.

## Generated C

```c
zlang_std_result_i32_status_t app_local_result =
    zlang_std_io_read_stream(zlang_std_io_stream_stdin(), &app_local_line,
                             storage, 32U, 0U, 8U);
```

## Example

```zlang
import { read } from "@std/io";

function main(): int32 {
    string[32] line;
    Result<int32, Status> count = read(line);
    write(count.value);
    return 0;
}
```

## See also

- [read_line](/docs/standard-library/namespaces/io/functions/read_line)
- [write](/docs/standard-library/namespaces/io/functions/write)
- [Catalog](/docs/standard-library/namespaces/io/functions)
