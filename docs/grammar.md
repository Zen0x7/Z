# Zlang grammar

## Scope

Z is a small source-to-source language for bounded, deterministic programs. A Zlang program is
translated to readable C99 that calls a small audited `zlang_*` runtime. There is no VM, no heap,
no garbage collector and no runtime name lookup. All types, capacities and storage decisions are
made by the compiler.

## Lexical rules

Identifiers start with a letter or underscore and continue with letters, digits or underscores.
Keywords are `boolean`, `break`, `continue`, `else`, `false`, `foreach`, `function`, `if`, `in`,
`integer`, `length`, `print`, `repeat`, `return`, `sequence`, `string`, `true`, `unsigned` and
`while`.

Integer literals are decimal and may carry a `u` suffix. String literals are double-quoted and
support escaped quotes, backslashes, newlines and tabs. Line comments `// ...` and block comments
`/* ... */` are discarded before parsing.

## Declarations

```zlang
integer total = 1;
unsigned limit = 3U;
boolean ready = true;
string label = "z" + "lang";
string[32] bounded = label + " value";

sequence[4] integer values;
sequence[] integer inferred = {1, 2, 3};
sequence[5] integer partial = {1, 2, 3};
sequence[2] string[8] names = {"a", "b"};
```

`sequence[N]` declares a fixed number of elements. An empty capacity infers N from a brace
initializer. `string[M]` declares a bounded string with M content bytes. Applied inside a sequence
declaration, it sets the capacity of each string element.

Shadowing a visible name is rejected; sibling blocks may reuse a source name because the emitter
generates unique C identifiers when needed.

## Statements

| Syntax | Meaning |
| --- | --- |
| `type name = expression;` | Scalar declaration |
| `sequence[N] type name;` | Fixed-capacity sequence |
| `sequence[] type name = {...};` | Sequence with inferred capacity |
| `if (expression) {...} else if (expression) {...} else {...}` | Conditional chain |
| `while (expression) {...}` | Pre-tested loop |
| `foreach (item in sequence) {...}` | Native indexed loop |
| `repeat (count) {...}` | Native counted loop |
| `break;` / `continue;` | Loop control, valid only inside a loop |
| `function name(type name, ...): type {...}` | Function with optional return type |
| `return expression;` / `return;` | Return from a function |
| `print(expression);` | Write a string or scalar |
| `append(sequence, expression);` | Append with capacity trap |
| `name = expression;` | Assignment |

## Expressions

Supported forms are literals, identifiers, grouping, indexing and assignment, calls to user
functions, `print`, `append`, `length`, unary `!` and `-`, and binary `+`, `-`, `*`, `/`, `%`,
`==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`. Compound assignment is available for arithmetic
operators.

`if` and `while` conditions must be `boolean`. `repeat` counts must be `unsigned`. A non-void
function must return a value of its declared type; a void function must return without a value.

String concatenation folds literals at compile time. Concatenation involving a variable requires
an explicit string capacity and copies into caller-owned storage.

## Builtins

`length(sequence)` emits the native sequence `length_` field as an unsigned value. `print`
accepts strings, integers, unsigned integers and booleans, using fixed-stack runtime helpers.

## Diagnostics

| Code | Meaning |
| --- | --- |
| `ZLANG1001` | Unexpected token |
| `ZLANG1002` | Unterminated string |
| `ZLANG1003` | Invalid character |
| `ZLANG1004` | Unsupported construct or invalid placement |
| `ZLANG2001` | Literal out of range |
| `ZLANG2002` | Duplicate declaration |
| `ZLANG2003` | Undefined identifier |
| `ZLANG2004` | Type mismatch |
| `ZLANG2005` | Shadowed declaration |
| `ZLANG2006` | Explicit string capacity required |
| `ZLANG3001` | Unbounded sequence |
| `ZLANG3002` | Unused capacity warning |
| `ZLANG3003` | Initializer too long |

Runtime traps use fixed codes: `1` overflow, `2` bounds, `3` capacity, `4` invalid argument and
`5` division by zero.
