---
title: Lexical rules
description: Identifiers, keywords, literals and comments.
navigation:
  title: Lexical rules
---

# Lexical rules

Identifiers start with a letter or underscore. They continue with letters, digits or underscores.

Source programs in this guide use these keywords: `array`, `arguments`, `boolean`, `break`, `clone`, `const`, `continue`, `double`, `else`, `enum`, `false`, `float`, `for`, `foreach`, `from`, `function`, `if`, `import`, `in`, `int64`, `integer`, `length`, `reference`, `repeat`, `return`, `string`, `struct`, `true`, `uint64`, `unsigned` and `while`. The lexer also reserves `constructor`, `static`, `trait`, `uses` and `requires`; no example in this guide uses them.

Output is a standard library function, not a keyword. Import it with `import { write } from "@std/io";`.

## Literals

Integers are decimal. Unsigned literals end in `u` or `U`. Signed 64-bit literals end in `i64` or `I64`, and unsigned 64-bit literals end in `u64` or `U64`. Floating literals use a decimal point or an exponent; `f` or `F` marks a `float`. The declarations below appear inside a function.

```zlang
integer count = 10;
unsigned limit = 3U;
int64 big = 9000000000i64;
uint64 wide = 18000000000000000000u64;
double ratio = 0.5;
float measure = 1.5f;
```

Strings use double quotes and support the escapes `\"`, `\\`, `\n` and `\t`.

## Comments

Line comments start with `//`. Block comments run from `/*` to `*/`. The emitter writes them back near the construct they document.

Names beginning with `zlang`, `_zlang` or `ZLANG` belong to generated C and are rejected.
