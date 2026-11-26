---
title: Classification
description: The eight values recorded for every registered standard function.
kind: classification
status: implemented
---

# Classification

The reference uses eight axes to describe a standard function. The current registry records only state, opacity and parameter modes; the remaining values are documentation metadata and are being moved into the registry.

| Axis | Meaning | Values |
| --- | --- | --- |
| Lifecycle | How committed the signature and behavior are | `DRAFT`, `UNSTABLE`, `STABLE` |
| Safety | Whether the function hides state or unsafe resource behavior | `SAFE`, `UNSAFE` |
| Evaluation | Whether the result may be folded at compile time | `PURE`, `OPAQUE`, `STATEFUL`, `EFFECTFUL` |
| Determinism | Whether the same inputs and state produce the same result | `DETERMINISTIC`, `ENTROPY` |
| Resource | What the function returns or writes into | `VALUE`, `BUFFER`, `HANDLE` |
| Error | How invalid input is reported | `TRAP`, `STATUS`, `TOTAL` |
| Portability | Whether behavior is the same on every target | `PORTABLE`, `PLATFORM` |
| Optimizer | What the transpile-time optimizer may do | `FOLDABLE`, `BARRIER`, `NO_CSE` |

## Reading a function page

A function page shows these values as a badge row. When a value is not yet recorded in the compiler registry, the page says so instead of guessing one.

The registry currently records state, opacity and parameter modes. Full lifecycle and optimizer metadata is being moved into the registry. Function pages must follow the registry, not aspirational tables.

## Example

`random.generate_int32` is stateful and opaque. The compiler may fold its constant arguments, but it must not fold, cache or reorder the result.

`io.write` is effectful and deterministic for a given value. It writes to a borrowed stream and returns `Result<int32, Status>` with the number of bytes written. The [write reference](/docs/standard-library/namespaces/io/functions/write) records the canonical signatures.
