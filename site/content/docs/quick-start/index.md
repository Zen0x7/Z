---
title: Quick start
description: Build zc and run a Zlang program.
navigation:
  title: Quick start
---

# Quick start

You need `zc` and a C compiler. Build the compiler once, write a `.z` file, then run the binary that `zc` produces.

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build -j
```

Write `hello.z`:

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

```console
Hello from Zlang
```

`zc` writes `hello.c` and calls your C compiler. The binary uses the generated C and the header-only runtime.

For sanitizers, coverage, tests and header checks, see [Build](/docs/quick-start/build).

## Pages at this level

- [Overview](/docs/quick-start) — the short path.
- [Build](/docs/quick-start/build) — build options and verification.
- [First program](/docs/quick-start/first-program) — the program above, line by line.
- [Command line](/docs/quick-start/command-line) — common `zc` forms.
- [Examples](/docs/quick-start/examples) — golden fixtures in the repository.
