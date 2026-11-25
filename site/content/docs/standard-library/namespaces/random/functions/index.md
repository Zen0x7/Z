---
title: Overview
description: Catalog of functions exported by @std/random.
domain: random
kind: catalog
status: implemented
navigation:
  title: Overview
---

# Functions

`@std/random` registers six generators. Each name has one registered overload, and both arguments use the return type.

## Catalog

| Name | Signature | Parameter mode | C symbol | Macro |
| --- | --- | --- | --- | --- |
| [`generate_int32`](/docs/standard-library/namespaces/random/functions/generate_int32) | `function generate_int32(clone integer from, clone integer to): integer` | clone | `zlang_std_random_generate_int32` | `ZLANG_STD_RANDOM_GENERATE_INT32` |
| [`generate_uint32`](/docs/standard-library/namespaces/random/functions/generate_uint32) | `function generate_uint32(clone unsigned from, clone unsigned to): unsigned` | clone | `zlang_std_random_generate_uint32` | `ZLANG_STD_RANDOM_GENERATE_UINT32` |
| [`generate_int64`](/docs/standard-library/namespaces/random/functions/generate_int64) | `function generate_int64(clone int64 from, clone int64 to): int64` | clone | `zlang_std_random_generate_int64` | `ZLANG_STD_RANDOM_GENERATE_INT64` |
| [`generate_uint64`](/docs/standard-library/namespaces/random/functions/generate_uint64) | `function generate_uint64(clone uint64 from, clone uint64 to): uint64` | clone | `zlang_std_random_generate_uint64` | `ZLANG_STD_RANDOM_GENERATE_UINT64` |
| [`generate_float32`](/docs/standard-library/namespaces/random/functions/generate_float32) | `function generate_float32(clone float from, clone float to): float` | clone | `zlang_std_random_generate_float32` | `ZLANG_STD_RANDOM_GENERATE_FLOAT32` |
| [`generate_float64`](/docs/standard-library/namespaces/random/functions/generate_float64) | `function generate_float64(clone double from, clone double to): double` | clone | `zlang_std_random_generate_float64` | `ZLANG_STD_RANDOM_GENERATE_FLOAT64` |

The registry marks all six as stateful and opaque. Generated C passes one shared runtime context before the two Zlang arguments. No generator exposes a seed or a reset.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/random/functions) — this catalog.
- [generate_int32](/docs/standard-library/namespaces/random/functions/generate_int32)
- [generate_uint32](/docs/standard-library/namespaces/random/functions/generate_uint32)
- [generate_int64](/docs/standard-library/namespaces/random/functions/generate_int64)
- [generate_uint64](/docs/standard-library/namespaces/random/functions/generate_uint64)
- [generate_float32](/docs/standard-library/namespaces/random/functions/generate_float32)
- [generate_float64](/docs/standard-library/namespaces/random/functions/generate_float64)
