---
title: Functions
description: Function syntax, parameter modes and main.
navigation:
  title: Functions
---

# Functions

A function has a name, parameters and an optional return type.

```zlang
function add(clone int32 left, clone int32 right): int32 {
    return left + right;
}

function bump(reference int32 value) {
    value += 1;
}
```

Except in `main`, every parameter uses `clone` or `reference`. Add `const` before the mode when the function must not change the value.

- `clone` passes a copy.
- `reference` aliases caller storage.
- References are parameter-only. They cannot be stored or returned.

A function without a return type emits `void`.

## main

`main` is the program entry point. Its parameters use implicit binding modes, so `clone` and `reference` are not written.

```zlang
function main(): int32 {
    return 0;
}
```

With host arguments:

```zlang
function main(int32 argc, arguments argv): int32 {
    return 0;
}
```

## Visibility

Zlang has no mutable globals. Functions receive state through parameters. Stateful standard functions receive a runtime context as an extra C argument; `zc` builds that argument in `main`. The Zlang signature does not include it.
