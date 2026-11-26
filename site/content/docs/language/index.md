---
title: Language
description: Types, declarations, statements, functions, expressions and errors.
navigation:
  title: Language
---

# Language

Zlang fixes types and capacities before it emits C. Arrays use an explicit capacity or infer one from a brace initializer. A string built with `+` from a variable declares a byte capacity. The compiler rejects missing capacities before it writes C; bounds checks stay in the emitted program.

```zlang
import { write } from "@std/io";

function main(): integer {
    array[4] integer values = {2, 4, 6, 8};
    integer total = 0;

    foreach (value in values) {
        total += value;
    }

    write(total);
    write("\n");
    return 0;
}
```

The array holds four integers. `foreach` walks those four elements with a native C loop. No storage grows while the program runs.

For the generated loop and its names, see [Emitted C](/docs/compiler/emitted-c).

## Pages at this level

- [Language](/docs/language) — this page.
- [Lexical rules](/docs/language/lexical)
- [Types](/docs/language/types)
- [Declarations](/docs/language/declarations)
- [Statements](/docs/language/statements)
- [Functions](/docs/language/functions)
- [Expressions](/docs/language/expressions)
- [Diagnostics](/docs/language/diagnostics)
