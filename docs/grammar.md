# Zlang grammar

## Scope

Z is a small source-to-source language for bounded, deterministic programs. A Zlang program is
translated to readable C99 that calls a small audited `zlang_*` runtime. There is no VM, no heap,
no garbage collector and no runtime name lookup. All types, capacities and storage decisions are
made by the compiler.

## Lexical rules

Identifiers start with a letter or underscore and continue with letters, digits or underscores.
Keywords are `boolean`, `break`, `clone`, `const`, `continue`, `else`, `enum`, `false`, `for`,
`foreach`, `from`, `function`, `if`, `import`, `in`, `integer`, `length`, `print`, `record`, `reference`, `repeat`,
`return`, `array`, `string`, `true`, `unsigned` and `while`.

Integer literals are decimal and may carry a `u` suffix. String literals are double-quoted and
support escaped quotes, backslashes, newlines and tabs. Line comments `// ...` and block comments
`/* ... */` do not affect parsing, but the emitter preserves them in the generated C next to the construct they document. Use `--disable-comments` to omit them.

Identifiers beginning with `zlang`, `_zlang` or `ZLANG` are reserved for generated C and are rejected
with `ZLANG1004`.

## Imports

```zlang
import { generate_int32, generate_bytes } from "random";
import { tcp_listener } from "networking";
```

Imports appear at file scope and introduce standard library function names. They do not import user
modules. Generated C activates only the imported runtime headers.

## Declarations

```zlang
const integer base = 10;
integer total = 1;
unsigned limit = 3U;
boolean ready = true;
string label = "z" + "lang";
string[32] bounded = label + " value";

array[4] integer values;
array[] integer inferred = {1, 2, 3};
array[5] integer partial = {1, 2, 3};
array[2] string[8] names = {"a", "b"};
```

`array[N]` declares a fixed number of elements. An empty capacity infers N from a brace
initializer. `string[M]` declares a bounded string with M content bytes. Applied inside a sequence
declaration, it sets the capacity of each string element.

A `const` declaration requires an initializer and cannot be assigned to.

A `record` declares a fixed aggregate with named fields. An `enum` declares named integer values
and a user type. Both can be used as declaration types, array element types, function parameter
types and function return types:

```zlang
record Point {
    integer x;
    integer y;
}

enum Color {
    RED,
    GREEN,
    BLUE
}

array[4] Point points;
Color current = GREEN;
```

Every parameter needs `clone` or `reference`, with optional `const` before the mode. `clone` is a
local value copy; `reference` aliases caller storage and cannot be stored or returned. Shadowing a
visible name is rejected;
sibling blocks may reuse a source name because the emitter generates unique C identifiers when
needed.

## Statements

| Syntax | Meaning |
| --- | --- |
| `type name = expression;` | Scalar declaration |
| `array[N] type name;` | Fixed-shape array with explicit extents |
| `array type name = {{...}, {...}};` | Nested initializer with inferred shape |
| `array type name;` | Array with inferred rank and extents |
| `array[] type name = {...};` | Array with inferred extent |
| `if (expression) {...} else if (expression) {...} else {...}` | Conditional chain |
| `while (expression) {...}` | Pre-tested loop |
| `for (init; condition; update) {...}` | Native C-style loop |
| `foreach (item in sequence) {...}` | Native indexed loop |
| `foreach (index, item in sequence) {...}` | Indexed loop exposing the index as `unsigned` |
| `foreach (row, col in grid) {...}` | Multidimensional index loop in row-major order |
| `repeat (count) {...}` | Native counted loop |
| `break;` / `continue;` | Loop control, valid only inside a loop |
| `function name(const? clone array name, ...): array {...}` | Function with inferred array parameters and return |
| `function name(const? clone type name, ...): type {...}` | Function with value parameters |
| `function name(const? reference type name, ...): type {...}` | Function with caller-owned references |
| `return expression;` / `return;` | Return from a function |
| `print(expression);` | Write a string or scalar |
| `name = expression;` | Assignment |
| `record Name { field* }` | Fixed aggregate type |
| `enum Name { VALUE, ... }` | Named integer values and their type |
| `value.field` | Field read |
| `value.field = expression;` | Field assignment; checked for array elements |

## Expressions

Supported forms are literals, identifiers, grouping, indexing, field access and assignment, calls
to user functions, `print`, `length`, unary `!` and `-`, and binary `+`, `-`, `*`, `/`, `%`,
`==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`. Compound assignment is available for arithmetic
operators.

`if`, `while` and `for` conditions must be `boolean`. `repeat` counts must be `unsigned`. A
non-void function must return a value of its declared type; a void function must return without a
value. Loops may nest freely, including `for` inside `for` and `foreach` inside `foreach`.

String concatenation folds literals at compile time. Concatenation involving a variable requires
an explicit string capacity and copies into caller-owned storage. Indexing a string reads one byte
as `unsigned` with a bounds trap; string elements are read-only.

## Builtins

`length(sequence)` emits the native first extent macro as an unsigned value. `print`
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
| `ZLANG3001` | Unbounded array |
| `ZLANG3002` | Unused capacity warning |
| `ZLANG3003` | Initializer too long |
| `ZLANG3004` | Unnecessary reference parameter warning |

Runtime traps use fixed codes: `1` overflow, `2` bounds, `3` capacity, `4` invalid argument and
`5` division by zero.
