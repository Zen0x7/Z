---
title: Expressions
description: Operators, indexing, calls and conversions.
navigation:
  title: Expressions
---

# Expressions

Expressions include literals, names, grouping, indexing, field access, calls and assignments.

Supported operators:

```text
+ - * / % == != < > <= >= && || !
```

Arithmetic operators also have compound forms such as `+=`, `-=` and `*=`.

Binary arithmetic and comparisons require both operands to have the same type. There is no implicit numeric conversion. Use `to_int32`, `to_uint32`, `to_int64`, `to_uint64`, `to_float` or `to_double`.

## Indexing and fields

Inside a function with an array and a struct value, indexed reads and field writes use these forms:

```zlang
int32 first = values[0];
point.x = 3;
```

String indexing reads one byte as `uint32`. String elements are read-only. Array and string indexes are bounds checked.

## Concatenation

String literals fold at compile time. Concatenation involving a variable needs an explicit string capacity and copies into caller-owned storage:

```zlang
string[32] full = prefix + " value";
```

Here `prefix` is a declared `string`.

## Builtins

`length(values)` returns the first extent of a declared array as `uint32`. Output comes from `@std/io`:

```zlang
import { write } from "@std/io";

function main(): int32 {
    write("Hello");
    return 0;
}
```
