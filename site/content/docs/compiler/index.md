---
title: Compiler
description: How zc plans storage and writes C.
navigation:
  title: Compiler
---

# Compiler

`zc` writes C from Zlang source. By default it also calls a C compiler. With `--emit-c`, it stops after writing the `.c` file.

The compiler is a set of small stages. Each stage uses buffers owned by its caller. The core keeps no mutable globals and calls no allocator.

## Pages at this level

- [Overview](/docs/compiler) — this page.
- [Pipeline](/docs/compiler/pipeline)
- [Memory plan](/docs/compiler/memory-plan)
- [Emitted C](/docs/compiler/emitted-c)
- [Diagnostics](/docs/compiler/diagnostics)
