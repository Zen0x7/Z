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

`@std/strings` compares and searches bounded strings. All three functions take non-owning byte views. They never allocate, grow storage, or transfer ownership.

## Import

```zlang
import { compare, contains, starts_with } from "@std/strings";
```

## What it does

- `compare` returns `-1`, `0` or `1` from a byte-wise runtime comparison.
- `contains` reports whether one bounded byte sequence appears inside another.
- `starts_with` reports whether one bounded byte sequence begins another.

A Zlang string is a pointer and a byte length. A string literal is interned in read-only storage. A declared string with explicit capacity points to caller-owned storage. Both pass to these functions as `zlang_string_t` views.

## Types

`@std/strings` introduces no dedicated struct. It uses Zlang's bounded string, whose runtime shape is `zlang_string_t` with `data_` and `length_` fields. The declared length bounds every read.

`runtime.h` includes `zlang/standard/strings/strings.h` when any of the three activation macros is defined. Each function is a `static inline` wrapper. No mutable global is introduced.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/strings) — domain model.
- [Functions](/docs/standard-library/namespaces/strings/functions) — catalog and function reference.

## See also

- [Conventions](/docs/standard-library/conventions)
- [Generated C](/docs/standard-library/generated-c)
