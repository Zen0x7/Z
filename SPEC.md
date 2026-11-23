# Zlang Language Specification

**Status:** Draft 0.1  
**Scope:** the core language that the first version of `zlang-compile` must implement.

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

Zlang has exactly four value types in version 0.1.

### 7.1 `integer`

A signed 32-bit value. Emitted as `int32_t` from `<stdint.h>`.

### 7.2 `unsigned`

An unsigned 32-bit value. Emitted as `uint32_t` from `<stdint.h>`.

### 7.3 `boolean`

The values `true` and `false`. Emitted as `bool` from `<stdbool.h>`.

### 7.4 `string`

An immutable, non-owning view over UTF-8 bytes. It has two fields: a pointer to the first byte and a byte length. It never allocates and never owns memory.

A string literal is interned by the compiler and emitted in read-only storage. String concatenation is not an operation in Zlang, because it would require allocation.

### 7.5 Sequence values

A sequence is a fixed-capacity container written as `<type>[<capacity>]`. Capacity is part of the type, not a dynamic property.

```zlang
integer values[4];
string names[8];
```

The capacity must be a positive integer literal. `integer values[];` is illegal and produces a resource diagnostic.

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

A declaration introduces a compile-time symbol. The name is never emitted as a string. Generated C uses the same identifier unless it collides with a C keyword, in which case the compiler appends `_value` and records the mapping internally.

### 8.2 Initialization rules

- Scalars without an initializer are zero-initialized by the emitted C declaration.
- Strings without an initializer are the empty string.
- Sequences start with `length_ = 0` and uninitialized storage. Reading an element beyond the current length is a bounds trap.
- An initializer must have the exact declared type.

### 8.3 Scope

A name is visible from its declaration to the end of the enclosing block. Shadowing is not allowed; redeclaring a name in an inner block is a type diagnostic.

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
| `print(s)` | `zlang_print(s)` |
| `values[2]` | checked index, then `values.data_[2]` |
| `append(values, x)` | `CheckCapacity` then `values.data_[values.length_++] = x;` |
| `foreach (x in values)` | `for (uint32_t index = 0U; index < values.length_; ++index) { T x = values.data_[index]; ... }` |
| `if`, `while`, `repeat` | native C `if`, `while`, `for` |
| `function f(...)` | `static` C function with the same name |
| `return x;` | `return x;` |
| overflow, bounds, capacity failure | `zlang_trap(<fixed code>)` |

### 12.4 Runtime API

The only runtime functions that generated code may call in 0.1 are:

```c
bool zlang_print(zlang_string_t value);
int32_t zlang_add_i32(int32_t left, int32_t right);
uint32_t zlang_add_u32(uint32_t left, uint32_t right);
void zlang_array_append(void *data, uint32_t *length, uint32_t capacity,
                        size_t element_size, const void *value);
void zlang_trap(uint8_t code);
```

`zlang_array_append` implements `CheckCapacity`; checked indexing is emitted as
inline C with `zlang_trap(ZLANG_TRAP_BOUNDS)`. These helpers contain no
allocation, formatting, or dynamic dispatch.

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
               | type identifier "[" integerLiteral "]" ";" ;

type           = "integer" | "unsigned" | "boolean" | "string" ;

ifStatement    = "if" "(" expression ")" block
                 ( "else" block )? ;

whileStatement = "while" "(" expression ")" block ;

foreachStatement = "foreach" "(" identifier "in" identifier ")" block ;

repeatStatement  = "repeat" "(" expression ")" block ;

functionDeclaration = "function" identifier "(" parameters ")"
                      ( ":" type )? block ;

parameters     = ( type identifier ( "," type identifier )* )? ;

returnStatement = "return" expression? ";" ;

expression     = literal
               | identifier
               | expression "+" expression
               | expression "==" expression
               | identifier "=" expression
               | "print" "(" expression ")"
               | identifier "[" expression "]"
               | "append" "(" identifier "," expression ")" ;

literal        = integerLiteral | stringLiteral | "true" | "false" ;
```

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

## Annex C - Testability contract

A conformance test may assert on emitted C text without running the program:

```text
parse("integer var = 10;").transpile().contains("int32_t var = 10;")
```

```text
parse("integer values[];").diagnostics().contains("ZLANG3001")
```

These assertions are the primary way the language definition is kept honest: if the translation changes, the specification changes with it.
