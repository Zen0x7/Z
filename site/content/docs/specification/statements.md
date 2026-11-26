---
title: Statements and control flow
description: Expressions, branches, loops, returns and loop jumps.
navigation:
  title: Statements
---

# Statements and control flow

## Expression statements

An expression followed by `;` is a statement. Expression statements include assignment, indexed assignment, `write` calls and calls to user-defined functions.

## Branches

```zlang
if (<boolean-expression>) {
    <statements>
} else {
    <statements>
}
```

The `else` clause is optional. An `else` branch may contain another `if`, forming an `else if` chain that emits as native C. The condition must be `boolean`.

## Loops

```zlang
while (<boolean-expression>) { <statements> }
for (int32 index = 0; index < 3; index += 1) { <statements> }
repeat (2U) { <statements> }
foreach (value in values) { <statements> }
```

A `while` condition must be `boolean`. A `repeat` count must be `uint32` and is equivalent to a counted loop from `0` to `count - 1`. A `for` loop uses native C initialization, condition and update clauses.

A `foreach` over a rank-N array takes either N index names or N index names followed by one value name.

```zlang
foreach (row, col in square) { <statements> }
foreach (index, item in values) { <statements> }
```

The value variable is a fresh local of the element type. Assigning to it does not modify the array. Every loop emits native C control flow.

## Returns and loop jumps

```zlang
return <expression>;
break;
continue;
```

A non-void function must return a value whose type matches the declared return type. A void function must return without a value. A `return` outside a function is `ZLANG1004`.

`break` and `continue` are valid only inside `while`, `repeat`, `foreach` or `for`. They emit native C `break;` and `continue;` and are `ZLANG1004` elsewhere.

## Logical operators

`!`, `&&` and `||` require boolean operands. `!` binds tighter than comparisons, `&&` binds tighter than `||`, and assignment has the lowest precedence. Parentheses may group any expression. `&&` and `||` short-circuit in generated C. Reads that exist only under a short-circuit operator stay inline so trap timing is unchanged.
