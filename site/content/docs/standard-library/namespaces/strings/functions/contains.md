---
title: contains
description: Test whether one bounded string view contains another bounded byte sequence.
domain: strings
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_strings_contains
activation_macros:
  - ZLANG_STD_STRINGS_CONTAINS
navigation:
  title: contains
---

# contains

## Summary

`contains` reports whether the bytes of one bounded string view appear inside another.

## Import

```zlang
import { contains } from "@std/strings";
```

## Signature

```zlang
function contains(clone string value, clone string part): boolean
```

Both parameters are passed by clone. Each argument is a view of bytes, not an owned buffer.

## Parameters

| Name | Meaning | Ownership |
| --- | --- | --- |
| `value` | Bounded string to search | clone |
| `part` | Bounded byte sequence to find | clone |

Each view is valid when its data pointer is non-null or its declared length is zero.

## Result

`contains` returns `boolean`. It returns `true` when the bytes of `part` occur in `value`, and `false` otherwise. An empty `part` returns `true`. A `part` longer than `value` returns `false`.

## Bounds and ownership

The wrapper tries each byte offset in `value` and compares at most `part.length_` bytes. It stops at the first match. Every read stays inside the declared byte lengths.

`contains` copies no bytes. It allocates no buffer and takes ownership of neither input. Both views remain valid after the call.

## Errors and traps

`ZLANG_TRAP_INVALID_ARGUMENT` fires when either view has a null data pointer and a nonzero declared length. `contains` returns no status value and raises no other trap.

## State and concurrency

The registry marks `contains` as not stateful and opaque. The wrapper is `static inline` and reads no mutable global. Repeated calls with the same valid views return the same result.

## Generated C

The import activates one macro:

```c
#define ZLANG_STD_STRINGS_CONTAINS
#include <zlang/runtime/runtime.h>
```

`runtime.h` then includes `zlang/standard/strings/strings.h`. For `contains("abc", "b")`, the emitted call is:

```c
zlang_std_io_write_bool(zlang_std_strings_contains(zlang_gen_literal_0, zlang_gen_literal_1));
```

The registry symbol and macro match the emitted call.

## Example

```zlang
import { contains } from "@std/strings";
import { write } from "@std/io";

function main(): integer {
    write(contains("abc", "b"));
    write("\n");
    write(contains("abc", "z"));
    write("\n");
    return 0;
}
```

## See also

- [compare](/docs/standard-library/namespaces/strings/functions/compare)
- [starts_with](/docs/standard-library/namespaces/strings/functions/starts_with)
- [Functions catalog](/docs/standard-library/namespaces/strings/functions)
- [Generated C](/docs/standard-library/generated-c)
