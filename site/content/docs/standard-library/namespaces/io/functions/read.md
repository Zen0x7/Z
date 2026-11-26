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

`read` reads bytes into caller-owned bounded storage and returns the transferred byte count in a canonical `result_type<int32, status_code>`.

## Import

```zlang
import { read } from "@std/io";
```

## Signatures

```zlang
function read(reference string destination): result_type<int32, status_code>
function read(clone stream source, reference string destination,
              uint32 offset, uint32 count): result_type<int32, status_code>
```

The two declarations are overloads of one function. The convenience overload takes a `reference string` destination (caller-owned mutable bounded storage) and reads implicit `STDIN` from offset `0` for the declared capacity. The explicit overload takes a `clone stream` source, a borrowed descriptor rather than a copy of the device, and reads into a bounded `offset`/`count` window of the destination.

## Result

`value` is the number of bytes read, `code` is `STATUS_SUCCESS`, `STATUS_END_OF_INPUT` when no bytes were read at EOF, or `STATUS_IO_ERROR` on stream failure, and `what` is a bounded read-only view of the fixed status message. The C helper returns `zlang_std_result_i32_status_t`, not a bare `int32_t`.

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
    result_type<int32, status_code> count = read(line);
    write(count.value);
    return 0;
}
```

## See also

- [read_line](/docs/standard-library/namespaces/io/functions/read_line)
- [write](/docs/standard-library/namespaces/io/functions/write)
- [Catalog](/docs/standard-library/namespaces/io/functions)
