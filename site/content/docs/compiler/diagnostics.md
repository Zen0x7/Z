---
title: Diagnostics
description: Diagnostic format and verification rules.
navigation:
  title: Diagnostics
---

# Diagnostics

A diagnostic has a file, line, column, stable code and message:

```console
main.z:3:9: error[ZLANG3001]: unbounded array 'values' requires an explicit capacity.
```

Codes stay stable across releases. Scripts can match on them without parsing the message text.

Warnings use the same format with a warning code. Tests compare the full diagnostic line.

## Verification

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build -j
ctest --test-dir build --output-on-failure -j13
```

CI also runs C11, C17 and C23, sanitizers, static analysis, coverage and multiple architectures.

## See also

- [Language diagnostics](/docs/language/diagnostics)
- [Quick start command line](/docs/quick-start/command-line)
