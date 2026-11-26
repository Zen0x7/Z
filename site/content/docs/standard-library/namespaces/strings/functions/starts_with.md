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

`starts_with` reports whether one bounded byte sequence is the prefix of another.

## Import

```zlang
import { starts_with } from "@std/strings";
```

## Signature

```zlang
function starts_with(clone string value, clone string prefix): boolean
```

Both parameters are passed by clone. Each argument is a view of bytes, not an owned buffer.

## Parameters

| Name | Meaning | Ownership |
| --- | --- | --- |
| `value` | Bounded string to inspect | clone |
| `prefix` | Bounded byte sequence to match at the start | clone |

Each view is valid when its data pointer is non-null or its declared length is zero.

## Result

`starts_with` returns `boolean`. It returns `true` when the first `prefix.length_` bytes of `value` match every byte of `prefix`. An empty `prefix` returns `true`. A `prefix` longer than `value` returns `false`.

## Bounds and ownership

The wrapper compares bytes from index zero up to `prefix.length_`. Every read stays inside both declared byte lengths.

`starts_with` copies no bytes. It allocates no buffer and takes ownership of neither input. Both views remain valid after the call.

## Errors and traps

`ZLANG_TRAP_INVALID_ARGUMENT` fires when either view has a null data pointer and a nonzero declared length. `starts_with` returns no status value and raises no other trap.

## State and concurrency

The registry marks `starts_with` as not stateful and opaque. The wrapper is `static inline` and reads no mutable global. Repeated calls with the same valid views return the same result.

## Generated C

The import activates one macro:

```c
#define ZLANG_STD_STRINGS_STARTS_WITH
#include <zlang/runtime/runtime.h>
```

`runtime.h` then includes `zlang/standard/strings/strings.h`. The emitter interns each distinct source string literal in first-use order. The two literals in `starts_with("abc", "ab")` become read-only views named `zlang_gen_literal_0` and `zlang_gen_literal_1`. This excerpt from the Example shows their declarations and call statement:

```c
#define ZLANG_LITERAL_0_LENGTH 3U
static const char zlang_gen_literal_0_data[] = "abc";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 2U
static const char zlang_gen_literal_1_data[] = "ab";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

zlang_std_strings_starts_with(zlang_gen_literal_0, zlang_gen_literal_1);
```


## Example

```zlang
import { starts_with } from "@std/strings";
import { write } from "@std/io";

function main(): integer {
    write(starts_with("abc", "ab"));
    write("\n");
    write(starts_with("abc", "bc"));
    write("\n");
    return 0;
}
```

## See also

- [compare](/docs/standard-library/namespaces/strings/functions/compare)
- [contains](/docs/standard-library/namespaces/strings/functions/contains)
- [Functions catalog](/docs/standard-library/namespaces/strings/functions)
- [Generated C](/docs/standard-library/generated-c)
