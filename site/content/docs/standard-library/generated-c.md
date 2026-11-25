---
title: Generated C
description: How imports become activation macros and runtime calls.
kind: generated-c
status: implemented
---

# Generated C

An import activates one macro per resolved function before the runtime include:

```c
#define ZLANG_STD_IO_WRITE_STRING
#include <zlang/runtime/runtime.h>
```

`runtime.h` includes standard headers in two phases. The declaration phase exposes prototypes. The implementation phase emits `static inline` bodies. Calls between standard functions do not depend on include order.

## Activation

Activation is transitive and comes from registry metadata, not import order. If an imported function depends on another, both macros appear.

No macro is emitted for a function that was not imported or reached through a dependency.

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
