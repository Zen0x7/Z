---
title: Lexical rules
description: Identifiers, keywords, literals and comments.
navigation:
  title: Lexical rules
---

# Lexical rules

Identifiers start with a letter or underscore. They continue with letters, digits or underscores.

Keywords include `boolean`, `break`, `clone`, `const`, `continue`, `else`, `enum`, `false`, `for`, `foreach`, `from`, `function`, `if`, `import`, `in`, `integer`, `length`, `record`, `reference`, `repeat`, `return`, `array`, `string`, `true`, `unsigned`, `int64`, `uint64`, `float`, `double` and `while`.

Output is a standard library function, not a keyword:

```zlang
import { write } from "@std/io";
```

## Literals

Integers are decimal. Unsigned literals end in `u`. Floating literals use decimal notation and `f` for `float`.

```zlang
integer count = 10;
unsigned limit = 3U;
double ratio = 0.5;
float measure = 1.5f;
```

Strings use double quotes and support escaped quotes, backslashes, newlines and tabs.

## Comments

Zlang supports line and block comments. The emitter writes them back near the construct they document.

```zlang
// a line comment
/* a block comment */
```

Names beginning with `zlang`, `_zlang` or `ZLANG` belong to generated C and are rejected.
