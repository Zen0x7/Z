---
title: Overview
description: What Zlang is and what it guarantees.
navigation:
  title: Overview
---

# Overview

Zlang is a small source-to-source language. You write `.z` programs, and `zc` writes C99. Your normal C compiler builds the executable.

The language is for programs whose sizes are known before they run. An array has a fixed capacity. A string has a fixed capacity. Arithmetic and indexes are checked.

## What you get

- `zc`, the compiler.
- `libzlang`, a header-only runtime.
- C99 output you can read, compile and debug.

There is no VM, no bytecode and no runtime allocator hidden behind the source.

## What stays fixed

- Arrays cannot grow.
- Mutable strings cannot exceed their capacity. A plain `string` is a non-owning view over bytes.
- Numeric operations follow the rules of their declared type.
- Names are resolved during compilation.

These rules make the generated C predictable.

## Pages at this level

- [Overview](/docs/overview) — this page.
- [Pipeline](/docs/overview/pipeline) — the path from source to executable.
- [Boundaries](/docs/overview/boundaries) — what Zlang does and does not do.
