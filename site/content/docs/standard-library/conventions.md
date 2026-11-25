---
title: Conventions
description: How names, imports and domain specifiers work in the standard library.
kind: conventions
status: implemented
---

# Conventions

Imports activate standard functions by name. They are file-scope declarations and must appear before executable statements.

```zlang
import { write } from "@std/io";
```

Read that line as: from inputs and outputs, write.

## Rules

- Names go inside braces, separated by commas.
- `from` names the domain.
- One statement may import several names from one domain.
- Several statements may target the same domain.
- Duplicate imported names are rejected.
- An imported name cannot be redeclared by a user function or variable.
- There are no default imports, wildcards or aliases in the first version.

More than one name:

```zlang
import { read, write } from "@std/io";
```

## Domain specifiers

Canonical standard domains use `@std/<domain>`:

```zlang
import { generate_int32 } from "@std/random";
```

User modules use relative paths or `z.yaml` aliases. They are a different mechanism. A domain such as `@std/io` always refers to the language library.

## Naming

Domains are full lowercase nouns. Function names start with a verb and do not repeat the domain.

```zlang
import { starts_with } from "text";
```

Avoid abbreviations such as `inet`, `rng` or `str`. Avoid catch-all domains such as `util`, `common` and `core`.
