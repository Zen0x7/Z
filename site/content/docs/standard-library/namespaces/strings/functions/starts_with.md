---
title: starts_with
description: Test whether one bounded string view begins with another bounded byte sequence.
domain: strings
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_strings_starts_with
activation_macros:
  - ZLANG_STD_STRINGS_STARTS_WITH
navigation:
  title: starts_with
---

# starts_with

## Summary

`starts_with` reports whether one bounded byte sequence is the prefix of another, wrapped in `result_type<boolean, status_code>`.

## Import

```zlang
import { starts_with } from "@std/strings";
```

## Signature

```zlang
function starts_with(clone string value, clone string prefix): result_type<boolean, status_code>
```

Both parameters are passed by clone. Each argument is a view of bytes; cloning copies only the `zlang_string_t` descriptor, not the bytes.

## Parameters

| Name | Meaning | Ownership |
| --- | --- | --- |
| `value` | Bounded string to inspect | clone |
| `prefix` | Bounded byte sequence to match at the start | clone |

Each view is valid when its data pointer is non-null or its declared length is zero.

## Result

`value`/`get()` holds `true` when the first `prefix.length_` bytes of `value` match every byte of `prefix`, and `false` otherwise. An empty `prefix` returns `true`. A `prefix` longer than `value` returns `false`. `code` is a `status_code` set to `STATUS_SUCCESS` on every completed call, and `what` is a bounded read-only view of the fixed status message. The C helper returns `zlang_std_result_bool_status_t`, never a bare `bool`.

## Bounds and ownership

The wrapper compares bytes from index zero up to `prefix.length_`. Every read stays inside both declared byte lengths.

`starts_with` copies no bytes. It allocates no buffer and takes ownership of neither input; both views still point into caller-owned or host-owned storage after the call.

## Errors and traps

`ZLANG_TRAP_INVALID_ARGUMENT` fires when either view has a null data pointer and a nonzero declared length. A trapped call returns no result. There is no separate recoverable status: every completed call reports `STATUS_SUCCESS`.

## State and concurrency

The registry marks `starts_with` as not stateful and opaque. The wrapper is `static inline` and reads no mutable global. Repeated calls with the same valid views return the same result value and status.

## Generated C

The import activates one macro:

```c
#define ZLANG_STD_STRINGS_STARTS_WITH
#include <zlang/runtime/runtime.h>
```

`runtime.h` then includes `zlang/standard/strings/strings.h`. The wrapper returns the canonical result struct:

```c
ZLANG_STATIC_INLINE zlang_std_result_bool_status_t
zlang_std_strings_starts_with(zlang_string_t value, zlang_string_t prefix);
```

The emitter interns each distinct source string literal in first-use order. The two literals in `starts_with("abc", "ab")` become read-only views named `zlang_gen_literal_0` and `zlang_gen_literal_1`. This excerpt from the Example shows their declarations and the result assignment:

```c
#define ZLANG_LITERAL_0_LENGTH 3U
static const char zlang_gen_literal_0_data[] = "abc";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 2U
static const char zlang_gen_literal_1_data[] = "ab";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

zlang_std_result_bool_status_t app_local_matches = zlang_std_strings_starts_with(zlang_gen_literal_0, zlang_gen_literal_1);
```

## Example

```zlang
import { starts_with } from "@std/strings";
import { write } from "@std/io";

function main(): int32 {
    result_type<boolean, status_code> matches = starts_with("abc", "ab");
    write(matches.get());
    write("\n");
    result_type<boolean, status_code> other = starts_with("abc", "bc");
    write(other.value());
    write("\n");
    return 0;
}
```

## See also

- [compare](/docs/standard-library/namespaces/strings/functions/compare)
- [contains](/docs/standard-library/namespaces/strings/functions/contains)
- [Functions catalog](/docs/standard-library/namespaces/strings/functions)
- [Generated C](/docs/standard-library/generated-c)
