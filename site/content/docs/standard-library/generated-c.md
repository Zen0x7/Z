---
title: Generated C
description: How imports become activation macros and C calls.
kind: generated-c
status: implemented
---

# Generated C

An import emits activation macros before the runtime include:

```c
#define ZLANG_STD_IO_WRITE_STRING
#include <zlang/runtime/runtime.h>
```

`runtime.h` includes the runtime primitives first. It then includes each standard header whose activation macro is defined. Standard functions are emitted as `static inline` wrappers. There is no separate declaration and implementation phase for standard functions.

## Activation today

The emitter activates the first registered overload for an imported name. At each call it also activates the resolved overload.

That means an import can activate more than the exact macro needed by the call. It also means the emitter does not activate transitive dependencies: the registry has no dependency field. If a standard header calls a helper guarded by another macro, the source must import that helper too. The `program_options` reference records the current gap.

No macro is emitted for a name that was never imported and never called.

## Provenance

| Prefix | Owner | Meaning |
| --- | --- | --- |
| `app_*` | user source | Functions, locals, parameters and declared types |
| `zlang_gen_*` | compiler | Support derived from source |
| `zlang_std_*` | standard library | Registered standard function |
| `zlang_*` | runtime | Public runtime API |
| `ZLANG_*` | language | Constants and macros |

A value bug belongs to the source string. A declaration or lifetime bug in `zlang_gen_*` belongs to the compiler.

## State

Standard headers own no mutable globals and no hidden mutable `static` state. A stateful function receives state explicitly. When the emitter must thread that state through user functions, it adds a hidden parameter to definitions, prototypes and call sites together. Zlang source signatures do not expose it.
