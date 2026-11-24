# Zlang Language Specification

**Status:** Complete 0.2

**Scope:** the core language implemented by `zlang-compile` 0.2.

This document defines Zlang. It does not define the compiler internals, the command-line interface, or the CI pipeline. Those are implementation concerns and are described in `ROADMAP.md`.

The guiding rule of Zlang is:

> A program is a set of compile-time decisions that produces ordinary C. Nothing is decided at runtime that could have been decided at compile time.

---

## 1. Scope

Zlang is a source-to-source language. A Zlang program is translated to C99. The generated C calls a small, auditable runtime API named `zlang_*`, then a native toolchain produces an executable.

Zlang has no virtual machine, no intermediate bytecode, no runtime type metadata, and no garbage collector. Names, types, capacities, and storage locations exist only in the compiler.

## 2. Conformance

An implementation conforms to this specification if and only if it:

1. accepts every program described by the grammar in Annex A;
2. rejects every unbounded sequence at the declaration site with a resource diagnostic;
3. emits C that contains no call to `malloc`, `calloc`, `realloc`, `free`, or any equivalent allocation service;
4. emits native C control flow for all control structures, with no callback indirection;
5. makes the output of `--emit-c` byte-for-byte identical to the C that is compiled by default;
6. preserves the observable output order of `print` statements.

A conforming program is one that satisfies all static rules in clauses 6 through 11.

## 3. Normative references

- ISO/IEC 9899:1999 (C99), the target output language.
- `include/zlang/**`, the public C API that generated code may call.
- `AGENTS.md`, the repository conventions for naming and structure.

No external language standard is incorporated by reference. Zlang defines its own types, operations, and grammar.

## 4. Overview

Zlang intentionally resembles Java and PHP in readability. It uses braces, semicolons, and explicit types. It does not use PHP's `$` sigil, because a variable name is a compile-time symbol, not a runtime value.

A minimal program:

```zlang
integer var = 10;
print("Hello");
```

is translated to:

```c
#include <stdint.h>

int32_t var = 10;
zlang_print((zlang_string_t){ "Hello", 5U });
```

There is no `zlang_declare_var("var", ...)` call. Passing a source name as a string would turn a compile-time symbol into runtime data, which Zlang does not permit.

## 5. Notational conventions

- Metavariables are written in angle brackets: `<type>`, `<name>`, `<expression>`.
- "Must" denotes a requirement on a conforming implementation.
- "May" denotes an implementation-defined choice that must be documented.
- "Emits" denotes the exact C form in clause 12. Whitespace may differ; tokens may not.
- Line and column numbers are one-based and refer to the source file after comments are removed.

---

## 6. Source text and lexical grammar

### 6.1 Source text

A source file is UTF-8. Only ASCII characters are allowed in identifiers and keywords. String literals may contain any UTF-8 byte sequence except an unescaped double quote.

### 6.2 Lexical elements

| Element | Rule |
| --- | --- |
| Identifier | `[A-Za-z_][A-Za-z0-9_]*` |
| Integer literal | decimal digits, with optional `u` suffix for unsigned |
| String literal | `"..."` with escapes `\" \\ \n \t` |
| Comment | `//` to end of line, or `/* ... */` |
| Whitespace | space, tab, carriage return, line feed |

Keywords are reserved and cannot be used as identifiers:

```text
boolean break else false foreach function if integer
print repeat return string true unsigned while
```

### 6.3 Numeric literals

An integer literal without `u` has type `integer`. With `u`, it has type `unsigned`. A literal outside the range of its type is a type diagnostic.

---

## 7. Values and types

Zlang has exactly four value types through version 0.2.

### 7.1 `integer`

A signed 32-bit value. Emitted as `int32_t` from `<stdint.h>`.

### 7.2 `unsigned`

An unsigned 32-bit value. Emitted as `uint32_t` from `<stdint.h>`.

### 7.3 `boolean`

The values `true` and `false`. Emitted as `bool` from `<stdbool.h>`.

### 7.4 `string`

An immutable, non-owning view over UTF-8 bytes. It has two fields: a pointer to the first byte and a byte length. It never allocates and never owns memory.

A string literal is interned by the compiler and emitted in read-only storage. String concatenation is supported at language level 0.2, but it never allocates. When every operand is a literal, the compiler folds the result into one emitted literal. When any operand is a variable, the declaration must use explicit capacity syntax, the compiler reserves a fixed backing buffer, and `zlang_string_concat` copies the views into it. Overflow traps instead of growing the buffer.

### 7.5 Sequence values

