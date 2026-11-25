---
title: read
description: Read input into caller-owned bounded storage, keeping the line terminator.
domain: io
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_io_read
activation_macros:
  - ZLANG_STD_IO_READ
---

# read

## Summary

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

The declared capacity counts content bytes. The emitted buffer reserves one more byte for the C terminator. `read` adds the newline to the content when it is present, so the input bytes and the newline must fit in the declared capacity.

## Result

Returns the number of bytes written. Returns `-1` at end of input when no bytes were read.

## Bounds and ownership

- `read` never allocates.
- `read` does not grow the string.
- Capacity is checked before each content byte and before a retained newline.
- The destination remains owned by the caller.

## Errors and traps

- Null storage or zero capacity traps with invalid argument.
- Exceeding capacity traps with capacity.

## State and concurrency

The registry marks `read` as not stateful and opaque. The standard header owns no mutable globals. Zlang source has no threads; sequential reads follow program order.

## Generated C

The import emits:

```c
#define ZLANG_STD_IO_READ
#include <zlang/runtime/runtime.h>
```

The C helper takes the view pointer, storage pointer and capacity in that order:

```c
int32_t zlang_std_io_read(zlang_string_t *view, char *storage, uint32_t capacity);
```

For `string[32] line; read(line);`, the emitter uses the declaration's view, backing storage and capacity:

```c
zlang_std_io_read(&app_local_line, zlang_gen_line_storage_0, 32);
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
- [Catalog](/docs/standard-library/namespaces/io/functions)
