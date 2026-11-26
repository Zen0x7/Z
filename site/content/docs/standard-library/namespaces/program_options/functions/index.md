---
title: Overview
description: Catalog of functions exported by @std/program_options.
domain: program_options
kind: catalog
status: implemented
navigation:
  title: Overview
---

# Functions

`@std/program_options` registers five functions with one signature each. The registry stores parameter types and modes; the parameter names in this catalog are documentation labels.

## Catalog

| Name | Signature | Parameter mode | C symbol | Macro |
| --- | --- | --- | --- | --- |
| [`argument_count`](/docs/standard-library/namespaces/program_options/functions/argument_count) | `function argument_count(reference arguments argv): uint32` | reference | `zlang_std_program_options_argument_count` | `ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT` |
| [`argument_at`](/docs/standard-library/namespaces/program_options/functions/argument_at) | `function argument_at(reference arguments argv, clone uint32 index): string` | reference, clone | `zlang_std_program_options_argument_at` | `ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_AT` |
| [`has_option`](/docs/standard-library/namespaces/program_options/functions/has_option) | `function has_option(reference arguments argv, clone string option): boolean` | reference, clone | `zlang_std_program_options_has_option` | `ZLANG_STD_PROGRAM_OPTIONS_HAS_OPTION` |
| [`option_value`](/docs/standard-library/namespaces/program_options/functions/option_value) | `function option_value(reference arguments argv, clone string option): string` | reference, clone | `zlang_std_program_options_option_value` | `ZLANG_STD_PROGRAM_OPTIONS_OPTION_VALUE` |
| [`has_flag`](/docs/standard-library/namespaces/program_options/functions/has_flag) | `function has_flag(reference arguments argv, clone string flag): boolean` | reference, clone | `zlang_std_program_options_has_flag` | `ZLANG_STD_PROGRAM_OPTIONS_HAS_FLAG` |

The registry marks all five as not stateful and opaque. It records no activation dependency between them. The generated header calls the count helper from the other four functions but defines it only under `ZLANG_STD_PROGRAM_OPTIONS_ARGUMENT_COUNT`, so import `argument_count` with `argument_at`, `has_option`, `option_value` or `has_flag`.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/program_options/functions) — this catalog.
- [argument_count](/docs/standard-library/namespaces/program_options/functions/argument_count)
- [argument_at](/docs/standard-library/namespaces/program_options/functions/argument_at)
- [has_option](/docs/standard-library/namespaces/program_options/functions/has_option)
- [option_value](/docs/standard-library/namespaces/program_options/functions/option_value)
- [has_flag](/docs/standard-library/namespaces/program_options/functions/has_flag)

## See also

- [Program Options overview](/docs/standard-library/namespaces/program_options)
- [Generated C](/docs/standard-library/generated-c)
