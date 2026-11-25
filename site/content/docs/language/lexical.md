---
title: Lexical rules
description: Identifiers, keywords, literals and comments.
navigation:
  title: Lexical rules
---

# Lexical rules

Identifiers start with a letter or underscore. They continue with letters, digits or underscores.

Keywords include `boolean`, `break`, `clone`, `const`, `constructor`, `continue`, `else`, `enum`, `false`, `for`, `foreach`, `from`, `function`, `if`, `import`, `in`, `integer`, `length`, `struct`, `static`, `reference`, `repeat`, `return`, `array`, `string`, `true`, `arguments`, `unsigned`, `int64`, `uint64`, `float`, `double` and `while`.

Output is a standard library function, not a keyword:

```zlang
import { write } from "@std/io";
```

## Literals

Integers are decimal. Unsigned literals end in `u` or `U`. 64-bit literals end in `i64` or `u64`. Floating literals use a decimal point or an exponent, and `f` marks a `float`.

```zlang
integer count = 10;
unsigned limit = 3U;
int64 big = 9000000000i64;
uint64 wide = 18000000000000000000u64;
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
