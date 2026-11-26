---
title: Overview
description: Bounded random value generation for Zlang programs.
domain: random
kind: domain-overview
status: implemented
navigation:
  title: Overview
---

# Randomness

`@std/random` generates numeric values inside caller-specified bounds. Each generator takes two clone bounds of one numeric type and returns `result_type<T, status_code>`, where `T` is the generated numeric type. `value`/`get()` holds the generated value, `code` is a `status_code` set to `STATUS_SUCCESS` for valid bounds, and `what` is a bounded read-only view of the fixed status message.

The registry marks every generator as stateful and opaque. Calls advance one runtime context shared by the generated program. The compiler passes that context explicitly. Generated `main` initializes it from a constant; Zlang source never names the context, and no import seeds or resets it. The helpers do not read an OS entropy source. Do not rely on a specific value or sequence. No security guarantee is part of this API.

## Import

```zlang
import { generate_int32, generate_float64 } from "@std/random";
```

## What it does

- `generate_int32` returns `result_type<int32, status_code>` with a generated `int32` in the inclusive range from `from` to `to` in `value`/`get()`.
- `generate_uint32` returns `result_type<uint32, status_code>` with a generated `uint32` in the inclusive range from `from` to `to` in `value`/`get()`.
- `generate_int64` returns `result_type<int64, status_code>` with a generated `int64` in the inclusive range from `from` to `to` in `value`/`get()`.
- `generate_uint64` returns `result_type<uint64, status_code>` with a generated `uint64` in the inclusive range from `from` to `to` in `value`/`get()`.
- `generate_float32` returns `result_type<float, status_code>` with a generated `float` from the range before floating-point rounding in `value`/`get()`.
- `generate_float64` returns `result_type<double, status_code>` with a generated `double` from the range before floating-point rounding in `value`/`get()`.

An inverted range traps with invalid argument and returns no result. Non-finite float bounds also trap with invalid argument. There are no other recoverable statuses: every returned `code` is `STATUS_SUCCESS`.

## Types

`@std/random` introduces no Zlang type of its own. The runtime context is a generated C detail. The generators work with the six numeric types already in the language, wrapping each generated value in `result_type<T, status_code>`.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/random) — domain model.
- [Functions](/docs/standard-library/namespaces/random/functions) — catalog and function reference.

## See also

- [Conventions](/docs/standard-library/conventions)
- [Generated C](/docs/standard-library/generated-c)
