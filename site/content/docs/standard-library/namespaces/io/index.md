---
title: Overview
description: Bounded input and output for Zlang programs.
domain: io
kind: domain-overview
status: implemented
navigation:
  title: Overview
---

# Inputs and Outputs

`@std/io` gives programs a bounded way to read and write data. Output takes values. Input writes into caller-owned storage.

Nothing is buffered implicitly. A read needs a string with enough capacity, and that capacity is part of the program's fixed memory plan.

## Import

```zlang
import { read, write } from "@std/io";
```

## What it does

- `write` sends a bounded string or scalar to standard output.
- `read` reads input into a bounded string and keeps the newline.
- `read_line` reads one line and drops the newline.

Every input function writes into storage owned by the caller. Every output function takes a value by clone.

## Types

`@std/io` introduces no dedicated struct. It uses Zlang's bounded string, so the capacity belongs to the declaration in the program.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/io) — domain model.
- [Functions](/docs/standard-library/namespaces/io/functions) — catalog and function reference.

## See also

- [Conventions](/docs/standard-library/conventions)
- [Generated C](/docs/standard-library/generated-c)
