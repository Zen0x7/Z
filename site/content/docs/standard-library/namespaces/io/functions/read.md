---
title: read
description: Read input into caller-owned bounded storage, keeping the line terminator.
domain: io
kind: function
status: implemented
stateful: false
opaque: false
canonical_symbols:
  - zlang_std_io_read
activation_macros:
  - ZLANG_STD_IO_READ
---

# read

`read` reads from standard input into a bounded string owned by the caller. It keeps a newline terminator when one is present.

## Import

```zlang
import { read } from "@std/io";
```

## Signature

```zlang
function read(reference string value): integer
```

The parameter is passed by reference. The function writes into the caller's storage.

## Parameters

| Name | Meaning | Ownership |
| --- | --- | --- |
| `value` | Destination bounded string | reference |

The destination must have enough declared capacity for the input plus the terminator.

## Result

Returns the number of bytes written. Returns `-1` at end of input when no bytes were read.

## Bounds and ownership

- `read` never allocates.
- `read` does not grow the string.
- Capacity is checked before each write.
- The destination remains owned by the caller.

## Errors and traps

- Null storage or zero capacity traps with invalid argument.
- Exceeding capacity traps with capacity.

## State and concurrency

The registry marks `read` as not stateful and not opaque. Standard headers own no mutable globals. Input order between concurrent readers depends on the host process.

## Generated C

```c
#define ZLANG_STD_IO_READ
#include <zlang/runtime/runtime.h>
```

The generated call passes storage, a pointer to the bounded string view and the declared capacity:

```c
zlang_std_io_read(&app_local_view, app_local_storage, 32U);
```

## Example

```zlang
import { read } from "@std/io";

function main(): integer {
    string[32] line;
    read(line);
    return 0;
}
```

## See also

- [read_line](/docs/standard-library/namespaces/io/functions/read_line)
- [write](/docs/standard-library/namespaces/io/functions/write)
