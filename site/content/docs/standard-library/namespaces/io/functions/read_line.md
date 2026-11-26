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

`import { read_line } from "@std/io";` activates `ZLANG_STD_IO_READ_LINE`. The emitted preamble includes `<zlang/runtime/runtime.h>`, which includes `zlang/standard/io/io.h`. Excerpt of the preamble:

```c
#define ZLANG_STD_IO_READ_LINE

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>
```

The header defines the helper with the view pointer, storage pointer and capacity in that order:

```c
ZLANG_STATIC_INLINE int32_t zlang_std_io_read_line(zlang_string_t *view, char *storage,
                                                   uint32_t capacity) {
    return zlang_std_io_read_impl(view, storage, capacity, false);
}
```

For `string[32] line; read_line(line);`, the emitted `app_fn_main` declares the view and its storage before the call. Excerpt:

```c
    char zlang_gen_line_storage_0[ZLANG_LINE_CAPACITY + ZLANG_ONE] = { ZLANG_ZERO };
    zlang_string_t app_local_line = (zlang_string_t){ zlang_gen_line_storage_0, ZLANG_ZERO };
    zlang_std_io_read_line(&app_local_line, zlang_gen_line_storage_0, 32);
```

`app_fn_main` is the emitted C function for the Zlang `main`. `app_local_line` is the view for the source variable `line`. `zlang_gen_line_storage_0` is the compiler-created backing array for that declaration; the suffix `0` separates it from other generated storage. `ZLANG_LINE_CAPACITY` is the generated capacity macro for `string[32]`, and `ZLANG_ZERO` and `ZLANG_ONE` are generated value constants. The call passes the source capacity `32`.

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
