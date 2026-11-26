---
title: compare
description: Compare two bounded string views byte by byte and return a three-way int32 value in result_type<int32, status_code>.
domain: strings
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_strings_compare
activation_macros:
  - ZLANG_STD_STRINGS_COMPARE
navigation:
  title: compare
---

# compare

## Summary

`compare` orders two bounded strings by their declared bytes through the runtime helper `zlang_string_equal_compare` and wraps the three-way result in `result_type<int32, status_code>`.

## Import

```zlang
import { compare } from "@std/strings";
```

## Signature

```zlang
function compare(clone string left, clone string right): result_type<int32, status_code>
```

Both parameters are passed by clone. A string argument is a view, so cloning it copies only the `zlang_string_t` descriptor (pointer and byte length), not the bytes.

This is an explicit function call. Ordering operators such as `<` and `>` remain rejected for strings by the language.

## Parameters

| Name | Meaning | Ownership |
| --- | --- | --- |
| `left` | First bounded string view | clone |
| `right` | Second bounded string view | clone |

Each view is valid when its data pointer is non-null or its declared length is zero.

## Result

`value`/`get()` holds `-1` when `left` is byte-wise smaller, `0` when both views have equal bytes and equal length, and `1` when `left` is byte-wise greater. `code` is a `status_code` set to `STATUS_SUCCESS` when the helper completes, and `what` is a bounded read-only view of the fixed status message. The C helper returns `zlang_std_result_i32_status_t`, never a bare `int32_t`.

## Bounds and ownership

`zlang_string_equal_compare` reads at most the smaller of the two declared byte lengths. It compares bytes as uint32 values. Equal prefixes are resolved by length, so `"a"` sorts before `"aa"`.

`compare` copies no bytes. It allocates no buffer and takes ownership of neither input; both views still point into caller-owned or host-owned storage and remain valid after the call.

## Errors and traps

`ZLANG_TRAP_INVALID_ARGUMENT` fires when either view has a null data pointer and a nonzero declared length. A trapped call returns no result, so `code` and `what` describe only completed calls. Every completed call reports `STATUS_SUCCESS`; there is no separate recoverable status.

## State and concurrency

The registry marks `compare` as not stateful and opaque. The wrapper is `static inline` and reads no mutable global. Repeated calls with the same valid views return the same result value and status.

## Generated C

The import activates one macro:

```c
#define ZLANG_STD_STRINGS_COMPARE
#include <zlang/runtime/runtime.h>
```

`runtime.h` then includes `zlang/standard/strings/strings.h`. The wrapper delegates to the runtime comparator and returns the canonical result struct:

```c
ZLANG_STATIC_INLINE zlang_std_result_i32_status_t zlang_std_strings_compare(zlang_string_t left,
                                                                            zlang_string_t right) {
    return zlang_std_result_i32(zlang_std_status_success_, zlang_string_equal_compare(left, right));
}
```

The emitter interns each distinct source string literal in first-use order. The two literals in `compare("abc", "abd")` become read-only views named `zlang_gen_literal_0` and `zlang_gen_literal_1`. This excerpt from the Example shows their declarations and the result assignment:

```c
#define ZLANG_LITERAL_0_LENGTH 3U
static const char zlang_gen_literal_0_data[] = "abc";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 3U
static const char zlang_gen_literal_1_data[] = "abd";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

zlang_std_result_i32_status_t app_local_smaller = zlang_std_strings_compare(zlang_gen_literal_0, zlang_gen_literal_1);
```

## Example

```zlang
import { compare } from "@std/strings";
import { write } from "@std/io";

function main(): int32 {
    result_type<int32, status_code> smaller = compare("abc", "abd");
    write(smaller.get());
    write("\n");
    result_type<int32, status_code> equal = compare("abc", "abc");
    write(equal.value());
    write("\n");
    return 0;
}
```

## See also

- [contains](/docs/standard-library/namespaces/strings/functions/contains)
- [starts_with](/docs/standard-library/namespaces/strings/functions/starts_with)
- [Functions catalog](/docs/standard-library/namespaces/strings/functions)
- [Generated C](/docs/standard-library/generated-c)
