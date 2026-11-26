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

Each name below has overloads. `read(destination)` and `read_line(destination)` take a `reference string` destination: caller-owned mutable bounded storage. They read implicit `STDIN` from offset `0` up to the declared capacity. The explicit `read(stream, destination, offset, count)` and `read_line(stream, destination, offset, count)` forms take a `clone stream` source and read into a bounded offset/count window. A cloned `stream` copies a borrowed descriptor, not the device. `write(value)` uses implicit `STDOUT`; `write(stream, value)` writes to a borrowed stream handle.

Every overload returns `result_type<int32, status_code>`: `value` is the transferred byte count, `code` is the `status_code`, and `what` is a bounded read-only view of the fixed status message.

## Import

```zlang
import { read, write } from "@std/io";
```

## What it does

- `write` sends a bounded string or scalar to a stream, with implicit `STDOUT` in the one-value form.
- `read` reads input into a bounded string and keeps the newline when it fits the window.
- `read_line` stops at a newline and drops the delimiter.

Every input function writes into storage owned by the caller. Every output function takes a value by clone.

## Types

`@std/io` introduces no dedicated struct. It uses Zlang's bounded string, so the capacity belongs to the declaration in the program.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/io) — domain model.
- [Functions](/docs/standard-library/namespaces/io/functions) — catalog and function reference.

## See also

- [Conventions](/docs/standard-library/conventions)
- [Generated C](/docs/standard-library/generated-c)