A sequence is a fixed-capacity container written as `<type>[<capacity>]`. Capacity is part of the type, not a dynamic property.

```zlang
integer values[4];
string names[8];
```

The capacity must be a positive integer literal. `integer values[];` is illegal and produces a resource diagnostic. At language level 0.2 a sequence may also be initialized with a brace list; the compiler infers the capacity from the number of elements. An explicit capacity may be combined with a shorter initializer, in which case capacity is unchanged and length is the number of initializer elements. `integer values[] = {};` is rejected with `ZLANG3001`, and an initializer longer than the declared capacity is rejected with `ZLANG3003`.

A sequence has a current length that starts at zero and never exceeds its capacity. Its storage is decided at compile time:

- total size at or below `ZLANG_STACK_LIMIT` (default `4096` bytes) is emitted as a local C aggregate;
- larger sequences are emitted as `static` storage with internal linkage;
- the compiler must document the threshold used when it differs from the default.

### 7.6 Type compatibility

There is no implicit conversion between `integer` and `unsigned`, or between numeric types and `boolean`. A string can only be used where a `string` is expected. Sequence types with different capacities are different types.

---

## 8. Declarations and storage

### 8.1 Variable declarations

```zlang
<type> <name> [= <expression>] ;
```

Examples:

```zlang
integer count = 10;
boolean ready = true;
string greeting = "Hello";
integer values[4];
```

A declaration introduces a compile-time symbol. The name is never emitted as a string. Generated C uses the same source identifier when it is available; otherwise the compiler appends a numeric suffix and records the mapping internally. Sibling scopes may reuse the same source name without colliding in generated C.

### 8.2 Initialization rules

- Scalars without an initializer are zero-initialized by the emitted C declaration.
- Strings without an initializer are the empty string.
- Sequences start with `length_ = 0` and uninitialized storage unless a brace initializer is present.
- A sequence initializer sets `length_` to the initializer element count and zero-fills the remaining declared capacity.
- Reading an element beyond the current length is a bounds trap.
- An initializer must have the exact declared type.

### 8.3 Scope

A name is visible from its declaration to the end of the enclosing block. Shadowing is not allowed; redeclaring a visible name in an inner block is `ZLANG2005`, while redeclaring it in the same scope is `ZLANG2002`.

---

## 9. Abstract operations

These operations define meaning. They are not functions that a program can call.

### 9.1 `CheckCapacity(sequence, amount)`

If `sequence.length_ + amount` exceeds the declared capacity, invoke `Trap(ZLANG_TRAP_CAPACITY)`. Otherwise return the old length.

### 9.2 `CheckIndex(sequence, index)`

If `index` is greater than or equal to `sequence.length_`, invoke `Trap(ZLANG_TRAP_BOUNDS)`. Otherwise return the element at `index`.

### 9.3 `CheckArithmetic(operation, left, right)`

Perform the operation with overflow detection. On overflow, invoke `Trap(ZLANG_TRAP_OVERFLOW)`. Otherwise return the result.

### 9.4 `Trap(code)`

Terminate the process by calling `zlang_trap(code)`. The runtime must not construct a message, allocate memory, or perform output. Trap codes are fixed:

| Code | Name |
| --- | --- |
| `1` | `ZLANG_TRAP_OVERFLOW` |
| `2` | `ZLANG_TRAP_BOUNDS` |
| `3` | `ZLANG_TRAP_CAPACITY` |
| `4` | `ZLANG_TRAP_INVALID_ARGUMENT` |

### 9.5 `EmitForEach(node)`

Validate that the iterable is a sequence and that the iteration variable has the sequence element type. Emit a native C `for` loop over indices from `0` to `length_ - 1`. The loop must not become a runtime callback.

---

## 10. Statements and control flow

### 10.1 Expression statements

An expression followed by `;`. The only expression statements in 0.1 are assignment and `print`.

### 10.2 `if`

```zlang
if (<boolean-expression>) {
    <statements>
} else {
    <statements>
}
```

The `else` clause is optional. The condition must be `boolean`.

### 10.3 `while`

```zlang
while (<boolean-expression>) {
    <statements>
}
```

### 10.4 `foreach`

```zlang
foreach (<name> in <sequence>) {
    <statements>
}
```

The iteration variable is a fresh local of the sequence element type. It is a copy; assigning to it does not modify the sequence.

### 10.5 `repeat`

```zlang
repeat (<count-expression>) {
    <statements>
}
```

Equivalent to a counted loop from `0` to `count - 1`. The expression must be `unsigned`.

### 10.6 `return`

