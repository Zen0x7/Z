# Zlang 0.3 grammar and hardening

## Scope

Zlang 0.3 keeps the language model of 0.2 and adds loop-control statements. It also tightens
semantic rules that were previously documented but not fully enforced.

The 0.1 and 0.2 grammars remain the historical baselines in [grammar_01.md](grammar_01.md) and
[grammar_02.md](grammar_02.md). This document describes only the 0.3 additions.

## Statements

Two new jump statements are available:

| Syntax | Meaning |
| --- | --- |
| `break;` | Leave the innermost enclosing loop |
| `continue;` | Skip to the next iteration of the innermost enclosing loop |

They are valid only inside `while`, `repeat` or `foreach`. A `break` or `continue` outside a loop,
or inside a function nested in a loop, is rejected with `ZLANG1004`. Both emit native C `break;`
and `continue;` statements.

## Tightened semantic rules

- `if` and `while` conditions must be `boolean`.
- `repeat` counts must be `unsigned`.
- `return` is valid only inside a function.
- A `void` function cannot return a value.
- A non-void function must return a value of its declared type.
- Nested statements are analyzed and emitted once by their owning block, not again by an outer
  block.

These checks use `ZLANG2004` for type mismatches and `ZLANG1004` for constructs used in an invalid
position.

## Diagnostics

| Code | Severity | Meaning |
| --- | --- | --- |
| `ZLANG1004` | error | `break`/`continue` outside a loop, or `return` outside a function |
| `ZLANG2004` | error | Invalid condition, repeat count or return type |

Runtime trap codes are unchanged: `1` overflow, `2` bounds, `3` capacity and `4` invalid argument.
