---
title: Diagnostics
description: Compiler error format, semantic codes and runtime trap codes.
navigation:
  title: Diagnostics
---

# Diagnostics

The compiler prints errors with a stable code and a source location. For this source:

```zlang
function main(): integer {
    integer value = 1 @ 2;
    return 0;
}
```

`zc` prints:

```console
main.z:2:23: error[ZLANG1003]: invalid character
main.z:1:1: error[ZLANG1001]: module source has invalid tokens
```

Each line gives a file, line, column and code. `zc` can print more than one diagnostic for one source file. Match on the code; the message text is not a stable API.

## Common semantic codes

| Code | Meaning |
| --- | --- |
| `ZLANG1001` | Unexpected token |
| `ZLANG1002` | Unterminated string |
| `ZLANG1003` | Invalid character |
| `ZLANG1004` | Unsupported construct or placement |
| `ZLANG2001` | Literal out of range |
| `ZLANG2002` | Duplicate declaration |
| `ZLANG2003` | Undefined identifier |
| `ZLANG2004` | Type mismatch |
| `ZLANG2005` | Shadowed declaration |
| `ZLANG2006` | Explicit string capacity required |
| `ZLANG3001` | Unbounded array |
| `ZLANG3002` | Unused array capacity (warning) |
| `ZLANG3003` | Initializer too long |
| `ZLANG3004` | Unnecessary reference parameter (warning) |

Warnings use the `warning` prefix and do not stop compilation.

## Runtime traps

Generated C uses fixed trap codes:

| Code | Meaning |
| --- | --- |
| `1` (`ZLANG_TRAP_OVERFLOW`) | overflow |
| `2` (`ZLANG_TRAP_BOUNDS`) | bounds |
| `3` (`ZLANG_TRAP_CAPACITY`) | capacity |
| `4` (`ZLANG_TRAP_INVALID_ARGUMENT`) | invalid argument |
| `5` (`ZLANG_TRAP_DIVISION_BY_ZERO`) | division by zero |

Generated code never formats an error string at runtime.
