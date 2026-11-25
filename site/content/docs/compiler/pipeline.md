---
title: Pipeline
description: Lexer, parser, semantics, planner and emitter.
navigation:
  title: Pipeline
---

# Pipeline

```console
source.z
  -> lexer          tokens and copied text
  -> parser         flat AST
  -> semantics      names, types and diagnostics
  -> planner        storage for each declaration
  -> emitter        C99 source
  -> C compiler     native executable
```

The lexer copies identifiers and strings into scratch storage. The parser writes statements and expressions into flat arrays. Semantic analysis binds names and records diagnostics. The planner chooses storage. The emitter appends C text to a caller-provided buffer.

## Why stages are separate

Each stage can be tested on its own. Data crossing a stage boundary is owned by the caller, so sanitizers can see it.

## See also

- [Memory plan](/docs/compiler/memory-plan)
- [Emitted C](/docs/compiler/emitted-c)
