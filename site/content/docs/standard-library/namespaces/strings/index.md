---
title: Overview
description: Bounded string comparison and search for Zlang programs.
domain: strings
kind: domain-overview
status: implemented
navigation:
  title: Overview
---

# Strings

`@std/strings` compares and searches bounded strings. All three functions take non-owning byte views by clone and return `result_type<T, status_code>`. Cloning a `string` copies only the `zlang_string_t` descriptor, not the bytes. The functions never allocate, grow storage, or transfer ownership. `value`/`get()` holds the ideal result value, `code` is a `status_code`, and `what` is a bounded read-only view of the fixed status message.

## Import

```zlang
import { compare, contains, starts_with } from "@std/strings";
```

## What it does

- `compare` returns `result_type<int32, status_code>` with `-1`, `0` or `1` in `value`/`get()` from a byte-wise runtime comparison.
- `contains` returns `result_type<boolean, status_code>` reporting whether one bounded byte sequence appears inside another.
- `starts_with` returns `result_type<boolean, status_code>` reporting whether one bounded byte sequence begins another.

A Zlang string is a pointer and a byte length. The emitter interns each distinct source string literal in first-use order as a read-only `zlang_string_t` named `zlang_gen_literal_N`. A declared string with explicit capacity points to caller-owned storage. Both pass to these functions as `zlang_string_t` views, and the C helpers return canonical `zlang_std_result_*_status_t` structs rather than bare scalars.

## Types

`@std/strings` introduces no dedicated struct. It wraps an `int32` or `boolean` payload in `result_type<T, status_code>` and reads bounded byte views through the runtime's `zlang_string_t` shape, with `data_` and `length_` fields. The declared length bounds every read.

`runtime.h` includes `zlang/standard/strings/strings.h` when any of the three activation macros is defined. Each function is a `static inline` wrapper that constructs its result with a `status_code` and a fixed message view. No mutable global is introduced.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/strings) — domain model.
- [Functions](/docs/standard-library/namespaces/strings/functions) — catalog and function reference.

## See also

- [Conventions](/docs/standard-library/conventions)
- [Generated C](/docs/standard-library/generated-c)
