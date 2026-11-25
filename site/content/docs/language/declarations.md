---
title: Declarations
description: Variables, constants, arrays, structs and enums.
navigation:
  title: Declarations
---

# Declarations

A variable declaration names a type and gives it a value or storage shape.

```zlang
integer total = 1;
unsigned limit = 3U;
boolean ready = true;
string label = "z" + "lang";
string[32] bounded = label + " value";
```

A `const` declaration cannot be assigned to. An uninitialized `const` scalar starts at zero.

## Arrays

```zlang
array[4] integer values;
array[] integer inferred = {1, 2, 3};
array[5] integer partial = {1, 2, 3};
array[2] string[8] names = {"a", "b"};
```

`array[N]` sets the capacity. `array[]` infers it from the initializer. An explicit capacity may be combined with a shorter initializer.

An array without an explicit capacity needs a non-empty initializer. Without either, the compiler reports `ZLANG3001`. An initializer longer than the capacity is reported as `ZLANG3003`.

## Structs and enums

```zlang
struct Point {
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

Structs and enums declare user types. They can be used as variables, array elements, parameters and return values.
