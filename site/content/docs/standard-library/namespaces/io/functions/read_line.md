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
activation_macros:
  - ZLANG_STD_IO_READ_LINE
---

# read_line

## Summary

`read_line` reads one line from standard input into a bounded string. It drops the newline terminator.

## Import

```zlang
import { read_line } from "@std/io";
```

## Signature

```zlang
function read_line(reference string value): integer
```

The parameter is passed by reference.

## Parameters

| Name | Meaning | Ownership |
| --- | --- | --- |
| `value` | Destination bounded string | reference |

The declared capacity counts content bytes, excluding the dropped line terminator. The emitted buffer reserves one more byte for the C terminator.

## Result

Returns the content length written to the view, excluding the line terminator. Returns `-1` at end of input when no bytes were read.

The helper stops at `\n`. A preceding `\r` remains in the view on hosts that do not translate CRLF in text mode. This differs from the CRLF normalization stated in `SPEC.md` 0.25.

## Bounds and ownership

The destination keeps ownership of its storage. Capacity is checked before each content byte. No buffer is allocated by the function.

## Errors and traps

- Null storage or zero capacity traps with invalid argument.
- Exceeding capacity traps with capacity.

## State and concurrency

The registry marks `read_line` as not stateful and opaque. The standard header owns no mutable globals. Zlang source has no threads; sequential reads follow program order.

## Generated C

The import emits:

```c
#define ZLANG_STD_IO_READ_LINE
#include <zlang/runtime/runtime.h>
```

The C helper takes the view pointer, storage pointer and capacity in that order:

```c
int32_t zlang_std_io_read_line(zlang_string_t *view, char *storage, uint32_t capacity);
```

For `string[32] line; read_line(line);`, the emitter uses the declaration's view, backing storage and capacity:

```c
zlang_std_io_read_line(&app_local_line, zlang_gen_line_storage_0, 32);
```

## Example

```zlang
import { read_line } from "@std/io";

function main(): integer {
    string[32] line;
    read_line(line);
    return 0;
}
```

## See also

- [read](/docs/standard-library/namespaces/io/functions/read)
- [write](/docs/standard-library/namespaces/io/functions/write)
- [Catalog](/docs/standard-library/namespaces/io/functions)
