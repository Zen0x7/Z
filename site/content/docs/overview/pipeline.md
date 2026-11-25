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

The generated file includes the runtime and calls documented `zlang_*` functions. It has no opcode loop, no class registry and no runtime name lookup.

See [Compiler pipeline](/docs/compiler/pipeline) for the detailed contract.
