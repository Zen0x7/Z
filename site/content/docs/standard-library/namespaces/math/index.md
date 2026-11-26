---
title: Overview
description: Scalar numeric functions and their overloads in @std/math.
domain: math
kind: domain-overview
status: implemented
navigation:
  title: Overview
---

# Mathematics

`@std/math` computes scalar numeric results from scalar inputs. Every overload takes its arguments by clone and returns `result_type<T, status_code>`, where `T` is the same numeric type as the computed value. Read the numeric result through `value`/`get()`; `code` is a `status_code` that is `STATUS_SUCCESS` for valid input, and `message` is a bounded read-only view of the fixed status message. The functions do not allocate, buffer, or write into caller-owned storage.

## Import

```zlang
import { abs, clamp, min, max, sqrt } from "@std/math";
```

Import the names you call. The compiler resolves each call to an overload from the exact argument types.

## What it does

- `abs` returns the magnitude of one value.
- `clamp` confines a value to an inclusive range.
- `min` and `max` select one of two values.
- `sqrt` applies the square-root helper to finite, non-negative input. Its result value is unchanged for inputs up to one; see the function page for the exact range behavior.

Each name has overloads for `int32`, `uint32`, `int64` and `uint64`. `abs`, `clamp`, `min` and `max` also have `float` and `double` overloads. `sqrt` has `float` and `double` overloads only. Zlang has no implicit conversion between numeric types, so all arguments of a call must match the selected overload.

Integer literals without a suffix have type `int32`. A `u` suffix selects `uint32`; `i64` and `u64` select the 64-bit types. A floating literal has type `double`; an `f` suffix selects `float`.

## Types

`@std/math` introduces no dedicated type. It operates on the six numeric primitives, and every overload wraps its numeric result in `result_type<T, status_code>`. `float` is IEEE-754 binary32 and `double` is IEEE-754 binary64.

Valid inputs yield `STATUS_SUCCESS`. Signed `abs` traps on the minimum value of its type. `clamp` traps when its lower bound exceeds its upper bound. The `float` and `double` overloads of `clamp` and `sqrt` also trap on non-finite input. Each trap uses a fixed runtime code and emits no message; no additional recoverable status is defined.

## State and activation

The registry marks every math entry as not stateful and opaque. The emitted calls pass values directly and return canonical result structs; no hidden context argument is added. The standard headers own no mutable globals, so repeated calls are independent.

The registry records one activation macro per overload. The emitter also activates the first overload registered under an imported name at the import statement. The `sqrt` and `abs` pages describe the resulting macro behavior.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/math) — domain model.
- [Functions](/docs/standard-library/namespaces/math/functions) — catalog and function reference.

## See also

- [Conventions](/docs/standard-library/conventions)
- [Generated C](/docs/standard-library/generated-c)
