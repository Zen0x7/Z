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

## Generated C

`zc` writes the two literals as interned views and lowers each `write` call to the C symbol registered for a string argument. This is an excerpt from `examples/features/hello/main.c`:

```c
/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 16U
static const char zlang_gen_literal_0_data[] = "Hello from Zlang";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };

/* generated function prototypes */

int32_t app_fn_main(void);

int32_t app_fn_main(void) {
    zlang_std_io_write_string(zlang_gen_literal_0);
    zlang_std_io_write_string(zlang_gen_literal_1);

    return 0;
}
```

`zlang_gen_literal_0` holds `"Hello from Zlang"` and `zlang_gen_literal_1` holds `"\n"`. `app_fn_main` comes from `function main(): integer`. The two `zlang_std_io_write_string` calls come from the example's string `write` calls; they do not define another function's behavior. See [Emitted C](/docs/compiler/emitted-c) for the full name mapping.

## See also

- [write](/docs/standard-library/namespaces/io/functions/write)
- [Functions](/docs/language/functions)
