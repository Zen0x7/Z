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

`@std/strings` registers three functions. Each takes two bounded string views by clone. `compare` returns `int32`; `contains` and `starts_with` return `boolean`.

## Catalog

| Name | Status | Signature | Parameter mode | C symbol | Macro |
| --- | --- | --- | --- | --- | --- |
| [`compare`](/docs/standard-library/namespaces/strings/functions/compare) | implemented | `function compare(clone string left, clone string right): int32` | clone, clone | `zlang_std_strings_compare` | `ZLANG_STD_STRINGS_COMPARE` |
| [`contains`](/docs/standard-library/namespaces/strings/functions/contains) | implemented | `function contains(clone string value, clone string part): boolean` | clone, clone | `zlang_std_strings_contains` | `ZLANG_STD_STRINGS_CONTAINS` |
| [`starts_with`](/docs/standard-library/namespaces/strings/functions/starts_with) | implemented | `function starts_with(clone string value, clone string prefix): boolean` | clone, clone | `zlang_std_strings_starts_with` | `ZLANG_STD_STRINGS_STARTS_WITH` |

The registry lists no overloads in this domain. The emitted calls use exactly these symbols and macros. None of these functions writes into caller storage.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/strings/functions) — this catalog.
- [compare](/docs/standard-library/namespaces/strings/functions/compare)
- [contains](/docs/standard-library/namespaces/strings/functions/contains)
- [starts_with](/docs/standard-library/namespaces/strings/functions/starts_with)
