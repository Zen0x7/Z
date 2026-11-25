---
title: Diagnostics
description: Compiler error format, semantic codes and runtime trap codes.
navigation:
  title: Diagnostics
---

# Diagnostics

The compiler prints errors with a stable code and a source location.

```console
input.z:12:5: error[ZLANG3001]: cannot infer array extent
```

The file, line, column and code belong to the source construct that caused the error.

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
