---
title: option_value
description: Return the host argument that follows an exact option match.
domain: program_options
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_program_options_option_value
activation_macros:
  - ZLANG_STD_PROGRAM_OPTIONS_OPTION_VALUE
---

# option_value

## Summary

`option_value` returns the host argument immediately after the first exact option match.

## Import

```zlang
import { argument_count, option_value } from "@std/program_options";
```

`argument_count` is in the import line because the generated header needs the count helper for this call. See Generated C.

## Signature

```zlang
function option_value(reference arguments argv, clone string option): string
```

The registry records an `arguments` parameter in reference mode and a `string` parameter in clone mode.

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `argv` | `arguments` | reference | Host argument view built by generated `main`. |
| `option` | `string` | clone | Bounded string compared byte for byte with each host argument. |

The scan starts at index `1` and checks pairs of arguments. A matching option must have an argument after it. The value is returned verbatim; `--name=value` does not match `"--name"`, and a following argument such as `--verbose` is returned as the value.

## Result

Returns `string`, emitted as `zlang_string_t`. The view points to the host's `argv` entry after the matched option and records its byte length. No copy is made.

## Bounds and ownership

- The option must be a valid bounded string; the header reads its bytes with `memcmp`.
- The result borrows host-owned memory and stays valid for the lifetime of the host arguments.
- The function does not allocate and does not mutate the view or the argument bytes.
- No bounded string capacity is declared for the result.

## Errors and traps

A missing match traps with `ZLANG_TRAP_INVALID_ARGUMENT` (code `4`). A matching option in the final argument position also traps, because no value follows it. The empty string after the trap call is unreachable.

## State and concurrency

The registry marks `option_value` as not stateful and opaque. The function reads only its arguments and owns no mutable globals. The same view and option produce the same string or the same trap.

## Generated C

Importing `argument_count` and `option_value` activates both macros:

```c
#define ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT
#define ZLANG_STD_PROGRAM_OPTIONS_OPTION_VALUE
#include <zlang/runtime/runtime.h>
```

`<zlang/runtime/runtime.h>` includes `zlang/standard/program/options.h` when either macro is set. The options header calls `zlang_std_program_options_argument_count` from `zlang_std_program_options_option_value`, but defines that helper only under `ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT`. Importing `option_value` alone emits only `ZLANG_STD_PROGRAM_OPTIONS_OPTION_VALUE`, and the generated C does not compile.

In the five-function fixture, the generated call is:

```c
zlang_std_io_write_string(zlang_std_program_options_option_value(app_param_argv, zlang_gen_literal_1));
```

## Example

```zlang
import { write } from "@std/io";
import { argument_count, option_value } from "@std/program_options";

function main(integer argc, arguments argv): integer {
    write(option_value(argv, "--name"));
    write("\n");
    return 0;
}
```

Run with `--name beta` after the program path; the call returns `"beta"`.

## See also

- [has_option](/docs/standard-library/namespaces/program_options/functions/has_option)
- [argument_at](/docs/standard-library/namespaces/program_options/functions/argument_at)
- [Functions catalog](/docs/standard-library/namespaces/program_options/functions)
