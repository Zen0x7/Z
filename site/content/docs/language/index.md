---
title: Language
description: Types, declarations, statements, functions, expressions and errors.
navigation:
  title: Language
---

# Language

Zlang gives every value a known shape. Array capacities are explicit or inferred from an initializer. Mutable strings declare a capacity. The compiler checks those numbers before it writes C.

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

The array holds four integers. `foreach` walks those four elements. Nothing grows while the program runs.

## Pages at this level

- [Language](/docs/language) — this page.
- [Lexical rules](/docs/language/lexical)
- [Types](/docs/language/types)
- [Declarations](/docs/language/declarations)
- [Statements](/docs/language/statements)
- [Functions](/docs/language/functions)
- [Expressions](/docs/language/expressions)
- [Diagnostics](/docs/language/diagnostics)
