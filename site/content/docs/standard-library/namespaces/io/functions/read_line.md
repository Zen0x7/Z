---
title: read_line
description: Read one line into caller-owned bounded storage without the newline.
domain: io
kind: function
status: implemented
stateful: false
opaque: false
canonical_symbols:
  - zlang_std_io_read_line
activation_macros:
  - ZLANG_STD_IO_READ_LINE
---

# read_line

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

## Result

Returns the number of bytes written. Returns `-1` at end of input when no bytes were read.

## Bounds and ownership

The destination keeps ownership of its storage. Capacity is checked before each write. No buffer is allocated by the function.

## Errors and traps

- Null storage or zero capacity traps with invalid argument.
- Exceeding capacity traps with capacity.

## State and concurrency

The registry marks `read_line` as not stateful and not opaque. Input order between concurrent readers depends on the host process.

## Generated C

```c
#define ZLANG_STD_IO_READ_LINE
#include <zlang/runtime/runtime.h>
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
