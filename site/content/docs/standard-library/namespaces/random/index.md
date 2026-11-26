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

`@std/random` generates numeric values inside caller-specified bounds. Each generator takes two values of one numeric type and returns one value of that type.

The registry marks every generator as stateful and opaque. Calls advance one runtime context shared by the generated program. The compiler passes that context explicitly. Generated `main` initializes it from a constant; Zlang source never names the context, and no import seeds or resets it. The helpers do not read an OS entropy source. Do not rely on a specific value or sequence. No security guarantee is part of this API.

## Import

```zlang
import { generate_int32, generate_float64 } from "@std/random";
```

## What it does

- `generate_int32` returns an `int32` in the inclusive range from `from` to `to`.
- `generate_uint32` returns a `uint32` in the inclusive range from `from` to `to`.
- `generate_int64` returns an `int64` in the inclusive range from `from` to `to`.
- `generate_uint64` returns a `uint64` in the inclusive range from `from` to `to`.
- `generate_float32` returns a `float` from the range before floating-point rounding.
- `generate_float64` returns a `double` from the range before floating-point rounding.

An inverted range traps with invalid argument. Non-finite float bounds also trap with invalid argument.

## Types

`@std/random` introduces no Zlang type. The runtime context is a generated C detail. The generators work with the six numeric types already in the language.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/random) — domain model.
- [Functions](/docs/standard-library/namespaces/random/functions) — catalog and function reference.

## See also

- [Conventions](/docs/standard-library/conventions)
- [Generated C](/docs/standard-library/generated-c)