Valid only inside a function. The expression type must match the declared return type.

---

## 11. Functions

```zlang
function <name>(<parameter-list>): <return-type> {
    <statements>
}
```

A parameter is `<type> <name>`. Parameters are passed by value. Functions have no access to mutable globals, because Zlang 0.1 has no global variables.

```zlang
function add(integer left, integer right): integer {
    return left + right;
}
```

A function with no return type returns nothing and emits `void`.

---

## 12. Translation to C

This clause is normative. Each row states the exact C form for a Zlang construct.

### 12.1 Prologue

Every generated C file begins with:

```c
#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>
```

### 12.2 Type mapping

| Zlang | Emitted C |
| --- | --- |
| `integer` | `int32_t` |
| `unsigned` | `uint32_t` |
| `boolean` | `bool` |
| `string` | `zlang_string_t` |
| `T[N]` | generated aggregate `{ T data_[N]; uint32_t length_; }` |

### 12.3 Construct mapping

| Zlang | Emitted C |
| --- | --- |
| `integer var = 10;` | `int32_t var = 10;` |
| `unsigned n = 2u;` | `uint32_t n = 2U;` |
| `boolean ready = true;` | `bool ready = true;` |
| `string s = "Hi";` | `zlang_string_t s = { "Hi", 2U };` |
| `a + b` | checked addition, then plain `a + b` |
| `a == b` | `a == b` |
| `a < b`, `a > b`, `a <= b`, `a >= b`, `a != b` | native C comparison |
| `!a`, `a && b`, `a || b` | native C logical operators with short-circuit behavior |
| `f(a, b)` | direct C call to the emitted function |
| `"a" + "b"` | folded string literal |
| `prefix + suffix` with explicit capacity | `zlang_string_concat(storage, capacity, parts, count)` |
| `print(s)` | `zlang_print(s)` |
| `values[2]` | checked index, then `values.data_[2]` |
| `values[0] = x` | checked index statement, then direct store to `values.data_[0]` |
| `append(values, x)` | `CheckCapacity` then `values.data_[values.length_++] = x;` |
| `foreach (x in values)` | `for (uint32_t index = 0U; index < values.length_; ++index) { T x = values.data_[index]; ... }` |
| `if`, `while`, `repeat` | native C `if`, `while`, `for` |
| `function f(...)` | `static` C function with the same name |
| `return x;` | `return x;` |
| overflow, bounds, capacity failure | `zlang_trap(<fixed code>)` |

### 12.4 Runtime API

The runtime functions that generated code may call through language level 0.2 are:

```c
bool zlang_print(zlang_string_t value);
int32_t zlang_add_i32(int32_t left, int32_t right);
uint32_t zlang_add_u32(uint32_t left, uint32_t right);
void zlang_array_append(void *data, uint32_t *length, uint32_t capacity,
                        size_t element_size, const void *value);
zlang_string_t zlang_string_concat(char *storage, uint32_t capacity,
                                   const zlang_string_t *parts, size_t count);
void zlang_trap(uint8_t code);
```

`zlang_array_append` implements `CheckCapacity`; checked indexing and indexed
stores use inline C with `zlang_trap(ZLANG_TRAP_BOUNDS)`. `zlang_string_concat`
copies into caller-owned storage and traps on overflow or insufficient
capacity. These helpers contain no allocation, formatting, or dynamic dispatch.

Any additional `zlang_*` call requires an amendment to this specification.

### 12.5 Prohibited emissions

Generated C must not contain:

- a function pointer table for dispatch;
- a closure or callback passed to a runtime helper;
- a string literal containing a source variable name;
- `malloc`, `calloc`, `realloc`, `free`, `strdup`, or equivalent;
- a runtime type lookup by name.

---

## 13. Diagnostics

Diagnostics use the form:

```text
file:line:column: error[ZLANGxxxx]: message
```

Error classes:

| Range | Class |
| --- | --- |
| `ZLANG1xxx` | Syntax |
| `ZLANG2xxx` | Type |
| `ZLANG3xxx` | Resource and allocation |

The stable codes are `ZLANG1001` unexpected token, `ZLANG1002` unterminated
string, `ZLANG1003` invalid character, `ZLANG1004` unsupported construct,
`ZLANG2001` literal out of range, `ZLANG2002` duplicate declaration,
`ZLANG2003` undefined identifier, `ZLANG2004` type mismatch,
`ZLANG2005` shadowed declaration, `ZLANG2006` string capacity required,
`ZLANG3001` unbounded sequence, `ZLANG3002` unused capacity, and
`ZLANG3003` initializer too long. `ZLANG3002` is a warning: it is printed with
the `warning[...]` prefix and does not stop compilation.

