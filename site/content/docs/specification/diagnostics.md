---
title: Diagnostics
description: Diagnostic format, stable codes and source locations.
navigation:
  title: Diagnostics
---

# Diagnostics

Diagnostics use this form:

```text
file:line:column: error[ZLANGxxxx]: message
```

`warning[ZLANGxxxx]` is used for warnings. Line and column numbers are one-based and point at the source construct that caused the diagnostic.

## Classes

| Range | Class |
| --- | --- |
| `ZLANG1xxx` | Syntax |
| `ZLANG2xxx` | Type |
| `ZLANG3xxx` | Resource and allocation |

## Stable codes

| Code | Meaning |
| --- | --- |
| `ZLANG1001` | unexpected token |
| `ZLANG1002` | unterminated string |
| `ZLANG1003` | invalid character |
| `ZLANG1004` | unsupported construct |
| `ZLANG2001` | literal out of range |
| `ZLANG2002` | duplicate declaration |
| `ZLANG2003` | undefined identifier |
| `ZLANG2004` | type mismatch |
| `ZLANG2005` | shadowed declaration |
| `ZLANG2006` | string capacity required |
| `ZLANG3001` | unbounded array |
| `ZLANG3002` | unused capacity warning |
| `ZLANG3003` | initializer too long |
| `ZLANG3004` | unnecessary reference parameter |

Legacy postfix array declarations and reserved identifiers use `ZLANG1004`. Invalid generic applications, duplicate type parameters and exhausted generic capacity are reported through the syntax, type and resource classes.

Resource errors point at the declaration that needs a limit, not at a later use:

```text
main.z:3:9: error[ZLANG3001]: unbounded array 'values' requires an explicit capacity.
```

`ZLANG3002` is a warning and does not stop compilation. The compiler may emit more than one diagnostic, but it must not generate C for a program that has any error.
