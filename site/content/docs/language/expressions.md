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

Binary arithmetic and comparisons require both operands to have the same type. There is no implicit numeric conversion. Use `to_integer`, `to_unsigned`, `to_int64`, `to_uint64`, `to_float` or `to_double`.

## Indexing and fields

```zlang
integer first = values[0];
point.x = 3;
```

String indexing reads one byte as `unsigned`. String elements are read-only. Array and string indexes are bounds checked.

## Concatenation

String literals fold at compile time. Concatenation involving a variable needs an explicit string capacity and copies into caller-owned storage.

```zlang
string[32] full = prefix + " value";
```

## Builtins

`length(values)` returns the first extent of a declared array as `unsigned`. Output comes from `@std/io`:

```zlang
import { write } from "@std/io";

function main(): integer {
    write("Hello");
    return 0;
}
```
