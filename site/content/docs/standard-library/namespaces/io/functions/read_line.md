---
title: read_line
description: Read one line into caller-owned bounded storage without the newline.
domain: io
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_io_read_line
  - zlang_std_io_read_line_stream
activation_macros:
  - ZLANG_STD_IO_READ_LINE
---

# read_line

## Summary

`read_line` reads one line into caller-owned bounded storage, drops the newline and returns the transferred byte count in a canonical `result_type<int32, status_code>`.

## Import

```zlang
import { read_line } from "@std/io";
```

## Signatures

```zlang
function read_line(reference string destination): result_type<int32, status_code>
function read_line(clone stream source, reference string destination,
                   uint32 offset, uint32 count): result_type<int32, status_code>
```

The first overload reads from `stdin` at offset `0` for the declared capacity.

## Result

`value` is the number of content bytes written. `code` is `STATUS_SUCCESS`, `STATUS_END_OF_INPUT` when no bytes were read at EOF, or `STATUS_IO_ERROR` on stream failure. A trailing `\r` remains in the view on hosts that do not translate CRLF in text mode.

## Bounds and ownership

The destination remains owned by the caller. Invalid offsets, null storage and capacity overflow trap. No allocation occurs.

## Generated C

```c
zlang_std_result_i32_status_t app_local_result =
    zlang_std_io_read_line_stream(zlang_std_io_stream_stdin(), &app_local_line,
                                  storage, 32U, 0U, 8U);
```

## Example

```zlang
import { read_line } from "@std/io";

function main(): int32 {
    string[32] line;
    result_type<int32, status_code> count = read_line(line);
    write(count.value);
    return 0;
}
```

## See also

- [read](/docs/standard-library/namespaces/io/functions/read)
- [write](/docs/standard-library/namespaces/io/functions/write)
- [Catalog](/docs/standard-library/namespaces/io/functions)