Resource errors point at the declaration that needs a limit, not at the later use.

```text
main.zlang:3:9: error[ZLANG3001]: unbounded sequence 'values' requires an explicit capacity.
```

The compiler may emit more than one diagnostic, but it must not generate C for a program that has any error.

---

## 14. Non-goals

The following are explicitly outside Zlang 0.1:

- runtime reflection, dynamic dispatch, or runtime names;
- classes, inheritance, and objects;
- string concatenation or any operation that grows storage;
- garbage collection or reference counting;
- hard realtime timing guarantees beyond predictable memory behavior;
- module imports; each compilation unit is one source file.

---

## Annex A - Grammar summary

```text
program        = statement* ;

statement      = declaration
               | expression ";"
               | ifStatement
               | whileStatement
               | foreachStatement
               | repeatStatement
               | functionDeclaration
               | returnStatement
               | block ;

block          = "{" statement* "}" ;

declaration    = type identifier ( "=" expression )? ";"
               | type identifier "[" integerLiteral? "]"
                 ( "=" sequenceInitializer )? ";"
               | type "[" integerLiteral "]" identifier
                 ( "=" expression )? ";" ;

sequenceInitializer = "{" ( expression ( "," expression )* )? "}" ;

type           = "integer" | "unsigned" | "boolean" | "string" ;

ifStatement    = "if" "(" expression ")" block
                 ( "else" ( block | ifStatement ) )? ;

whileStatement = "while" "(" expression ")" block ;

foreachStatement = "foreach" "(" identifier "in" identifier ")" block ;

repeatStatement  = "repeat" "(" expression ")" block ;

functionDeclaration = "function" identifier "(" parameters ")"
                      ( ":" type )? block ;

parameters     = ( type identifier ( "," type identifier )* )? ;

returnStatement = "return" expression? ";" ;

expression     = literal
               | identifier
               | "(" expression ")"
               | "!" expression
               | expression "&&" expression
               | expression "||" expression
               | expression "+" expression
               | expression ( "==" | "!=" | "<" | ">" | "<=" | ">=" ) expression
               | identifier "=" expression
               | identifier "[" expression "]" "=" expression
               | identifier "(" ( expression ( "," expression )* )? ")"
               | "print" "(" expression ")"
               | identifier "[" expression "]"
               | "append" "(" identifier "," expression ")" ;

literal        = integerLiteral | stringLiteral | "true" | "false" ;
```

Logical operators require boolean operands and follow C precedence: `!` binds tighter than
comparisons, `&&` binds tighter than `||`, and assignment has the lowest precedence. Parentheses
may group any expression. An `else if` chain is represented as nested `ifStatement` nodes and is
emitted as native `else if` chains.

## Annex B - Worked translation example

Source:

```zlang
integer var = 10;
integer values[4];

foreach (item in values) {
    print("Hello");
}
```

Expected emitted C, ignoring whitespace:

```c
#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

int32_t var = 10;

struct zlang_sequence_i32_4_t {
    int32_t data_[4];
    uint32_t length_;
};

struct zlang_sequence_i32_4_t values = { { 0, 0, 0, 0 }, 0U };

int main(void) {
    for (uint32_t index = 0U; index < values.length_; ++index) {
        int32_t item = values.data_[index];
        (void)item;
        zlang_print((zlang_string_t){ "Hello", 5U });
    }
    return 0;
}
```

The iteration exists as a `for_each` node in the compiler's intermediate representation. The emitted binary contains only the native loop; no callback survives.

## 0.1 implementation status

The 0.1 implementation is complete:

- lexer, parser, AST, and source locations;
- semantic analysis with scopes, type checks, and resource diagnostics;
- memory planner with a default 4096-byte stack limit;
- checked C emitter with native control flow;
- bounded runtime API with fixed trap codes;
- `zlang-compile` with `--emit-c`.

Diagnostics use the exact format in clause 13. Every diagnostic carries the
line and column of the source construct that caused it.

## 0.2 implementation status

The 0.2 implementation adds lexical comments, lexical scopes with unique generated
C identifiers, sequence brace initializers, checked index assignment, bounded
string concatenation, direct function calls, ordering and inequality operators,
logical operators, expression grouping, and `else if` chains. It rejects shadowing with
`ZLANG2005`, rejects variable concatenation without explicit capacity with `ZLANG2006`,
rejects overlong sequence initializers with `ZLANG3003`, and warns about undisciplined
explicit capacity with `ZLANG3002`.

