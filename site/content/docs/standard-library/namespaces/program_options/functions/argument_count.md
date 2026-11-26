---
title: argument_count
description: Return the number of host arguments in the arguments view.
domain: program_options
kind: function
status: implemented
stateful: false
opaque: true
canonical_symbols:
  - zlang_std_program_options_argument_count
activation_macros:
  - ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT
---

# argument_count

## Summary

`argument_count` returns the number of host arguments available through an `arguments` value.

## Import

```zlang
import { argument_count } from "@std/program_options";
```

## Signature

```zlang
function argument_count(reference arguments argv): uint32
```

The registry records one parameter of type `arguments` in reference mode.

## Parameters

| Name | Type | Mode | Meaning |
| --- | --- | --- | --- |
| `argv` | `arguments` | reference | Host argument view built by generated `main` from `argc` and `argv`. |

Pass the `arguments` parameter of `main`. The `argc` parameter is not used by this function.

## Result

Returns `uint32`, emitted as `uint32_t`. The count includes the program path at index `0` when the host supplies one. In the `zc` fixture, four arguments after the program path produce `5`.

## Bounds and ownership

- The view is host-owned and read-only. The function does not mutate or own `argv` memory.
- No storage is allocated and no capacity is declared.
- A null view, a null `argv_` field or a negative `argc_` field yields `0`.

## Errors and traps

`argument_count` has no trap path. It validates the view fields and returns `0` when they are unusable.

## State and concurrency

The registry marks `argument_count` as not stateful and opaque. The function reads only the view passed to it. Standard headers own no mutable globals, so repeated calls with the same view return the same value.

## Generated C

Importing `argument_count` activates its macro. The generated prologue defines the macro before including `<zlang/runtime/runtime.h>`, which includes `zlang/standard/program/options.h` for any program-options import:

```c
#define ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT
#include <zlang/runtime/runtime.h>
```

The generated function for source `main` names the `arguments` parameter `app_param_argv` and passes it as `zlang_arguments_t *`. This excerpt comes from `tools/zc/tests/program_options.z`:

```c
int32_t app_fn_main(int32_t app_param_argc, zlang_arguments_t *app_param_argv);
...
zlang_std_program_options_argument_count(app_param_argv);
```


## Example

```zlang
import { write } from "@std/io";
import { argument_count } from "@std/program_options";

function main(int32 argc, arguments argv): int32 {
    write(argument_count(argv));
    write("\n");
    return 0;
}
```

## See also

- [argument_at](/docs/standard-library/namespaces/program_options/functions/argument_at)
- [Functions catalog](/docs/standard-library/namespaces/program_options/functions)
- [Generated C](/docs/standard-library/generated-c)
