---
title: Diagnostics
description: Compiler error format, semantic codes and runtime trap codes.
navigation:
  title: Diagnostics
---

# Diagnostics

The compiler prints errors with a stable code and a source location.

```console
input.z:12:5: error[ZLANG3001]: sequence 'names' has no declared capacity
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
| `ZLANG3003` | Initializer too long |

## Runtime traps

Generated C uses fixed trap codes:

| Code | Meaning |
| --- | --- |
| `1` | overflow |
| `2` | bounds |
| `3` | capacity |
| `4` | invalid argument |
| `5` | division by zero |

Generated code never formats an error string at runtime.
