---
title: Examples
description: Golden programs in the repository.
navigation:
  title: Examples
---

# Examples

The repository keeps worked programs under `examples/algorithms/` and `examples/features/`.

Each directory contains:

- `main.z` — the source program;
- `main.c` — the golden generated C;
- `expected.txt` — exact stdout;
- `README.md` — a short explanation.

The golden C uses generated names: source functions become `app_fn_*`, source locals become `app_local_*`, interned string views become `zlang_gen_literal_*`, and imported calls become `zlang_std_*`. [Emitted C](/docs/compiler/emitted-c) maps those names to Fibonacci source lines.

Run one example:

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.algorithms.fibonacci
```

Regenerate a golden only after an intentional emitter change:

```sh
cd examples/algorithms/fibonacci
../../../build/tools/zc/zc main.z
./main > expected.txt
```

Review the C diff before updating the test.

## See also

- [Fibonacci on GitHub](https://github.com/Zen0x7/Z/blob/master/examples/algorithms/fibonacci/main.z)
