---
title: Overview
description: Catalog of functions exported by @std/strings.
domain: strings
kind: catalog
status: implemented
navigation:
  title: Overview
---

# Functions

`@std/strings` registers three functions. Each takes two bounded string views by clone and returns `result_type<T, status_code>`: `compare` wraps `int32`, while `contains` and `starts_with` wrap `boolean`. For every result, `value`/`get()` holds the ideal value, `code` is a `status_code`, and `what` is a bounded read-only view of the fixed status message.

## Catalog

| Name | Status | Signature | Parameter mode | C symbol | Macro |
| --- | --- | --- | --- | --- | --- |
| [`compare`](/docs/standard-library/namespaces/strings/functions/compare) | implemented | `function compare(clone string left, clone string right): result_type<int32, status_code>` | clone, clone | `zlang_std_strings_compare` | `ZLANG_STD_STRINGS_COMPARE` |
| [`contains`](/docs/standard-library/namespaces/strings/functions/contains) | implemented | `function contains(clone string value, clone string part): result_type<boolean, status_code>` | clone, clone | `zlang_std_strings_contains` | `ZLANG_STD_STRINGS_CONTAINS` |
| [`starts_with`](/docs/standard-library/namespaces/strings/functions/starts_with) | implemented | `function starts_with(clone string value, clone string prefix): result_type<boolean, status_code>` | clone, clone | `zlang_std_strings_starts_with` | `ZLANG_STD_STRINGS_STARTS_WITH` |

The registry lists no overloads in this domain. The emitted calls use exactly these symbols and macros and assign their canonical `zlang_std_result_*_status_t` return types. Cloning a string copies only its descriptor, not its bytes. None of these functions writes into caller storage.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/strings/functions) — this catalog.
- [compare](/docs/standard-library/namespaces/strings/functions/compare)
- [contains](/docs/standard-library/namespaces/strings/functions/contains)
- [starts_with](/docs/standard-library/namespaces/strings/functions/starts_with)
