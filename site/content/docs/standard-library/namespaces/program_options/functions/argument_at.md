---
title: argument_at
description: Return a bounded string view for a host argument index.
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

`argument_at` returns a bounded string view for one host argument index.

## Import

```zlang
import { argument_count, argument_at } from "@std/program_options";
```

`argument_count` is in the import line because the generated header needs the count helper for this call. See Generated C.

## Signature

```zlang
function argument_at(reference arguments argv, clone unsigned index): string
```

The registry records an `arguments` parameter in reference mode and an `unsigned` parameter in clone mode.

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `argv` | `arguments` | reference | Host argument view built by generated `main`. |
| `index` | `unsigned` | clone | Zero-based host argument index. |

Index `0` is the program path when the host supplies one. The first argument after the program name has index `1`.

## Result

Returns `string`, emitted as `zlang_string_t`. The view points to the host's `argv[index]` C string and records its byte length. No copy is made.

## Bounds and ownership

- `index` must be less than `argument_count(argv)`.
- The result borrows host-owned memory. It stays valid for the lifetime of the host arguments.
- The function does not allocate and does not mutate the view or the argument bytes.
- No bounded string capacity is declared for the result.

## Errors and traps

An index greater than or equal to the argument count traps with `ZLANG_TRAP_BOUNDS` (code `2`). The empty string after the trap call is unreachable.

## State and concurrency

The registry marks `argument_at` as not stateful and opaque. The function reads only its two arguments and owns no mutable globals. The same view and index produce the same string or the same trap.

## Generated C

Importing `argument_count` and `argument_at` activates both macros:

```c
#define ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT
#define ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_AT
#include <zlang/runtime/runtime.h>
```

`<zlang/runtime/runtime.h>` includes `zlang/standard/program/options.h` when either macro is set. The options header calls `zlang_std_program_options_argument_count` from `zlang_std_program_options_argument_at`, but defines that helper only under `ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT`. Importing `argument_at` alone emits only `ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_AT`, and the generated C does not compile.

In the five-function fixture, the generated call is:

```c
zlang_std_io_write_string(zlang_std_program_options_argument_at(app_param_argv, 1U));
```

## Example

```zlang
import { write } from "@std/io";
import { argument_count, argument_at } from "@std/program_options";

function main(integer argc, arguments argv): integer {
    write(argument_at(argv, 1U));
    write("\n");
    return 0;
}
```

Run with one host argument after the program path; index `1` reads that argument.

## See also

- [argument_count](/docs/standard-library/namespaces/program_options/functions/argument_count)
- [option_value](/docs/standard-library/namespaces/program_options/functions/option_value)
- [Functions catalog](/docs/standard-library/namespaces/program_options/functions)
