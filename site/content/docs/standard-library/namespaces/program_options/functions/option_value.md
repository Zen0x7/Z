---
title: option_value
description: Return the host argument that follows an exact option match in result_type<string, status_code>.
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

`option_value` returns the host argument immediately after the first exact option match in `result_type<string, status_code>`.

## Import

```zlang
import { argument_count, option_value } from "@std/program_options";
```

`argument_count` is in the import line because the generated header needs the count helper for this call. See Generated C.

## Signature

```zlang
function option_value(reference arguments argv, clone string option): result_type<string, status_code>
```

The registry records an `arguments` parameter in reference mode and a `string` parameter in clone mode. Cloning the string copies only its bounded descriptor, not its bytes.

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `argv` | `arguments` | reference | Host argument view built by generated `main`. |
| `option` | `string` | clone | Bounded string compared byte for byte with each host argument. |

The scan starts at index `1` and checks pairs of arguments. A matching option must have an argument after it. The value is returned verbatim; `--name=value` does not match `"--name"`, and a following argument such as `--verbose` is returned as the value.

## Result

`value`/`get()` holds a bounded string view pointing to the host's `argv` entry after the matched option, with its byte length recorded. No copy is made. `code` is a `status_code` set to `STATUS_SUCCESS` when an option has a following value, and `what` is a bounded read-only view of the fixed status message. The C helper returns `zlang_std_result_string_status_t`, not a bare `zlang_string_t`.

## Bounds and ownership

- The option must be a valid bounded string; the header reads its bytes with `memcmp`.
- The result borrows host-owned memory and stays valid for the lifetime of the host arguments.
- The function does not allocate and does not mutate the view or the argument bytes.
- No bounded string capacity is declared for the result.

## Errors and statuses

A missing match, including a matching option in the final argument position, takes the header's failure branch. That branch calls `zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT)` (code `4`) and then returns `zlang_std_result_string(zlang_std_status_invalid_argument_, (zlang_string_t){"", 0U})`. `STATUS_INVALID_ARGUMENT` with an empty string view is the status the current header carries on that path; `what` is the bounded read-only view of its fixed message. A generated program terminates at the trap, so the returned status is not a recoverable branch to rely on.

## State and concurrency

The registry marks `option_value` as not stateful and opaque. The function reads only its arguments and owns no mutable globals. The same view and option produce the same result value and status, or the same trap.

## Generated C

Importing `argument_count` and `option_value` activates both macros:

```c
#define ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT
#define ZLANG_STD_PROGRAM_OPTIONS_OPTION_VALUE
#include <zlang/runtime/runtime.h>
```

`<zlang/runtime/runtime.h>` includes `zlang/standard/program/options.h` when either macro is set. The wrapper returns the canonical result struct:

```c
ZLANG_STATIC_INLINE zlang_std_result_string_status_t
zlang_std_program_options_option_value(const zlang_arguments_t *arguments, zlang_string_t option) {
    const uint32_t _count_k = zlang_std_program_options_argument_count(arguments).value_;
    for (uint32_t _index_k = 1U; _index_k + 1U < _count_k; ++_index_k) {
        const char *const _value_k = arguments->argv_[_index_k];
        if (strlen(_value_k) == option.length_ &&
            memcmp(_value_k, option.data_, option.length_) == 0) {
            const char *const _result_k = arguments->argv_[_index_k + 1U];
            return zlang_std_result_string(
                zlang_std_status_success_,
                (zlang_string_t){_result_k, (uint32_t)strlen(_result_k)});
        }
    }
    zlang_trap(ZLANG_TRAP_INVALID_ARGUMENT);
    return zlang_std_result_string(zlang_std_status_invalid_argument_, (zlang_string_t){"", 0U});
}
```

The options header calls `zlang_std_program_options_argument_count` from `zlang_std_program_options_option_value`, but defines that helper only under `ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT`. Importing `option_value` alone emits only `ZLANG_STD_PROGRAM_OPTIONS_OPTION_VALUE`, and the generated C does not compile.

The generated function for source `main` names the `arguments` parameter `app_param_argv`. The emitter interns each distinct source string literal in first-use order; the current fixture makes `zlang_gen_literal_0` the generated view for `"--name"` in `tools/zc/tests/program_options.z`. This excerpt shows its declaration and the call:

```c
#define ZLANG_LITERAL_0_LENGTH 6U
static const char zlang_gen_literal_0_data[] = "--name";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };

int32_t app_fn_main(int32_t app_param_argc, zlang_arguments_t *app_param_argv);
...
zlang_std_result_string_status_t app_local__value = zlang_std_program_options_option_value(app_param_argv, zlang_gen_literal_0);
```


## Example

```zlang
import { write } from "@std/io";
import { argument_count, option_value } from "@std/program_options";

function main(int32 argc, arguments argv): int32 {
    result_type<string, status_code> value = option_value(argv, "--name");
    write(value.get());
    write("\n");
    return 0;
}
```

Run with `--name beta` after the program path; the call returns `"beta"` in `value`/`get()`.

## See also

- [has_option](/docs/standard-library/namespaces/program_options/functions/has_option)
- [argument_at](/docs/standard-library/namespaces/program_options/functions/argument_at)
- [Functions catalog](/docs/standard-library/namespaces/program_options/functions)
