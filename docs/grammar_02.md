# Zlang 0.2 grammar

## Scope

Zlang 0.2 keeps the four value types and the no-allocation model of 0.1. It adds source-level
conveniences that still lower to readable C99 and the fixed `zlang_*` runtime API.

The 0.1 grammar remains the baseline in [grammar_01.md](grammar_01.md). This document describes
only the additions and the refined rules.

## Lexical additions

Line comments `// ...` and block comments `/* ... */` are discarded by the lexer. They do not
create tokens and do not change source locations of later tokens.

## Declarations

A scalar declaration keeps its 0.1 form and gains one string-specific form:

```text
string[capacity] name (= expression)? ;
```

The capacity is written before the string name. It reserves `capacity` mutable content bytes plus
one terminator byte in generated C.

A sequence declaration gains a brace initializer and an inferred capacity:

```text
type name[] = { expression (, expression)* } ;
type name[capacity] = { expression (, expression)* } ;
```

Rules:

- an empty inferred initializer is `ZLANG3001`;
- more initializer elements than the declared capacity is `ZLANG3003`;
- an explicit capacity that is never followed by `append` or index assignment emits the
  `ZLANG3002` warning;
- `length_` is the number of initializer elements, and the unused slots are zero-filled.

## Scopes and names

A name is visible from its declaration to the end of the enclosing block. Reusing a visible name
in a child block is rejected with `ZLANG2005`. Sibling blocks may reuse the same source name; the
emitter appends a numeric suffix to the later generated C identifier so the output remains valid.

## Expressions

New expression forms:

| Syntax | Meaning |
| --- | --- |
| `values[index] = value` | Checked index assignment followed by a direct store |
| `name(argument, ...)` | Direct call to a user-defined function |
| `a < b`, `a > b`, `a <= b`, `a >= b`, `a != b` | Native C comparisons |
| `"a" + "b"` | Compile-time string literal folding |
| `prefix + suffix` | Bounded runtime concatenation when capacity is explicit |

String concatenation involving a variable requires a declared capacity:

```zlang
string message = "temp_" + "_01";          // folded literal
string[32] label = message + "sensor";     // bounded runtime concatenation
```

Without `string[32]`, the compiler reports `ZLANG2006`.

## Conditional chains

An `else` branch may contain either a block or another `if`. This makes chains direct and avoids
extra nesting in source and in generated C:

```zlang
if (value == 1) {
    print("one");
} else if (value == 2) {
    print("two");
} else {
    print("other");
}
```

The compiler emits a native `else if` chain. Chains of any length are represented as nested
`ifStatement` nodes.

## Logical operators and grouping

The unary `!` and the binary `&&` and `||` operators require boolean operands and emit native C
operators, preserving short-circuit behavior. Parentheses may group any expression:

```zlang
boolean allowed = !(ready && blocked) || override;
```

Precedence, from tightest to loosest, is grouping, `!`, comparisons and equality, `&&`, `||`, then
assignment.

## Diagnostics

| Code | Severity | Meaning |
| --- | --- | --- |
| `ZLANG2005` | error | Shadowed declaration |
| `ZLANG2006` | error | Explicit string capacity required |
| `ZLANG3002` | warning | Explicit sequence capacity is never mutated |
| `ZLANG3003` | error | Initializer is longer than the declared capacity |

Runtime trap codes are unchanged: `1` overflow, `2` bounds, `3` capacity and `4` invalid argument.
