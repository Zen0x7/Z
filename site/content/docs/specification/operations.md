---
title: Abstract operations
description: The checks that define bounds, arithmetic, capacity and iteration.
navigation:
  title: Abstract operations
---

# Abstract operations

These operations define meaning. A program cannot call them.

## CheckIndex

`CheckIndex(array, index)` traps with `ZLANG_TRAP_BOUNDS` when `index` is greater than or equal to the array capacity. Otherwise it returns the element at `index`. Indexed stores perform the same check before writing.

## CheckArithmetic

`CheckArithmetic(operation, left, right)` performs the operation with overflow detection. Signed overflow and negation of the signed minimum value trap with `ZLANG_TRAP_OVERFLOW`. Integer division by zero traps with `ZLANG_TRAP_DIVISION_BY_ZERO`. Floating arithmetic follows IEEE-754 and never traps on division by zero.

## CheckCapacity

`CheckCapacity(storage, capacity, needed)` traps with `ZLANG_TRAP_CAPACITY` when a bounded string concatenation would exceed `capacity`. It returns the storage to write into when the operation fits. Arrays are fixed at compile time and never grow.

## CheckResultContract

`CheckResultContract(result_type<T, status_code>)` requires `status_code` as the error type. For a user payload `T`, `T` must be a struct that implements `dump(): string`. Native payloads use the public layouts in `<zlang/standard/result/result.h>`.

## CheckLabel

`CheckLabel(label, jump)` requires a function-scoped label declaration before its `jump`, and requires a labelled loop for `break label;` and `continue label;`.

## Trap

`Trap(code)` terminates the process by calling `zlang_trap(code)`. The runtime must not construct a message, allocate memory or perform output. The codes are fixed:

| Code | Name |
| --- | --- |
| `1` | `ZLANG_TRAP_OVERFLOW` |
| `2` | `ZLANG_TRAP_BOUNDS` |
| `3` | `ZLANG_TRAP_CAPACITY` |
| `4` | `ZLANG_TRAP_INVALID_ARGUMENT` |
| `5` | `ZLANG_TRAP_DIVISION_BY_ZERO` |

## EmitForEach

`EmitForEach(node)` validates that the iterable is an array or exclusive `uint32` range and that each iteration variable has the required `uint32` index type or the array element type. It emits a native C `for` loop over the fixed extent or range. The loop must not become a runtime callback.
