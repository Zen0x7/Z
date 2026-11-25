---
title: Statements
description: Control flow and loop forms.
navigation:
  title: Statements
---

# Statements

Zlang has native loops and block statements. It has no callback-based iteration.

| Statement | Use |
| --- | --- |
| `if (...) {...} else {...}` | Choose a branch |
| `while (...)` | Repeat while a condition holds |
| `for (init; condition; update)` | Count with an explicit loop |
| `foreach (item in values)` | Walk an array |
| `foreach (index, item in values)` | Walk with an `unsigned` index |
| `repeat (count)` | Run a counted loop |
| `break;` / `continue;` | Leave or skip part of a loop |
| `return` / `return value;` | Leave a function |
| `name = value;` | Assign a value |

Conditions for `if`, `while` and `for` must be `boolean`. `repeat` takes an `unsigned` count.

## Example

```zlang
import { write } from "@std/io";

function main(): integer {
    for (integer index = 0; index < 3; index += 1) {
        if (index == 1) {
            continue;
        }
        write(index);
    }
    return 0;
}
```

`foreach` becomes a C `for` loop. The generated binary contains no callback.