## Annex C - Testability contract

A conformance test may assert on emitted C text without running the program:

```text
parse("integer var = 10;").transpile().contains("int32_t var = 10;")
```

```text
parse("integer values[];").diagnostics().contains("ZLANG3001")
```

These assertions are the primary way the language definition is kept honest: if the translation changes, the specification changes with it.

## Annex D - Conformance matrix

Every Annex A production and every diagnostic is covered by a CTest case.
The matrix is maintained in `tests/spec/test_cases.h`.

| Annex A production or rule | Conformance test |
| --- | --- |
| `program`, `declaration`, `type`, scalar initializer | `emits_a_scalar_declaration` |
| all four `type` alternatives | `covers_every_annex_a_production` |
| `expression ";"` and `identifier` expression | `covers_every_annex_a_production` |
| `block` with nested declarations | `covers_every_annex_a_production` |
| `ifStatement` with `else` | `emits_if_and_else` |
| `ifStatement` without `else` | `covers_every_annex_a_production` |
| `whileStatement` | `covers_every_annex_a_production` |
| `foreachStatement` | `emits_a_native_foreach_loop` |
| `repeatStatement` | `emits_repeat_as_native_for` |
| `functionDeclaration` with parameters and return type | `emits_function_and_return` |
| `functionDeclaration` with empty parameters and no return type | `covers_every_annex_a_production` |
| `parameters` | `emits_function_and_return` |
| `returnStatement` with expression | `emits_function_and_return` |
| `returnStatement` without expression | `covers_every_annex_a_production` |
| `expression "+" expression` | `emits_checked_addition` |
| `expression "==" expression` | `covers_every_annex_a_production` |
| `identifier "=" expression` | `covers_every_annex_a_production` |
| `"print" "(" expression ")"` | `emits_a_print_call` |
| `identifier "[" expression "]"` | `emits_append_and_checked_index` |
| `"append" "(" identifier "," expression ")"` | `emits_append_and_checked_index` |
| integer, string, and boolean literals | `covers_every_annex_a_production` |
| sequence declaration with capacity | `uses_static_storage_for_sequences_over_the_stack_limit` |
| comments and escaped string literals | `skips_comments_and_unescapes_strings` |
| `ZLANG1001` unexpected token | `reports_syntax_diagnostics_with_source_locations` |
| `ZLANG1002` unterminated string | `reports_syntax_diagnostics_with_source_locations` |
| `ZLANG1003` invalid character | `reports_syntax_diagnostics_with_source_locations` |
| `ZLANG1004` unsupported construct | `reports_syntax_diagnostics_with_source_locations` |
| `ZLANG2001` literal out of range | `reports_type_and_resource_diagnostics_with_source_locations` |
| `ZLANG2002` duplicate declaration | `reports_type_and_resource_diagnostics_with_source_locations` |
| `ZLANG2003` undefined identifier | `reports_type_and_resource_diagnostics_with_source_locations` |
| `ZLANG2004` type mismatch | `reports_type_and_resource_diagnostics_with_source_locations` |
| `ZLANG3001` unbounded sequence | `rejects_an_unbounded_sequence_at_the_declaration`, `rejects_an_empty_inferred_sequence` |
| `ZLANG2005` shadowed declaration | `rejects_a_shadowed_declaration` |
| `ZLANG2006` string capacity required | `rejects_variable_concatenation_without_capacity` |
| `ZLANG3002` unused capacity warning | `warns_about_unused_explicit_capacity` |
| `ZLANG3003` initializer too long | `rejects_an_initializer_longer_than_capacity` |
| sequence initializer and inferred capacity | `emits_an_inferred_sequence_initializer`, `emits_a_partial_explicit_sequence_initializer` |
| checked index assignment | `emits_index_assignment_with_a_checked_store` |
| literal string concatenation | `folds_literal_string_concatenation` |
| bounded variable string concatenation | `emits_bounded_variable_string_concatenation` |
| direct function call | `emits_direct_function_calls` |
| ordering and inequality operators | `emits_ordering_and_inequality_operators` |
| `else ifStatement` | `emits_else_if_chains` |
| `!`, `&&`, `||` and grouped expressions | `emits_logical_operators` |
| boolean operand checking | `rejects_non_boolean_logical_operands` |
| sibling scope identifier generation | `renames_sibling_scope_declarations` |
| diagnostic source locations | `reports_syntax_diagnostics_with_source_locations`, `reports_type_and_resource_diagnostics_with_source_locations`, `zlang.cli.diagnostic_format` |
