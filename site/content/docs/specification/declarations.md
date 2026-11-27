---
title: Declarations and storage
description: Variables, constants, initializers, imports and compile-time storage.
navigation:
  title: Declarations and storage
---

# Declarations and storage

## Variables

```zlang
<type> <name> [= <expression>] ;
```

A declaration introduces a compile-time symbol. Its name is never emitted as a string. Generated C uses the same source identifier when it is available; otherwise it appends a numeric suffix. Sibling scopes may reuse a source name without colliding in generated C.

Scalars without an initializer are zero-initialized. Strings without an initializer are the empty string. An initializer must have the exact declared type.

`auto` replaces the declared type when the declaration has an initializer. The compiler resolves the initializer type before storage planning; the emitted C always uses the concrete type. A native generic argument may also be inferred:

```zlang
auto value = query();
result_type<auto, status_code> result = query();
```

An `auto` declaration without an initializer is rejected.

## Arrays and strings with capacity

```zlang
array[4] int32 values = {1, 2, 3};
string[32] greeting = prefix + " tour";
```

An unbounded array is `ZLANG3001`. An empty inferred array is also rejected with `ZLANG3001`. An initializer longer than the declared capacity is `ZLANG3003`. An explicit capacity that exceeds the initializer is reported as `ZLANG3002`, a warning that does not stop compilation.

A variable string concatenation requires explicit capacity. The compiler reserves a fixed backing buffer and copies the views into it. Overflow traps with `ZLANG_TRAP_CAPACITY` instead of growing the buffer. When every operand is a literal, the compiler folds the expression into one emitted literal.

## Scope

A name is visible from its declaration to the end of the enclosing block. Shadowing is not allowed. Redeclaring a visible name in an inner block is `ZLANG2005`; redeclaring it in the same scope is `ZLANG2002`. Assignment to a `const` declaration is rejected.

## Storage plan

Decisions about storage happen before C is written.

- Total size at or below `ZLANG_DEFAULT_STACK_LIMIT` (4096 bytes) is emitted as a local C aggregate.
- Larger values are emitted as `static` storage with internal linkage.
- The compiler must document any threshold that differs from the default.

Arrays keep their extents in generated macros and use typed getters and setters for checked access. Only referenced helpers are emitted.

## Imports

```zlang
import { write } from "@std/io";
import { base } from "@lib/config";
```

Imports activate standard functions by name. Standard domains use canonical `@std/<domain>` specifiers. User modules use relative paths or `@alias/...` paths declared in `z.yaml`. Each imported function activates one macro in generated C. Legacy bare-domain imports such as `"random"` are rejected, as are cyclic imports and paths that escape the module root.
