---
title: Pipeline
description: The path from Zlang source to a native executable.
navigation:
  title: Pipeline
---

# Pipeline

The compiler is a sequence of small stages:

```console
source.z -> lexer -> parser -> semantics -> planner -> emitter -> C compiler
```

The lexer reads source text. The parser builds a flat AST. Semantic analysis binds names and checks types. The planner decides storage. The emitter writes C99.

`zc --emit-c` stops after the emitter. Without that flag, the driver calls the host C compiler.

## Ownership

Each stage writes into buffers owned by its caller. The compiler core has no mutable globals and calls no allocator.

That keeps stages independent and makes failures easier to place.

## Generated C

The emitter writes native C99 with runtime includes and activation macros for imported functions. Source functions become `app_fn_*`, source locals become `app_local_*`, and imported standard calls become `zlang_std_*` symbols from the registry. Bounds and arithmetic checks call `zlang_*` runtime helpers. There is no opcode loop, class registry or runtime name lookup.

See [Emitted C](/docs/compiler/emitted-c) for an annotated Fibonacci excerpt and [Compiler pipeline](/docs/compiler/pipeline) for the stage contract.
