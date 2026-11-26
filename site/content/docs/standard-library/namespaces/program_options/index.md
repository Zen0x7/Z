---
title: Overview
description: Read host process arguments and command line options.
domain: program_options
kind: domain-overview
status: implemented
navigation:
  title: Overview
---

# Program Options

`@std/program_options` reads host process arguments through the `arguments` value bound to `main`. It counts arguments, reads an argument by index, matches option strings and reads the value after an option.

Generated C builds a `zlang_arguments_t` view from `argc` and `argv`. Every function in this domain reads that view through a pointer. Returned strings point into host-owned `argv` memory.

## Import

```zlang
import { argument_count, argument_at, has_option, option_value, has_flag } from "@std/program_options";
```

## What it does

- `argument_count` returns the number of host arguments, including the program path.
- `argument_at` returns a bounded string for a zero-based argument index.
- `has_option` and `has_flag` report an exact byte match after the program path.
- `option_value` returns the host argument immediately after an exact option match.

No function in this domain parses `--name=value` syntax or allocates storage.

## Host arguments

`main` receives the view through its two implicit host parameters:

```zlang
function main(integer argc, arguments argv): integer {
    return 0;
}
```

Generated C wraps the host `argc` and `argv` in one `zlang_arguments_t` object and passes a pointer to the generated entry function. The functions in this domain read the wrapper, not the `argc` parameter.

Index `0` is the program path when the host supplies one. `argument_at(argv, 1U)` reads the first argument written after the program name.

## Generated C activation

The compiler defines one macro for each imported name. The generated C includes `<zlang/runtime/runtime.h>`, which includes `zlang/standard/program/options.h` when any program-options macro is set. The options header defines the count helper only when `ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT` is defined, while `argument_at`, `has_option`, `option_value` and `has_flag` call that helper. Import `argument_count` with any of those four names. The emitted program does not compile when one of the four is imported without `argument_count`.

Generated C wraps the host arguments for `main`. This excerpt comes from `tools/zc/tests/program_options.z`:

```c
int32_t app_fn_main(int32_t app_param_argc, zlang_arguments_t *app_param_argv);

int32_t app_fn_main(int32_t app_param_argc, zlang_arguments_t *app_param_argv) {
    (void)app_param_argc;
    ...
}

int main(int argc, char **argv) {
    zlang_arguments_t _zlang_arguments = { argc, argv };
    return (int)app_fn_main(argc, &_zlang_arguments);
}
```

`app_fn_main` is the generated function for source `main`. The names `app_param_argc` and `app_param_argv` come from `main`'s `argc` and `arguments argv` parameters. The options helpers read the `zlang_arguments_t` view, not `app_param_argc`. The source string literals in the fixture become generated views named `zlang_gen_literal_N`; the function pages show their declarations.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/program_options) — domain model.
- [Functions](/docs/standard-library/namespaces/program_options/functions) — catalog and function reference.

## See also

- [Functions](/docs/standard-library/namespaces/program_options/functions)
- [Conventions](/docs/standard-library/conventions)
- [Generated C](/docs/standard-library/generated-c)
