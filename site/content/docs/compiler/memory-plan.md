---
title: Memory plan
description: Automatic and static storage decisions.
navigation:
  title: Memory plan
---

# Memory plan

The planner computes each declaration's byte size before emitting C.

- Values at or below the stack limit become local C variables.
- Larger values become file-scope `static` objects with internal linkage.
- Neither kind uses the heap.

`ZLANG_STACK_LIMIT` sets the automatic-storage budget. The default is `4096` bytes. A compiler that uses a different limit must document it.

An array with no capacity never reaches the planner. Semantic analysis stops it with `ZLANG3001`.

## Why it matters

The generated program has no runtime growth strategy. Storage is part of the compiled artifact, so bounds and capacity can be checked against fixed values.

## See also

- [Arrays](/docs/language/types)
- [Pipeline](/docs/compiler/pipeline)
