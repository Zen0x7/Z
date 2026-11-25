---
title: Language
description: Types, declarations, statements, functions, builtins and errors in Zlang.
---

# Language

Zlang gives every value a known shape. You declare the length of an array and the capacity of a string. The compiler checks those numbers before it writes C.

## A first look

```zlang
array[4] integer values = {2, 4, 6, 8};
integer total = 0;

foreach (value in values) {
    total += value;
}

print(total);
print("\n");
```

The array holds four integers. `foreach` walks those four elements. No allocation happens while the program runs.

## Names and literals

Identifiers start with a letter or underscore. They continue with letters, digits or underscores. Keywords are `boolean`, `break`, `clone`, `const`, `continue`, `else`, `enum`, `false`, `for`, `foreach`, `from`, `function`, `if`, `import`, `in`, `integer`, `length`, `print`, `record`, `reference`, `repeat`, `return`, `array`, `string`, `true`, `unsigned` and `while`.

Integers are decimal and may end in `u`. Strings use double quotes and support escaped quotes, backslashes, newlines and tabs. Comments use `//` or `/* ... */` and survive into the generated C.

Names that start with `zlang`, `_zlang` or `ZLANG` belong to generated C. Zlang rejects them with `ZLANG1004`.

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

A `const` value needs an initializer and never changes. `array[N]` sets the number of elements. An empty `array[]` takes its length from the initializer. `string[M]` sets how many bytes the string can hold.

Records and enums give names to your own shapes:

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

## Functions

```zlang
function add(clone integer left, clone integer right): integer {
    return left + right;
}

function bump(reference integer value) {
    value += 1;
}
```

Every parameter says how it is passed. `clone` gives the function a copy. `reference` lets the function change the caller's value. Add `const` before either mode when the function must not change it.

## Statements

| Statement | Use |
| --- | --- |
| `if (...) {...} else {...}` | Choose a branch |
| `while (...)` | Repeat while a condition holds |
| `for (init; condition; update)` | Count with an explicit loop |
| `foreach (item in values)` | Walk a sequence |
| `foreach (index, item in values)` | Walk a sequence with its index |
| `repeat (count)` | Run a fixed number of times |
| `break;` / `continue;` | Leave or skip part of a loop |
| `print(value);` | Write a string or scalar |
| `name = value;` | Assign a value |

Conditions for `if`, `while` and `for` must be `boolean`. `repeat` takes an `unsigned` count.

## Expressions

You can use literals, names, parentheses, indexing, field access, calls, `print`, `length`, unary `!` and `-`, and the operators `+ - * / % == != < > <= >= && ||`. Arithmetic operators also have compound forms such as `+=`.

String literals fold at compile time. Concatenating a variable needs an explicit string capacity and copies into storage owned by the caller. Reading `text[index]` returns one byte as `unsigned`; string elements are read-only.

## Builtins

`length(values)` returns the first extent of a sequence as `unsigned`. `print` writes strings, integers, unsigned integers and booleans. Both use fixed-stack runtime helpers.

## Errors

The compiler prints errors with a stable code and a source location:

```console
input.z:12:5: error[ZLANG3001]: sequence 'names' has no declared capacity
```

Common semantic codes are `ZLANG2002` for a duplicate name, `ZLANG2003` for an unknown name, `ZLANG2004` for a type mismatch and `ZLANG2006` when a string needs an explicit capacity. `ZLANG3001` means an array has no declared capacity.

Runtime traps use five numbers: `1` overflow, `2` bounds, `3` capacity, `4` invalid argument and `5` division by zero.
