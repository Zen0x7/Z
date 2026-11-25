---
title: First program
description: Read a minimal Zlang program line by line.
navigation:
  title: First program
---

# First program

Write this to `hello.z`:

```zlang
import { write } from "@std/io";

function main(): integer {
    write("Hello from Zlang");
    write("\n");
    return 0;
}
```

`import` activates `write` from the standard library. `main` is the program entry point. `write` sends one bounded value to standard output. `return 0` ends the program successfully.

## Run it

```bash
./build/tools/zc/zc hello.z
./hello
```

```console
Hello from Zlang
```

`zc` writes `hello.c` and calls the host C compiler. The binary uses the generated C and the header-only runtime.

## See also

- [write](/docs/standard-library/namespaces/io/functions/write)
- [Functions](/docs/language/functions)
