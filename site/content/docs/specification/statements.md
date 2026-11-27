---
title: Statements and control flow
description: Expressions, branches, loops, labels, jumps, switches and returns.
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
do { <statements> } while (<boolean-expression>);
for (int32 index = 0; index < 3; index += 1) { <statements> }
repeat (2U) { <statements> }
foreach (value in values) { <statements> }
foreach (index in 0U..3U) { <statements> }
```

A `while` and `do while` condition must be `boolean`. A `repeat` count must be `uint32` and is equivalent to a counted loop from `0` to `count - 1`. A `for` loop uses native C initialization, condition and update clauses. A range `start..end` is an exclusive `uint32` range.

A `foreach` over a rank-N array takes either N index names or N index names followed by one value name.

```zlang
foreach (row, col in square) { <statements> }
foreach (index, item in values) { <statements> }
```

The value variable is a fresh local of the element type. Assigning to it does not modify the array. Every loop emits native C control flow.

## Switches

```zlang
switch (value) {
    case 1:
        write("one");
    default:
        write("other");
}
```

The selector may be `int32`, `uint32`, `int64`, `uint64`, `boolean` or an enum. Case expressions must have the selector's type. Case bodies do not fall through; the emitter inserts `break;` at the end of each case and default clause.

## Labels and jumps

```zlang
outer:
while (running) {
    jump done;
}

done:
write("done");
```

A label is a statement marker. `jump label;` emits native C `goto`. Labels are function-scoped. `break label;` and `continue label;` target a labelled enclosing loop; unlabelled forms target the innermost loop or switch.

## Returns and loop jumps

```zlang
return <expression>;
break;
continue;
```

A non-void function must return a value whose type matches the declared return type. A void function must return without a value. A `return` outside a function is `ZLANG1004`.

Unlabelled `break` is valid inside `switch`, `while`, `do while`, `repeat`, `foreach` or `for`. Unlabelled `continue` is valid inside loops. Labelled forms require a label on an enclosing loop. Every jump target must belong to the same function. Duplicate `case` values, duplicate `match` patterns and more than one `default` clause are diagnostics. Other invalid jumps are `ZLANG1004` or `ZLANG2003` when the label is undeclared.

## Logical operators

`!`, `&&` and `||` require boolean operands. Bitwise `&`, `|`, `^` and `~` require matching integral operands. Shifts take an integral left operand and a `uint32` count. Ternary `?:` requires a `boolean` condition and identical branch types. `match` arms use literal or enum patterns and require a final `default` arm. Parentheses may group any expression. `&&` and `||` short-circuit in generated C. Reads that exist only under a short-circuit operator stay inline so trap timing is unchanged.
