---
title: argument_at
description: Return a bounded string view for a host argument index in result_type<string, status_code>.
domain: program_options
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_program_options_argument_at
activation_macros:
  - ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_AT
---

# argument_at

## Summary

`argument_at` returns a bounded string view for one host argument index in `result_type<string, status_code>`.

## Import

```zlang
import { argument_count, argument_at } from "@std/program_options";
```

`argument_count` is in the import line because the generated header needs the count helper for this call. See Generated C.

## Signature

```zlang
function argument_at(reference arguments argv, clone uint32 index): result_type<string, status_code>
```

The registry records an `arguments` parameter in reference mode and an `uint32` parameter in clone mode.

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `argv` | `arguments` | reference | Host argument view built by generated `main`. |
| `index` | `uint32` | clone | Zero-based host argument index. |

Index `0` is the program path when the host supplies one. The first argument after the program name has index `1`.

## Result

`value`/`get()` holds a bounded string view pointing to the host's `argv[index]` C string with its byte length recorded. No copy is made; cloning a `string` argument elsewhere copies only its descriptor, not bytes. `code` is a `status_code` set to `STATUS_SUCCESS` for a valid index, and `what` is a bounded read-only view of the fixed status message. The C helper returns `zlang_std_result_string_status_t`, not a bare `zlang_string_t`.

## Bounds and ownership

- `index` must be less than `argument_count(argv)`.
- The result borrows host-owned memory. It stays valid for the lifetime of the host arguments.
- The function does not allocate and does not mutate the view or the argument bytes.
- No bounded string capacity is declared for the result.

## Errors and statuses

An index greater than or equal to the argument count takes the header's failure branch. That branch calls `zlang_trap(ZLANG_TRAP_BOUNDS)` (code `2`) and then returns `zlang_std_result_string(zlang_std_status_invalid_argument_, (zlang_string_t){"", 0U})`. `STATUS_INVALID_ARGUMENT` with an empty string view is the status the current header carries on that path; the `what` field is the bounded read-only view of its fixed message. A generated program terminates at the trap, so the returned status is not a recoverable branch to rely on.

## State and concurrency

The registry marks `argument_at` as not stateful and opaque. The function reads only its two arguments and owns no mutable globals. The same view and index produce the same result value and status, or the same trap.

## Generated C

Importing `argument_count` and `argument_at` activates both macros:

```c
#define ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT
#define ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_AT
#include <zlang/runtime/runtime.h>
```

`<zlang/runtime/runtime.h>` includes `zlang/standard/program/options.h` when either macro is set. The wrapper returns the canonical result struct:

```c
ZLANG_STATIC_INLINE zlang_std_result_string_status_t
zlang_std_program_options_argument_at(const zlang_arguments_t *arguments, uint32_t index) {
    const uint32_t _count_k = zlang_std_program_options_argument_count(arguments).value_;
    if (index >= _count_k) {
        zlang_trap(ZLANG_TRAP_BOUNDS);
        return zlang_std_result_string(zlang_std_status_invalid_argument_,
                                       (zlang_string_t){"", 0U});
    }
    const char *const _value_k = arguments->argv_[index];
    return zlang_std_result_string(zlang_std_status_success_,
                                   (zlang_string_t){_value_k, (uint32_t)strlen(_value_k)});
}
```

The options header calls `zlang_std_program_options_argument_count` from `zlang_std_program_options_argument_at`, but defines that helper only under `ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT`. Importing `argument_at` alone emits only `ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_AT`, and the generated C does not compile.

This excerpt from `tools/zc/tests/program_options.z` shows the generated function declaration and call:

```c
int32_t app_fn_main(int32_t app_param_argc, zlang_arguments_t *app_param_argv);
...
zlang_std_result_string_status_t app_local__arg = zlang_std_program_options_argument_at(app_param_argv, 1U);
```


## Example

```zlang
import { write } from "@std/io";
import { argument_count, argument_at } from "@std/program_options";

function main(int32 argc, arguments argv): int32 {
    result_type<string, status_code> argument = argument_at(argv, 1U);
    write(argument.get());
    write("\n");
    return 0;
}
```

Run with one host argument after the program path; index `1` reads that argument.

## See also

- [argument_count](/docs/standard-library/namespaces/program_options/functions/argument_count)
- [option_value](/docs/standard-library/namespaces/program_options/functions/option_value)
- [Functions catalog](/docs/standard-library/namespaces/program_options/functions)
