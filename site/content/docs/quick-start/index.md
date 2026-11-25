---
title: Quick start
description: Build zc and run a Zlang program.
navigation:
  title: Quick start
---

# Quick start

Build the compiler, write a small program, and run it.

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build -j
ctest --test-dir build --output-on-failure -j
```

Then write `hello.z` with an explicit `main`:

```zlang
import { write } from "@std/io";

function main(): integer {
    write("Hello from Zlang");
    write("\n");
    return 0;
}
```

Compile and run:

```bash
./build/tools/zc/zc hello.z
./hello
```

## Pages at this level

- [Overview](/docs/quick-start) — the short path.
- [Build](/docs/quick-start/build) — build options and tests.
- [First program](/docs/quick-start/first-program) — the program above, line by line.
- [Command line](/docs/quick-start/command-line) — every `zc` form.
- [Examples](/docs/quick-start/examples) — golden fixtures in the repository.
