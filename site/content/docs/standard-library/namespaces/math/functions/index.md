---
title: Overview
description: Catalog of functions exported by @std/math.
domain: math
kind: catalog
status: implemented
navigation:
  title: Overview
---

# Functions

`@std/math` registers 26 overloads under five names. The compiler picks the overload from the argument types. Every overload takes its arguments by clone and returns `result_type<T, status_code>`, where `T` is the same numeric type as the computed value. Parameter names come from the C helper signatures in `include/zlang/standard/math/math.h`; the registry records types, modes, symbols and activation macros.

## Catalog

| Name | Signature | Parameter mode | C symbol | Macro |
| --- | --- | --- | --- | --- |
| [`abs`](/docs/standard-library/namespaces/math/functions/abs) | `function abs(clone int32 value): result_type<int32, status_code>` | clone | `zlang_std_math_abs_i32` | `ZLANG_STD_MATH_ABS_I32` |
| `abs` | `function abs(clone uint32 value): result_type<uint32, status_code>` | clone | `zlang_std_math_abs_u32` | `ZLANG_STD_MATH_ABS_U32` |
| `abs` | `function abs(clone int64 value): result_type<int64, status_code>` | clone | `zlang_std_math_abs_i64` | `ZLANG_STD_MATH_ABS_I64` |
| `abs` | `function abs(clone uint64 value): result_type<uint64, status_code>` | clone | `zlang_std_math_abs_u64` | `ZLANG_STD_MATH_ABS_U64` |
| `abs` | `function abs(clone float value): result_type<float, status_code>` | clone | `zlang_std_math_abs_f32` | `ZLANG_STD_MATH_ABS_F32` |
| `abs` | `function abs(clone double value): result_type<double, status_code>` | clone | `zlang_std_math_abs_f64` | `ZLANG_STD_MATH_ABS_F64` |
| [`clamp`](/docs/standard-library/namespaces/math/functions/clamp) | `function clamp(clone int32 value, clone int32 low, clone int32 high): result_type<int32, status_code>` | clone | `zlang_std_math_clamp_i32` | `ZLANG_STD_MATH_CLAMP_I32` |
| `clamp` | `function clamp(clone uint32 value, clone uint32 low, clone uint32 high): result_type<uint32, status_code>` | clone | `zlang_std_math_clamp_u32` | `ZLANG_STD_MATH_CLAMP_U32` |
| `clamp` | `function clamp(clone int64 value, clone int64 low, clone int64 high): result_type<int64, status_code>` | clone | `zlang_std_math_clamp_i64` | `ZLANG_STD_MATH_CLAMP_I64` |
| `clamp` | `function clamp(clone uint64 value, clone uint64 low, clone uint64 high): result_type<uint64, status_code>` | clone | `zlang_std_math_clamp_u64` | `ZLANG_STD_MATH_CLAMP_U64` |
| `clamp` | `function clamp(clone float value, clone float low, clone float high): result_type<float, status_code>` | clone | `zlang_std_math_clamp_f32` | `ZLANG_STD_MATH_CLAMP_F32` |
| `clamp` | `function clamp(clone double value, clone double low, clone double high): result_type<double, status_code>` | clone | `zlang_std_math_clamp_f64` | `ZLANG_STD_MATH_CLAMP_F64` |
| [`min`](/docs/standard-library/namespaces/math/functions/min) | `function min(clone int32 left, clone int32 right): result_type<int32, status_code>` | clone | `zlang_std_math_min_i32` | `ZLANG_STD_MATH_MIN_I32` |
| `min` | `function min(clone uint32 left, clone uint32 right): result_type<uint32, status_code>` | clone | `zlang_std_math_min_u32` | `ZLANG_STD_MATH_MIN_U32` |
| `min` | `function min(clone int64 left, clone int64 right): result_type<int64, status_code>` | clone | `zlang_std_math_min_i64` | `ZLANG_STD_MATH_MIN_I64` |
| `min` | `function min(clone uint64 left, clone uint64 right): result_type<uint64, status_code>` | clone | `zlang_std_math_min_u64` | `ZLANG_STD_MATH_MIN_U64` |
| `min` | `function min(clone float left, clone float right): result_type<float, status_code>` | clone | `zlang_std_math_min_f32` | `ZLANG_STD_MATH_MIN_F32` |
| `min` | `function min(clone double left, clone double right): result_type<double, status_code>` | clone | `zlang_std_math_min_f64` | `ZLANG_STD_MATH_MIN_F64` |
| [`max`](/docs/standard-library/namespaces/math/functions/max) | `function max(clone int32 left, clone int32 right): result_type<int32, status_code>` | clone | `zlang_std_math_max_i32` | `ZLANG_STD_MATH_MAX_I32` |
| `max` | `function max(clone uint32 left, clone uint32 right): result_type<uint32, status_code>` | clone | `zlang_std_math_max_u32` | `ZLANG_STD_MATH_MAX_U32` |
| `max` | `function max(clone int64 left, clone int64 right): result_type<int64, status_code>` | clone | `zlang_std_math_max_i64` | `ZLANG_STD_MATH_MAX_I64` |
| `max` | `function max(clone uint64 left, clone uint64 right): result_type<uint64, status_code>` | clone | `zlang_std_math_max_u64` | `ZLANG_STD_MATH_MAX_U64` |
| `max` | `function max(clone float left, clone float right): result_type<float, status_code>` | clone | `zlang_std_math_max_f32` | `ZLANG_STD_MATH_MAX_F32` |
| `max` | `function max(clone double left, clone double right): result_type<double, status_code>` | clone | `zlang_std_math_max_f64` | `ZLANG_STD_MATH_MAX_F64` |
| [`sqrt`](/docs/standard-library/namespaces/math/functions/sqrt) | `function sqrt(clone float value): result_type<float, status_code>` | clone | `zlang_std_math_sqrt_f32` | `ZLANG_STD_MATH_SQRT_F32` |
| `sqrt` | `function sqrt(clone double value): result_type<double, status_code>` | clone | `zlang_std_math_sqrt_f64` | `ZLANG_STD_MATH_SQRT_F64` |

For each result, `value`/`get()` holds the numeric value, `code` is a `status_code` set to `STATUS_SUCCESS` for valid input, and `message` is a bounded read-only view of the fixed status message. There are no other recoverable statuses: programmer and resource errors trap.

## Activation

The registry defines one activation macro per overload. At an import statement the emitter activates the macro of the first registered overload with that name, then activates the resolved overload's macro at each call. Importing `sqrt` therefore emits `ZLANG_STD_MATH_SQRT_F32` even when the program calls only the `double` overload. The generated call still uses the resolved symbol. This is broader than one macro per resolved call. The [abs](/docs/standard-library/namespaces/math/functions/abs) page records a second mismatch between the header guard and the registry macros.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/math/functions) — this catalog.
- [abs](/docs/standard-library/namespaces/math/functions/abs)
- [clamp](/docs/standard-library/namespaces/math/functions/clamp)
- [min](/docs/standard-library/namespaces/math/functions/min)
- [max](/docs/standard-library/namespaces/math/functions/max)
- [sqrt](/docs/standard-library/namespaces/math/functions/sqrt)
