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

`contains` reports whether the bytes of one bounded string view appear inside another, wrapped in `result_type<boolean, status_code>`.

## Import

```zlang
import { contains } from "@std/strings";
```

## Signature

```zlang
function contains(clone string value, clone string part): result_type<boolean, status_code>
```

Both parameters are passed by clone. Each argument is a view of bytes; cloning copies only the `zlang_string_t` descriptor, not the bytes.

## Parameters

| Name | Meaning | Ownership |
| --- | --- | --- |
| `value` | Bounded string to search | clone |
| `part` | Bounded byte sequence to find | clone |

Each view is valid when its data pointer is non-null or its declared length is zero.

## Result

`value`/`get()` holds `true` when the bytes of `part` occur in `value`, and `false` otherwise. An empty `part` returns `true`. A `part` longer than `value` returns `false`. `code` is a `status_code` set to `STATUS_SUCCESS` on every completed call, and `what` is a bounded read-only view of the fixed status message. The C helper returns `zlang_std_result_bool_status_t`, never a bare `bool`.

## Bounds and ownership

The wrapper tries each byte offset in `value` and compares at most `part.length_` bytes. It stops at the first match. Every read stays inside the declared byte lengths.

`contains` copies no bytes. It allocates no buffer and takes ownership of neither input; both views still point into caller-owned or host-owned storage after the call.

## Errors and traps

`ZLANG_TRAP_INVALID_ARGUMENT` fires when either view has a null data pointer and a nonzero declared length. A trapped call returns no result. There is no separate recoverable status: every completed call reports `STATUS_SUCCESS`.

## State and concurrency

The registry marks `contains` as not stateful and opaque. The wrapper is `static inline` and reads no mutable global. Repeated calls with the same valid views return the same result value and status.

## Generated C

The import activates one macro:

```c
#define ZLANG_STD_STRINGS_CONTAINS
#include <zlang/runtime/runtime.h>
```

`runtime.h` then includes `zlang/standard/strings/strings.h`. The wrapper returns the canonical result struct:

```c
ZLANG_STATIC_INLINE zlang_std_result_bool_status_t zlang_std_strings_contains(zlang_string_t value,
                                                                              zlang_string_t part);
```

The emitter interns each distinct source string literal in first-use order. The two literals in `contains("abc", "b")` become read-only views named `zlang_gen_literal_0` and `zlang_gen_literal_1`. This excerpt from the Example shows their declarations and the result assignment:

```c
#define ZLANG_LITERAL_0_LENGTH 3U
static const char zlang_gen_literal_0_data[] = "abc";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "b";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

zlang_std_result_bool_status_t app_local_found = zlang_std_strings_contains(zlang_gen_literal_0, zlang_gen_literal_1);
```

## Example

```zlang
import { contains } from "@std/strings";
import { write } from "@std/io";

function main(): int32 {
    result_type<boolean, status_code> found = contains("abc", "b");
    write(found.get());
    write("\n");
    result_type<boolean, status_code> missing = contains("abc", "z");
    write(missing.value());
    write("\n");
    return 0;
}
```

## See also

- [compare](/docs/standard-library/namespaces/strings/functions/compare)
- [starts_with](/docs/standard-library/namespaces/strings/functions/starts_with)
- [Functions catalog](/docs/standard-library/namespaces/strings/functions)
- [Generated C](/docs/standard-library/generated-c)
