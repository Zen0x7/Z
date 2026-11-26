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
zc input.z --timing                      # instrument registered stdlib calls
zc input.z --dump                        # enable bounded result dump storage
```

With `--emit-c`, `zc` stops after the C emitter. Without it, the driver calls `$ZLANG_CC`, or `cc` on POSIX and `cl` on Windows.

`--timing` defines `ZLANG_STD_RESULT_TIMING` and instruments registered standard library calls.
Without it, `time()` returns `0` and no clock read is emitted. `--dump` defines
`ZLANG_STD_RESULT_DUMP` and adds bounded storage for `dump()`. Without it, `dump()` returns the
empty view. The flags are independent and can be combined.

## Diagnostics

Errors use one form:

```console
main.z:3:9: error[ZLANG3001]: unbounded array 'values' requires an explicit capacity.
```

The file, line and column point to the source construct. The code is stable and can be matched by scripts.

## See also

- [Emit C](/docs/compiler/emitted-c)
- [Diagnostics](/docs/compiler/diagnostics)
