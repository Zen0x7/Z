---
title: Command line
description: zc invocation forms and output files.
navigation:
  title: Command line
---

# Command line

```console
zc input.z --emit-c                       # write input.c
zc input.z                                # write input.c and build a native executable
zc input.z --emit-c --disable-comments    # omit source comments from input.c
```

With `--emit-c`, `zc` stops after the C emitter. Without it, the driver calls `$ZLANG_CC`, or `cc` on POSIX and `cl` on Windows.

## Diagnostics

Errors use one form:

```console
input.z:12:5: error[ZLANG3001]: sequence 'names' has no declared capacity
```

The file, line and column point to the source construct. The code is stable and can be matched by scripts.

## See also

- [Emit C](/docs/compiler/emitted-c)
- [Diagnostics](/docs/compiler/diagnostics)
