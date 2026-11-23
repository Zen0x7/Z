# Zlang 0.1 grammar

## Scope

Zlang 0.1 is the first supported language level. It is deliberately small: declarations, control
flow, functions and expressions that map to auditable C99. Modules, classes, closures and operator
overloading are 1.0 work.

## Lexical rules

Identifiers start with a letter or underscore and continue with letters, digits or underscores.
Keywords are `integer`, `unsigned`, `boolean`, `string`, `if`, `else`, `while`, `foreach`, `in`,
`repeat`, `function`, `return`, `true` and `false`.

Integer literals are decimal. String literals are double-quoted and support escaped quotes,
escaped backslashes, newline escapes and tab escapes. Comments are not part of 0.1.

## Statements

| Syntax | Meaning |
| --- | --- |
| `type name = expression;` | Scalar declaration |
| `type[capacity] name;` | Fixed-capacity sequence declaration |
| `type[] name;` | Unbounded sequence; rejected with `ZLANG3001` |
| `if (expression) { ... } else { ... }` | Conditional; `else` is optional |
| `while (expression) { ... }` | Pre-tested loop |
| `foreach (item in sequence) { ... }` | Native indexed loop |
| `repeat (count) { ... }` | Native counted loop |
| `function name(type name, ...): type { ... }` | Function with optional return type |
| `return expression;` / `return;` | Return from a function |
| `print(expression);` | Write a string to standard output |
| `append(sequence, expression);` | Append with capacity trap |
| `name = expression;` | Assignment |

## Expressions

Supported forms are literals, identifiers, indexing, `+`, `==`, assignment, `print(...)` and
`append(...)`. Addition is checked. Equality is emitted as native C comparison. Assignment returns
the assigned value.

## Type mapping

| Zlang | Generated C |
| --- | --- |
| `integer` | `int32_t` |
| `unsigned` | `uint32_t` |
| `boolean` | `bool` |
| `string` | `zlang_string_t` |

`zlang_string_t` is a non-owning view: a pointer plus an explicit byte length. Zlang 0.1 has no
string concatenation.

## Diagnostics

| Code | Meaning |
| --- | --- |
| `ZLANG1001` | Unexpected token |
| `ZLANG1002` | Unterminated string |
| `ZLANG1003` | Invalid character |
| `ZLANG1004` | Unsupported construct |
| `ZLANG2001` | Literal out of range |
| `ZLANG2002` | Duplicate declaration |
| `ZLANG2003` | Undefined identifier |
| `ZLANG2004` | Type mismatch |
| `ZLANG3001` | Unbounded sequence declaration |

Runtime trap codes are separate: `1` overflow, `2` bounds, `3` capacity and `4` invalid argument.
