---
title: has_option
description: Report whether host arguments contain an exact option string in result_type<boolean, status_code>.
domain: program_options
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_program_options_has_option
activation_macros:
  - ZLANG_STD_PROGRAM_OPTIONS_HAS_OPTION
---

# has_option

## Summary

`has_option` reports whether the host arguments contain an exact byte match for an option string in `result_type<boolean, status_code>`.

## Import

```zlang
import { argument_count, has_option } from "@std/program_options";
```

`argument_count` is in the import line because the generated header needs the count helper for this call. See Generated C.

## Signature

```zlang
function has_option(reference arguments argv, clone string option): result_type<boolean, status_code>
```

The registry records an `arguments` parameter in reference mode and a `string` parameter in clone mode. Cloning the option string copies only its bounded descriptor, not its bytes.

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `argv` | `arguments` | reference | Host argument view built by generated `main`. |
| `option` | `string` | clone | Bounded string compared byte for byte with each host argument. |

The scan starts at index `1`, so the program path is not compared. `--name=value` is one host argument and does not match the option `"--name"`.

## Result

`value`/`get()` holds `true` when an argument after the program path equals `option` byte for byte, and `false` otherwise. `code` is a `status_code` set to `STATUS_SUCCESS` on every completed call, and `what` is a bounded read-only view of the fixed status message. The C helper returns `zlang_std_result_bool_status_t`, not a bare `bool`.

## Bounds and ownership

- The option must be a valid bounded string; the header reads its bytes with `memcmp`.
- The host view and the option string stay owned by their callers; cloning the option copies only its descriptor.
- No storage is allocated and no argument bytes are changed.

## Errors and statuses

No match returns `false` in `value`/`get()` with `STATUS_SUCCESS`. The current `zlang_std_program_options_has_option` wrapper does not construct a `STATUS_INVALID_ARGUMENT` result and has no trap path. `STATUS_INVALID_ARGUMENT` is one of the `status_code` values carried by the canonical result type, but this helper does not return it: a caller must pass a valid bounded option view. Unlike `argument_at` and `option_value`, there is no unreachable status fallback here.

## State and concurrency

The registry marks `has_option` as not stateful and opaque. The function reads only its arguments and owns no mutable globals. Repeated calls with the same view and option return the same boolean value and the same `STATUS_SUCCESS` code.

## Generated C

Importing `argument_count` and `has_option` activates both macros:

```c
#define ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT
#define ZLANG_STD_PROGRAM_OPTIONS_HAS_OPTION
#include <zlang/runtime/runtime.h>
```

`<zlang/runtime/runtime.h>` includes `zlang/standard/program/options.h` when either macro is set. In that header, `zlang_std_program_options_has_option` calls the shared `zlang_std_program_options_has_bool` helper, which calls `zlang_std_program_options_argument_count`. Both wrappers return canonical result structs:

```c
ZLANG_STATIC_INLINE bool zlang_std_program_options_has_bool(const zlang_arguments_t *arguments,
                                                            zlang_string_t option);

ZLANG_STATIC_INLINE zlang_std_result_bool_status_t
zlang_std_program_options_has_option(const zlang_arguments_t *arguments, zlang_string_t option) {
    return zlang_std_result_bool(zlang_std_status_success_,
                                 zlang_std_program_options_has_bool(arguments, option));
}
```

The count helper is defined only under `ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT`. Importing `has_option` alone emits only `ZLANG_STD_PROGRAM_OPTIONS_HAS_OPTION`, and the generated C does not compile.

The generated function for source `main` names the `arguments` parameter `app_param_argv`. The emitter interns each distinct source string literal in first-use order; the current fixture makes `zlang_gen_literal_0` the generated view for `"--name"` in `tools/zc/tests/program_options.z`. This excerpt shows its declaration and the call:

```c
#define ZLANG_LITERAL_0_LENGTH 6U
static const char zlang_gen_literal_0_data[] = "--name";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };

int32_t app_fn_main(int32_t app_param_argc, zlang_arguments_t *app_param_argv);
...
zlang_std_result_bool_status_t app_local__has = zlang_std_program_options_has_option(app_param_argv, zlang_gen_literal_0);
```


## Example

```zlang
import { write } from "@std/io";
import { argument_count, has_option } from "@std/program_options";

function main(int32 argc, arguments argv): int32 {
    result_type<boolean, status_code> found = has_option(argv, "--verbose");
    write(found.get());
    write("\n");
    return 0;
}
```

## See also

- [has_flag](/docs/standard-library/namespaces/program_options/functions/has_flag)
- [option_value](/docs/standard-library/namespaces/program_options/functions/option_value)
- [Functions catalog](/docs/standard-library/namespaces/program_options/functions)
