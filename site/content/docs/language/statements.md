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
| `foreach (index, item in values)` | Walk with an `uint32` index |
| `repeat (count)` | Run a counted loop |
| `break;` / `continue;` | Leave or skip part of a loop |
| `return` / `return value;` | Leave a function |
| `name = value;` | Assign a value |

Conditions for `if`, `while` and `for` must be `boolean`. `repeat` takes an `uint32` count.

## Example

```zlang
import { write } from "@std/io";

function main(): int32 {
    for (int32 index = 0; index < 3; index += 1) {
        if (index == 1) {
            continue;
        }
        write(index);
    }
    return 0;
}
```

`foreach` lowers to a native C `for` loop over the array elements. The generated binary contains no callback. For the generated loop variable and array field names, see [Emitted C](/docs/compiler/emitted-c).
