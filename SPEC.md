# Zlang Language Specification

**Status:** Current

**Scope:** the core language implemented by `zc`.

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
2. rejects every unbounded array at the declaration site with a resource diagnostic;
3. emits C that contains no call to `malloc`, `calloc`, `realloc`, `free`, or any equivalent allocation service;
4. emits native C control flow for all control structures, with no callback indirection;
5. makes the output of `--emit-c` byte-for-byte identical to the C that is compiled by default;
6. preserves the observable output order of `write` calls.

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
int32 var = 10;
write("Hello");
```

is translated to:

```c
#include <stdint.h>

int32_t var = 10;
zlang_std_io_write((zlang_string_t){ "Hello", 5U });
```

There is no `zlang_declare_var("var", ...)` call. Passing a source name as a string would turn a compile-time symbol into runtime data, which Zlang does not permit.

## 5. Notational conventions

- Metavariables are written in angle brackets: `<type>`, `<name>`, `<expression>`.
- "Must" denotes a requirement on a conforming implementation.
- "May" denotes an implementation-defined choice that must be documented.
- "Emits" denotes the exact C form in clause 12. Whitespace may differ; tokens may not.
- Line and column numbers are one-based and refer to the original source file. Comments do not
  affect parsing, but the emitter preserves them in the generated C for traceability.

---

## 6. Source text and lexical grammar

### 6.1 Source text

A source file is UTF-8. Only ASCII characters are allowed in identifiers and keywords. String literals may contain any UTF-8 byte sequence except an unescaped double quote.

### 6.2 Lexical elements

| Element | Rule |
| --- | --- |
| Identifier | `[A-Za-z_][A-Za-z0-9_]*` |
| Integer literal | decimal digits with optional `u`, `i64` or `u64` suffix |
| Floating literal | decimal digits with `.` or exponent, optional `f` suffix |
| String literal | `"..."` with escapes `\" \\ \n \t` |
| Comment | `//` to end of line, or `/* ... */` |
| Whitespace | space, tab, carriage return, line feed |

Keywords are reserved and cannot be used as identifiers:

```text
boolean break clone const continue double else enum false float for foreach function if int32
int64 length struct reference repeat return sequence string true uint64 uint32 while
```

The generated-code namespace is also reserved. An identifier beginning with `zlang`, `_zlang` or
`ZLANG` is rejected with `ZLANG1004`.

### 6.3 Numeric literals

An integer literal without a suffix has type `int32`. With `u`, it has type `uint32`. With
`i64`, it has type `int64`. With `u64`, it has type `uint64`. A floating literal has type `double`;
with the `f` suffix it has type `float`. A literal outside the range of its type is a diagnostic.

---

## 7. Values and types

Zlang has eight numeric and scalar primitive types, plus user-defined struct and enum types.

### 7.1 `int32`

A signed 32-bit value. Emitted as `int32_t` from `<stdint.h>`.

### 7.2 `uint32`

An uint32 32-bit value. Emitted as `uint32_t` from `<stdint.h>`.

### 7.3 `int64`

A signed 64-bit value. Emitted as `int64_t` from `<stdint.h>`.

### 7.4 `uint64`

An uint32 64-bit value. Emitted as `uint64_t` from `<stdint.h>`.

### 7.5 `float`

An IEEE-754 binary32 value. Emitted as `float`, with `f`-suffixed literals.

### 7.6 `double`

An IEEE-754 binary64 value. Emitted as `double`.

### 7.7 `boolean`

The values `true` and `false`. Emitted as `bool` from `<stdbool.h>`.

### 7.8 `string`

An immutable, non-owning view over UTF-8 bytes. It has two fields: a pointer to the first byte and a byte length. It never allocates and never owns memory.

A string literal is interned by the compiler and emitted in read-only storage. String concatenation is supported, but it never allocates. When every operand is a literal, the compiler folds the result into one emitted literal. When any operand is a variable, the declaration must use explicit capacity syntax, the compiler reserves a fixed backing buffer, and `zlang_string_concat` copies the views into it. Overflow traps instead of growing the buffer.

### 7.9 Sequence values

An array is a fixed-shape container written as `<type>[<capacity>]`. Capacity is part of the type, not a dynamic property.

```zlang
array[4] int32 values;
array[8] string[32] names;
```

The capacity must be a positive integer literal. `array[] int32 values;` is illegal and produces a resource diagnostic. A sequence may also be initialized with a brace list; the compiler infers the capacity from the number of elements. An explicit capacity may be combined with a shorter initializer, in which case capacity is unchanged and length is the number of initializer elements. `array[] int32 values = {};` is rejected with `ZLANG3001`, and an initializer longer than the declared capacity is rejected with `ZLANG3003`.

An array has no logical length. Every extent is fixed at compile time and every index is bounds checked. Its storage is decided at compile time:

- total size at or below `ZLANG_STACK_LIMIT` (default `4096` bytes) is emitted as a local C aggregate;
- larger arrays are emitted as `static` storage with internal linkage;
- the compiler must document the threshold used when it differs from the default.

### 7.10 Type compatibility

There is no implicit conversion between numeric types, or between numeric types and `boolean`.
Binary arithmetic and comparisons require both operands to have exactly the same type. Explicit
conversion calls are `to_int32`, `to_uint32`, `to_int64`, `to_uint64`, `to_float` and
`to_double`; they take exactly one numeric operand. Conversion to an int32 type traps when the
value is out of range, and conversion from a floating-point value truncates toward zero and traps
on non-finite input. Floating arithmetic follows IEEE-754, so division by zero produces an infinity
or NaN instead of a Zlang trap. `%` is valid only for integral types. Unary negation of `int32`
and `int64` traps on the signed minimum value. String `==` and `!=` compare content through the
runtime helper; string ordering is rejected. A string can only be used where a `string` is
expected. Array types with different extents are different types. A struct or enum type matches only the same user-defined name, including in function parameters, return values and variable initializers.

---

## 8. Declarations and storage

### 8.1 Variable declarations

```zlang
<type> <name> [= <expression>] ;
```

Examples:

```zlang
int32 count = 10;
boolean ready = true;
string greeting = "Hello";
array[4] int32 values;
```

A declaration introduces a compile-time symbol. The name is never emitted as a string. Generated C uses the same source identifier when it is available; otherwise the compiler appends a numeric suffix and structs the mapping internally. Sibling scopes may reuse the same source name without colliding in generated C.

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
| `5` | `ZLANG_TRAP_DIVISION_BY_ZERO` |

### 9.5 `EmitForEach(node)`

Validate that the iterable is a sequence and that the iteration variable has the array element type. Emit a native C `for` loop over indices from `0` to `length_ - 1`. The loop must not become a runtime callback.

---

## 10. Statements and control flow

### 10.1 Expression statements

An expression followed by `;`. Expression statements include assignment, indexed assignment,
`write` from `@std/io` and calls to user-defined functions.

### 10.2 `if`

```zlang
if (<boolean-expression>) {
    <statements>
} else {
    <statements>
}
```

The `else` clause is optional. An `else` branch may contain another `if`, forming an `else if`
chain. The condition must be `boolean`.

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

The iteration variable is a fresh local of the array element type. It is a copy; assigning to it does not modify the sequence.

### 10.5 `repeat`

```zlang
repeat (<count-expression>) {
    <statements>
}
```

Equivalent to a counted loop from `0` to `count - 1`. The expression must be `uint32`.

### 10.6 `return`

Valid only inside a function. A non-void function must return a value whose type matches the
declared return type. A void function must return without a value. A `return` outside a function is
`ZLANG1004`.

### 10.7 `break` and `continue`

```zlang
break;
continue;
```

Both statements are valid only inside `while`, `repeat` or `foreach`. They emit native C `break;`
and `continue;` and are `ZLANG1004` elsewhere.

---

## 11. Functions

```zlang
function <name>(<parameter-list>): <return-type> {
    <statements>
}
```

A parameter is `<const?> <mode> <type> <name>`, where `<mode>` is `clone` or `reference`, and
`<type>` is a scalar or a previously declared struct or enum name. The return type uses the same set
of types, without a mode. Struct parameters and return values lower to generated C typedefs.
Functions have no access to mutable globals, because Zlang has no global variables.

```zlang
function add(clone int32 left, clone int32 right): int32 {
    return left + right;
}
```

`clone` passes a value copy; mutation affects only the local parameter. `reference` passes an alias
to the caller's lvalue; mutation is visible to the caller. `const clone` is a read-only copy and
`const reference` is a read-only alias. A mutable `reference` cannot bind a const argument, and
reference arguments must be lvalues. References are parameter-only: they cannot be stored or
returned.

A function with no return type returns nothing and emits `void`. A function with no parameters emits
`(void)` in C. Unused parameters receive `(void)` guards in the generated body.

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
| `int32` | `int32_t` |
| `uint32` | `uint32_t` |
| `boolean` | `bool` |
| `string` | `zlang_string_t` |
| `array[N] T name` | generated typedef `zlang_name_t` with a `ZLANG_NAME_CAPACITY` macro |

### 12.3 Construct mapping

| Zlang | Emitted C |
| --- | --- |
| `int32 var = 10;` | `int32_t var = 10;` |
| `uint32 n = 2u;` | `uint32_t n = 2U;` |
| `boolean ready = true;` | `bool ready = true;` |
| `string s = "Hi";` | `zlang_string_t s = { "Hi", 2U };` |
| `a + b` | checked addition, then plain `a + b` |
| `-a`, `a - b`, `a * b`, `a / b`, `a % b` | checked arithmetic helpers |
| `a += b` and other compound assignments | checked operation followed by assignment |
| `length(values)` | native `values.length_` as `uint32_t` |
| `a == b` for scalars | `a == b` |
| `a == b` for strings | `zlang_string_equal(a, b)` |
| `a < b`, `a > b`, `a <= b`, `a >= b`, `a != b` | native C comparison |
| `!a`, `a && b`, `a || b` | native C logical operators with short-circuit behavior |
| `f(a, b)` | direct C call to the emitted function |
| `"a" + "b"` | folded string literal |
| `prefix + suffix` with explicit capacity | `zlang_string_concat(storage, capacity, parts, count)` |
| `write(s)` | `zlang_std_io_write(s)` |
| `write(i)`, `write(u)`, `write(b)` | bounded scalar write helpers |
| `values[2]` | bounded typed getter such as `zlang_array_get_i32(values.data_, 2U, values.length_)` |
| `text[0]` | `zlang_string_byte(text, 0U)` |
| `values[0] = x` | checked index statement, then direct store to `values.data_[0]` |
| `foreach (x in values)` | `for (uint32_t _zlang_foreach_N = ZLANG_ZERO; ... ) { T _x = values.data_[_zlang_foreach_N]; ... }` |
| `if`, `while`, `repeat` | native C `if`, `while`, `for` |
| `function f(...)` | C function `_u_f(...)` |
| `return x;` | `return x;` |
| overflow, bounds, capacity, division-by-zero failure | `zlang_trap(<fixed code>)` |

Generated C is laid out in this order: includes, generated value constants (`ZLANG_ZERO`,
`ZLANG_ONE`, `ZLANG_TWO`), named capacity and initial-length macros, generated sequence typedefs,
named string literal constants, user functions and `main`. Local source symbols are emitted with a
leading `_`, function parameters keep their source names, struct members use a trailing `_`, and
user functions use `_u_`. Compiler-generated temporaries use `_zlang_`, generated macros use
`ZLANG_`, generated types use `zlang_` and runtime calls use `zlang_`. Only source blocks emit extra
braces; compiler temporaries are declared in the enclosing C block. A reference to a sequence
element is resolved once per context: the first field access materializes a checked pointer and later
accesses to the same element reuse it until an index dependency changes or the scope ends. An index
read repeated across an `if` chain and its branch bodies is materialized once and reused there. Trap
sites use the public `ZLANG_TRAP_*` constants rather than numeric literals.

### 12.4 Runtime API

The runtime functions that generated code may call are:

```c
void *zlang_array_get(void *data, uint32_t index, uint32_t length, size_t element_size);
bool zlang_std_io_write(zlang_string_t value);
int32_t zlang_add_i32(int32_t left, int32_t right);
uint32_t zlang_add_u32(uint32_t left, uint32_t right);
int32_t zlang_sub_i32(int32_t left, int32_t right);
uint32_t zlang_sub_u32(uint32_t left, uint32_t right);
int32_t zlang_mul_i32(int32_t left, int32_t right);
uint32_t zlang_mul_u32(uint32_t left, uint32_t right);
int32_t zlang_div_i32(int32_t left, int32_t right);
uint32_t zlang_div_u32(uint32_t left, uint32_t right);
int32_t zlang_mod_i32(int32_t left, int32_t right);
uint32_t zlang_mod_u32(uint32_t left, uint32_t right);
int32_t zlang_array_get_i32(const int32_t *data, uint32_t index, uint32_t length);
uint32_t zlang_array_get_u32(const uint32_t *data, uint32_t index, uint32_t length);
bool zlang_array_get_bool(const bool *data, uint32_t index, uint32_t length);
zlang_string_t zlang_array_get_string(const zlang_string_t *data, uint32_t index,
                                      uint32_t length);
uint32_t zlang_string_byte(zlang_string_t value, uint32_t index);
bool zlang_std_io_write_i32(int32_t value);
bool zlang_std_io_write_u32(uint32_t value);
bool zlang_std_io_write_bool(bool value);
void zlang_array_append(void *data, uint32_t *length, uint32_t capacity,
                        size_t element_size, const void *value);
zlang_string_t zlang_string_concat(char *storage, uint32_t capacity,
                                   const zlang_string_t *parts, size_t count);
zlang_string_t zlang_string_array_append(char *storage, zlang_string_t *data,
                                         uint32_t *length, uint32_t capacity,
                                         uint32_t element_capacity,
                                         const zlang_string_t *parts, size_t count);
void zlang_trap(uint8_t code);
```

checked indexing and indexed
stores use inline C with `zlang_trap(ZLANG_TRAP_BOUNDS)`. `zlang_string_concat`
copies into caller-owned storage and traps on overflow or insufficient
capacity. These helpers contain no allocation, formatting, or dynamic dispatch.

Any additional `zlang_*` call requires an amendment to this specification.

### 12.5 Prohibited emissions

Generated C must not contain:

- a function pointer table except the internal vtables required by 0.29 inheritance;
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
`ZLANG3001` unbounded array, `ZLANG3002` unused capacity, and
`ZLANG3003` initializer too long. `ZLANG3004` warns about an unnecessary reference parameter.
Legacy postfix array declarations use `ZLANG1004`.
`ZLANG3002` is a warning: it is printed with
the `warning[...]` prefix and does not stop compilation.

Resource errors point at the declaration that needs a limit, not at the later use.

```text
main.z:3:9: error[ZLANG3001]: unbounded array 'values' requires an explicit capacity.
```

The compiler may emit more than one diagnostic, but it must not generate C for a program that has any error.

---

## 14. Non-goals

The following are explicitly outside Zlang:

- runtime reflection or runtime names;
- classes, multiple inheritance, or heap-backed objects;
- heap-backed strings or vectors, or any operation that grows storage dynamically;
- garbage collection or reference counting;
- hard realtime timing guarantees beyond predictable memory behavior;
- standard library imports use canonical `@std/<domain>` specifiers; user modules use relative
  paths or `@alias/...` paths declared in `z.yaml`.

---

## Annex A - Grammar summary

```text
program        = statement* ;

statement      = declaration
               | expression ";"
               | "break" ";"
               | "continue" ";"
               | recordDeclaration
               | enumDeclaration
               | ifStatement
               | whileStatement
               | forStatement
               | foreachStatement
               | repeatStatement
               | functionDeclaration
               | returnStatement
               | block ;

block          = "{" statement* "}" ;

declaration    = "const"? arrayModifier? typeName typeModifier? identifier
                 ( "=" expression )? ";" ;

typeName       = builtinType | identifier ;
builtinType    = "int32" | "uint32" | "boolean" | "string" ;

recordDeclaration = "struct" identifier "{" declaration* "}" ;

enumDeclaration = "enum" identifier "{" identifier ( "," identifier )* "}" ;

arrayModifier = "array" ( "[" integerLiteral "]" )* ;
typeModifier     = "[" integerLiteral "]" ;

arrayInitializer = "{" ( expression | arrayInitializer ) ( "," ( expression | arrayInitializer ) )* "}" ;

type           = typeName ;

ifStatement    = "if" "(" expression ")" block
                 ( "else" ( block | ifStatement ) )? ;

whileStatement = "while" "(" expression ")" block ;

forStatement   = "for" "(" ( declaration | expression? ) ";"
                 expression ";" expression ")" block ;

foreachStatement = "foreach" "(" identifier ( "," identifier )* "in"
                   identifier ")" block ;

repeatStatement  = "repeat" "(" expression ")" block ;

functionDeclaration = "function" identifier "(" parameters ")"
                      ( ":" type )? block ;

parameterMode  = "clone" | "reference" ;
parameter      = "const"? parameterMode ( "array" | type ) identifier ;
parameters     = ( parameter ( "," parameter )* )? ;

returnStatement = "return" expression? ";" ;

expression     = literal
               | identifier
               | "(" expression ")"
               | "!" expression
               | "-" expression
               | expression "&&" expression
               | expression "||" expression
               | expression ( "+" | "-" | "*" | "/" | "%" ) expression
               | expression ( "==" | "!=" | "<" | ">" | "<=" | ">=" ) expression
               | identifier "=" expression
               | identifier ( "+=" | "-=" | "*=" | "/=" | "%=" ) expression
               | identifier "[" expression "]" "=" expression
               | identifier "(" ( expression ( "," expression )* )? ")"
               | "length" "(" identifier ")"
               | identifier "[" expression "]"
               | expression "." identifier ;

literal        = integerLiteral | stringLiteral | "true" | "false" ;
```

Logical operators require boolean operands and follow C precedence: `!` binds tighter than
comparisons, `&&` binds tighter than `||`, and assignment has the lowest precedence. Parentheses
may group any expression. An `else if` chain is represented as nested `ifStatement` nodes and is
emitted as native `else if` chains.

## Annex B - Worked translation example

Source:

```zlang
int32 var = 10;
array[4] int32 values;

foreach (item in values) {
    write("Hello");
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
        zlang_std_io_write((zlang_string_t){ "Hello", 5U });
    }
    return 0;
}
```

The iteration exists as a `for_each` node in the compiler's intermediate representation. The emitted binary contains only the native loop; no callback survives.

## Implementation status

The language described here is implemented across the lexer, parser, semantic analyzer, memory
planner, C emitter, runtime and `zc` driver. Diagnostics use the exact format in
clause 13, and every diagnostic carries the line and column of the source construct that caused
it. Version-by-version history is kept in `ROADMAP.md`.

## Annex C - Testability contract

A conformance test may assert on emitted C text without running the program:

```text
parse("int32 var = 10;").transpile().contains("int32_t var = 10;")
```

```text
parse("array[] int32 values;").diagnostics().contains("ZLANG3001")
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
| `import { write } from "@std/io"` | `emits_a_write_call` |
| `identifier "[" expression "]"` | `emits_append_and_checked_index` |
| `"append" "(" identifier "," expression ")"` | `emits_append_and_checked_index` |
| int32, string, and boolean literals | `covers_every_annex_a_production` |
| array declaration with capacity | `uses_static_storage_for_sequences_over_the_stack_limit` |
| preserved comments and escaped string literals | `preserves_comments_and_unescapes_strings` |
| comments attached to source constructs | `attaches_comments_to_source_constructs` |
| user-defined function parameters and returns | `emits_functions_with_user_defined_types`, `zlang.cli.compiles_user_types` |
| call arguments with nested field and index expressions | `passes_complex_function_arguments` |
| disabled comments | `zlang.cli.disable_comments.output` |
| `ZLANG1001` unexpected token | `reports_syntax_diagnostics_with_source_locations` |
| `ZLANG1002` unterminated string | `reports_syntax_diagnostics_with_source_locations` |
| `ZLANG1003` invalid character | `reports_syntax_diagnostics_with_source_locations` |
| `ZLANG1004` unsupported construct | `reports_syntax_diagnostics_with_source_locations` |
| `ZLANG2001` literal out of range | `reports_type_and_resource_diagnostics_with_source_locations` |
| `ZLANG2002` duplicate declaration | `reports_type_and_resource_diagnostics_with_source_locations` |
| `ZLANG2003` undefined identifier | `reports_type_and_resource_diagnostics_with_source_locations` |
| `ZLANG2004` type mismatch | `reports_type_and_resource_diagnostics_with_source_locations` |
| `ZLANG3001` unbounded array | `rejects_an_unbounded_sequence_at_the_declaration`, `rejects_an_empty_inferred_sequence` |
| `ZLANG2005` shadowed declaration | `rejects_a_shadowed_declaration` |
| `ZLANG2006` string capacity required | `rejects_variable_concatenation_without_capacity` |
| `ZLANG3002` unused capacity warning | `warns_about_unused_explicit_capacity` |
| `ZLANG3003` initializer too long | `rejects_an_initializer_longer_than_capacity` |
| `ZLANG3004` unnecessary reference | `warns_about_unnecessary_references`, `accepts_mutated_scalar_reference` |
| sequence initializer and inferred capacity | `emits_an_inferred_sequence_initializer`, `emits_a_partial_explicit_sequence_initializer` |
| checked index assignment | `emits_index_assignment_with_a_checked_store` |
| literal string concatenation | `folds_literal_string_concatenation` |
| bounded variable string concatenation | `emits_bounded_variable_string_concatenation` |
| direct function call | `emits_direct_function_calls` |
| ordering and inequality operators | `emits_ordering_and_inequality_operators` |
| `else ifStatement` | `emits_else_if_chains` |
| `!`, `&&`, `||` and grouped expressions | `emits_logical_operators` |
| boolean operand checking | `rejects_non_boolean_logical_operands` |
| `break` and `continue` | `emits_break_and_continue` |
| invalid loop jumps | `rejects_break_outside_loop`, `rejects_continue_outside_loop` |
| boolean and uint32 conditions | `rejects_non_boolean_if_condition`, `rejects_non_unsigned_repeat_count` |
| return validation | `validates_return_types`, `rejects_return_outside_function` |
| self concatenation | `accepts_self_concatenation_with_explicit_capacity` |
| nested statements emitted once | `does_not_duplicate_nested_statements` |
| generated C compiles | `zlang.cli.compiles_generated_c` |
| forward function calls through generated prototypes | `emits_function_prototypes`, `zlang.cli.compiles_forward_calls` |
| only referenced array helpers are emitted | `emits_only_used_array_helpers` |
| documented golden examples with committed generated C and expected stdout | `zlang.examples.*` |
| `-`, unary `-`, `*`, `/`, `%` | `emits_arithmetic_operators` |
| compound assignment | `emits_compound_assignments` |
| `length(sequence)` | `emits_length_builtin` |
| user module imports | `zlang.cli.modules` |
| canonical generated names | `zlang.cli.modules` |
| cyclic module rejection | `zlang.cli.modules_cycle` |
| module root escape rejection | `zlang.cli.modules_escape` |
| legacy `"random"` import rejection | `zlang.cli.legacy_std_import` |
| explicit `main` with host arguments | `zlang.cli.program_options` |
| standard command line options | `zlang.cli.program_options` |
| standard imports | `zlang.stdlib.random.generate_int32.basic` |
| transpile-time optimization | `zlang.cli.tto` |
| standard output | `emits_scalar_write` |
| bounded input | `emits_io_read_calls` |
| bounded input validation | `rejects_read_without_bounded_string` |
| range-proven index folding | `folds_proven_loop_indices` |
| unproven bounds preservation | `keeps_unproven_bounds_traps` |
| nested read materialization | `materializes_proven_nested_reads` |
| struct methods | `emits_struct_constructor_and_methods` |
| static struct methods | `emits_static_struct_method` |
| method/field collision | `rejects_method_field_name_conflict` |
| wide and floating primitives | `emits_wide_and_floating_primitives` |
| explicit numeric conversions | `emits_explicit_numeric_conversions` |
| implicit conversion rejection | `rejects_implicit_numeric_conversion` |
| floating modulo rejection | `rejects_float_modulo` |
| inferred shape index validation | `rejects_index_outside_inferred_shape` |
| removed `print` keyword | `rejects_print_keyword` |
| `@std/io.write` overloads | `emits_scalar_write` |
| mixed numeric arithmetic rejection | `rejects_mixed_numeric_addition` |
| mixed numeric comparison rejection | `rejects_mixed_numeric_comparison` |
| string content equality | `compares_strings_by_content` |
| string ordering rejection | `rejects_string_ordering` |
| checked unary negation | `emits_checked_unary_negation` |
| standard math overloads | `emits_math_overloads` |
| standard strings API | `emits_strings_api` |
| string array elements | `emits_string_sequence_operations` |
| legacy sequence syntax rejection | `rejects_legacy_sequence_syntax` |
| sibling scope identifier generation | `renames_sibling_scope_declarations` |
| native `for` loop | `emits_native_for_loops` |
| nested loops | `emits_nested_loops` |
| indexed `foreach` | `emits_indexed_foreach` |
| `const` declarations | `emits_const_declarations` |
| `const` assignment rejection | `rejects_assignment_to_const` |
| string indexing | `emits_string_index_read` |
| readable emitted C layout | `uses_readable_emitted_layout` |
| unique sequence types across sibling scopes | `emits_unique_sequence_types_for_sibling_scopes` |
| string literal deduplication | `deduplicates_string_literals` |
| bounded index getters | `checked_index_reads_return_values`, `uses_readable_emitted_layout` |
| named generated value constants | `uses_readable_emitted_layout` |
| structs and enums | `emits_structs_and_enums` |
| repeated index read reuse | `reuses_repeated_index_reads` |
| emitted C symbol naming | `emits_functions_with_user_defined_types`, `uses_readable_emitted_layout` |
| generated temporaries without synthetic blocks | `emits_temporaries_without_synthetic_blocks` |
| contextual field-pointer CSE | `reuses_field_pointers_contextually` |
| contextual index-value CSE across branches | `reuses_index_values_across_if_branches` |
| reserved generated namespace | `rejects_reserved_identifiers` |
| inferred local array shape | `infers_local_array_shape`, `rejects_unknown_array_extent` |
| inferred array function shape | `infers_array_function_shape` |
| source array helper rejection | `rejects_source_array_helpers` |
| nested array initializer | `emits_nested_array_initializer` |
| const array write rejection | `rejects_const_array_write` |
| whole-array copy | `emits_whole_array_copy` |
| multidimensional foreach | `emits_multidimensional_foreach` |
| array struct field access | `emits_array_record_field_access` |
| multiple array specializations | `emits_multiple_array_specializations` |
| short-circuit index reads stay inline | `keeps_short_circuit_index_reads_inline` |
| parameter qualifiers | `emits_parameter_qualifiers` |
| missing parameter qualifier rejection | `rejects_missing_parameter_qualifier` |
| const parameter mutation rejection | `rejects_mutation_of_const_parameter` |
| reference lvalue rejection | `rejects_non_lvalue_reference_argument` |
| diagnostic source locations | `reports_syntax_diagnostics_with_source_locations`, `reports_type_and_resource_diagnostics_with_source_locations`, `zlang.cli.diagnostic_format` |
| `struct Derived : Base` inheritance and vtable emission | `emits_struct_inheritance_and_vtable`, `emits_unbounded_polymorphic_metadata` |
| dynamic dispatch through a base receiver | `emits_dynamic_dispatch_call` |
| static traits via `uses` | `emits_trait_method_direct_call` |
| downcast rejection | `rejects_downcast` |
| override signature validation | `rejects_override_signature_mismatch` |
| trait collision rejection | `rejects_trait_override_of_inherited_method`, `rejects_conflicting_trait_methods` |


## 0.6 implementation status

Zlang 0.6 adds struct and enum declarations, user-defined declaration types, field reads and field
assignments, and arrays of structs. Struct fields are scalar or enum types. Enums emit named C
constants, and structs emit named C typedefs.


## 0.7 implementation status

Zlang 0.7 allows struct and enum names in function parameters and return values, allows struct
variables to be initialized from same-type values, and emits `(void)` for zero-argument functions.
Source comments attach to the construct they document in the generated C, with `--disable-comments`
available when the comments are not wanted.


## 0.8 implementation status

Zlang 0.8 separates emitted C names by role: local source symbols start with `_`, function
parameters keep their source names, struct members end with `_`, and user functions keep `_u_`.
Compiler temporaries, generated macros, generated types and runtime calls keep their established
prefixes.


## 0.9 implementation status

Zlang 0.9 reserves the `zlang`, `_zlang` and `ZLANG` identifier namespaces so generated names cannot
collide with source names. The emitter no longer introduces synthetic `{}` blocks for index and
field temporaries; those temporaries are declared directly in the enclosing C block. Only blocks
written in Zlang source produce C blocks.


## 0.10 implementation status

Zlang 0.10 resolves sequence field references once per context. The emitter materializes a checked
pointer at the first field access of an element and reuses it across statements until an index
dependency is assigned or the lexical scope ends. Repeated reads in condition chains keep their
existing local temporaries.


## 0.11 implementation status

Zlang 0.11 extends contextual CSE to index reads. A read repeated across an `if` chain and its branch
bodies is materialized once before the chain and reused inside the branches. Cached reads are
invalidated by writes and by scope exit. Reads that live only under short-circuit `&&` or `||` remain
inline so trap timing is unchanged.


## 0.12 implementation status

Zlang 0.12 adds mandatory parameter qualifiers. Every parameter uses `clone` or `reference`, with
optional `const` before the mode. `clone` is a local value copy; `reference` is a caller-owned alias
lowered to a C pointer that is never exposed in Zlang source. References cannot be stored or
returned, and reference arguments must be lvalues.


## 0.13 implementation status

Zlang 0.13 warns with `ZLANG3004` when a `reference` parameter of a small value type has no
mutational purpose. `const reference` to a scalar, string or enum is always warned about; a mutable
`reference` is warned about only when the function body never mutates it. Mutated reference
parameters and references to structs are accepted without this warning.


## 0.14 implementation status

Zlang 0.14 replaces `sequence` with fixed-shape `array`, adds multidimensional declarations and
indexing, emits extent macros, and lowers rank-2 and higher access through generated offset helpers
and typed get/set helpers. Arrays have no logical length and no append operation.


## 0.15 implementation status

Zlang 0.15 adds source-level `array_get(array, {i, ...})` and `array_set(array, {i, ...}, value)`.
The tuple rank must match the declared array rank, and both operations lower to the generated typed
helpers. In 0.17 those source helpers were removed again: only source indexing is available, and it
lowers to the generated typed helpers.


## 0.16 implementation status

Zlang 0.16 infers array rank and extents before semantic analysis. Declarations written as
`array T name;` use initializer count, constant indices and canonical loop induction ranges as
evidence. Function parameters and returns written as `array` infer one concrete shape from call
sites or return assignment. Canonical array typedefs keep locals, parameters and returns type
compatible in generated C. Multidimensional indexing now lowers directly to internal `_zlang_*`
helpers.


## 0.17 implementation status

Zlang 0.17 removes `array_get` and `array_set` from source. The generated C still uses internal
`_zlang_array_get_*` and `_zlang_array_set_*` helpers for bounds-checked multidimensional access,
but source code only uses `array[i]...`.


## 0.20 implementation status

Zlang 0.19 emits only the array helpers referenced by generated code and declares user functions
before their definitions, so functions may call functions defined later in the source.


## 0.29 implementation status

Zlang 0.29 supports single inheritance with `struct Derived : Base`. The base is the first C
subobject of the derived struct. A derived value can be used where a base value or `reference Base`
is expected; the reverse conversion is rejected. Arrays keep their exact element types and do not
participate in inheritance.

Instance methods are virtual by default. A polymorphic hierarchy emits one internal vtable type and
one vtable instance per concrete struct. The root subobject carries an internal type tag and vtable
pointer. Dynamic calls through a base receiver use the vtable slot for the method name; trait calls
are direct calls because trait methods are compiled into the using struct.

A trait declares required fields and methods with `trait Name requires ... { ... }`. A struct opts
in with `uses Name`. Trait methods become concrete methods of the using struct, cannot override
inherited methods, and introduce no runtime indirection. The cost of tags, vtable pointers and
indirect calls is accepted; generated programs still perform no dynamic allocation. The emitter
derives dispatch metadata from the AST, so it has no fixed vtable or type-tag capacity.

## 0.28 implementation status

Zlang 0.28 replaces `record` with `struct` and adds struct members. A struct can declare fields,
one constructor, instance methods and `static function` methods. Inside instance methods and
constructors, fields are referenced without an explicit receiver; the emitter supplies `self` in the
generated C. Instance calls use `value.method()` and static calls use `Struct.method()`. Method names
cannot collide with field names. Inheritance is not part of this release.

## 0.27 implementation status

With `--tto`, nested proven reads used by a single arithmetic assignment are emitted as sequential
typed temporaries before the final short assignment. This keeps generated C readable without
changing evaluation order or bounds semantics.

## 0.26 implementation status

With `--tto`, Zlang 0.26 proves ranges for canonical `for` loops over signed `int32` variables with
constant start, constant `<` or `<=` bound and `+= 1` update. One-dimensional array reads and writes
whose indices are the induction variable plus or minus a constant are emitted as direct `data_[]`
accesses when the whole proven range is inside the array extent. An index whose range cannot be
proved keeps its bounds check and `ZLANG_TRAP_BOUNDS`.

## 0.25 implementation status

Zlang 0.25 adds `read` and `read_line` to `@std/io`. Both take a mutable bounded string destination
and return `int32`: `-1` at EOF, otherwise the number of logical bytes read. Line endings are
normalized across platforms; CRLF is treated as one logical newline. Overflow traps with
`ZLANG_TRAP_CAPACITY`.

## 0.24 implementation status

Zlang 0.24 removes the built-in `print` construct. Output is performed by `write` imported from
`@std/io`, with overloads resolved from argument types. No compatibility spelling is provided.

## 0.23 implementation status

Zlang 0.23 enforces exact operand types in arithmetic and comparisons, compares strings by content,
rejects string ordering and checks signed unary negation. `@std/math` exposes `abs`, `clamp`, `min`,
`max` and `sqrt` with overload resolution by argument type. `@std/strings` exposes `compare`,
`contains` and `starts_with`.

## 0.22 implementation status

Zlang 0.22 adds `int64`, `uint64`, `float` and `double`, explicit `to_*` numeric conversions, IEEE-754
floating arithmetic and deterministic random generators for all six numeric types. Standard stateful
functions share one internal `zlang_runtime_context_t`; Zlang source never exposes it. Inferred array
functions emit one canonical C function per concrete shape.

## 0.20 implementation status

Zlang 0.20 supports standard library imports and the `random.generate_int32` function. Imports are
explicit, domain-based and activated in generated C by one macro per function. `--tto` enables a
fixed-point transpile-time optimizer for constant expressions and literal array access folding.

Generated C uses provenance prefixes: `app_*` for user-derived declarations, `zlang_gen_*` for
compiler support, `zlang_std_*` for standard functions, `zlang_*` for the runtime API and `ZLANG_*`
for language constants.

## 0.18 implementation status

Zlang 0.18 completes arrays. Initializers may be nested, `foreach` accepts one index name per axis,
structs may contain fixed array fields, `const array` declarations reject writes, whole-array copy
requires identical shape, and a generic inferred-array function is monomorphized once per called
shape.


## Emitted comments

Zlang comments are lexed but never become parser tokens. The emitter writes them back into the
generated C next to the source construct they document: a comment before a declaration is emitted
before that declaration, a comment inside a struct is emitted before the matching field, and a
comment inside an enum is emitted before the matching enumerator. Consecutive `//` lines stay
together as one block, and block comments keep their internal line breaks. Comments that do not
document a specific construct are emitted at the start of `main`. They are documentation only and
have no semantic effect. `--disable-comments` omits them from the generated C.

## 0.30 implementation status

Status: implemented.

Zlang 0.30 adds compile-time generics for structs and functions. A declaration may list up to four
type parameters, and an application may pass up to four concrete type arguments. Applications such
as `Pair<int32, boolean>` and `identity<int32>(1)` are monomorphized into concrete C99 types and
functions with substituted type names. There is no runtime generic representation, heap allocation
or reflection.

Generic structs cannot inherit, use traits or declare methods. Generic functions must be top-level
and have flat bodies. Nested type arguments are not part of the MVP. Invalid applications, duplicate
type parameters and capacity exhaustion produce diagnostics.

The canonical stdlib result shape is `Result<T, E>`: `T` is the value the function would ideally
return, `E` is the error code, and `what` carries a bounded message. A function with no payload uses
`Result<void, Status>`. Stdlib adoption is deferred to 0.31.

## 0.31 implementation status

Status: implemented.

Zlang 0.31 defines native generic facilities for the standard library. A closed registry of
`Result<T, Status>` applications maps canonical Zlang names to C layouts declared in
`<zlang/standard/result/result.h>`. Every result exposes `code`, `what` and, except for `void`,
`value`. Status constants are exposed as `STATUS_SUCCESS`, `STATUS_END_OF_INPUT`, `STATUS_IO_ERROR`,
`STATUS_INVALID_ARGUMENT` and `STATUS_CAPACITY`.

The stdlib registry records structured return metadata and parameter modes. Native function
development requires parity between the compiler registry, the public C header and the
documentation; a future binary verifier is planned for that check. Existing stdlib functions keep
their 0.30 signatures; no function is migrated in 0.31.

## 0.32 implementation status

Status: implemented.

Zlang 0.32 will redesign IO around streams, bounded caller-owned buffers and byte-transfer
results. Both `write` and `read` return `Result<int32, Status>`, where `value` is the number of
bytes actually transferred and `code` is `SUCCESS`, `END_OF_INPUT` or `IO_ERROR`. This follows the
`error_code, bytes_transferred` model used by libraries such as Boost.Asio, adapted to Zlang's
synchronous and allocation-free contract.

IO functions will accept an explicit stream handle. `stdin`, `stdout` and `stderr` are ordinary
stream values; the same API must be able to accept future stream facilities such as sockets
without a separate error protocol. Convenience overloads may keep implicit `stdout` and `stdin`,
but the explicit stream overload is canonical.

`read` will write into caller-owned storage and will take the destination view, an offset and a
maximum byte count. It returns the number of bytes read into that window. `write` will borrow a
bounded string view and return the number of bytes written. Neither operation copies buffer bytes;
string and stream parameters are views. The stdlib documentation must state that explicitly.

Programmer and resource errors such as null storage, zero capacity, invalid offsets and capacity
overflow remain traps. Recoverable stream conditions are reported through `Result<int32, Status>`.

The migration will update the stdlib registry, public headers, generated C fixtures, the golden
examples and the documentation. Native parity between registry entry, header declaration and docs
must be verified before each migrated function is considered complete.

## 0.35 implementation status

Status: implemented.

Zlang 0.35 defines a stable native stream contract. A stream is a borrowed, trivially copyable
handle with a constant operations table and caller-owned state. IO functions use explicit
`source` and `destination` roles: `read(source, destination, offset, count)` and
`write(destination, source, offset, count)`. Convenience overloads may keep implicit stdin or
stdout, but the explicit forms are canonical.

Stdio and caller-owned memory streams implement the contract. Future sockets, pipes, descriptors
and hardware backends must implement the same operations rather than introducing parallel IO
families. Status codes are shared through `<zlang/standard/status/status.h>`.

## 0.34 implementation status

Status: implemented.

Zlang 0.34 will rename the built-in scalar types `int32` and `uint32` to `int32` and `uint32`.
The explicit conversion calls follow the same rule: `to_int32` becomes `to_int32` and
`to_uint32` becomes `to_uint32`. The C representation is already `int32_t` and `uint32_t`; the
rename makes source names match the emitted representation and aligns them with `int64` and
`uint64`. This is a breaking change with no aliases or compatibility layer. Compiler rules, stdlib
signatures, docs, tests, goldens and site content must be updated together.

## 0.33 implementation status

Status: implemented.

Zlang 0.33 will add generic methods. The first target is `Result<T, E>` ergonomics: `is_ok()`,
`get()` and `message()` will be implemented as native helpers and exposed through method syntax.
After that, general methods on generic structs and functions may be introduced. Generic methods will
be monomorphized per specialization and will not add runtime dispatch. Generic traits and generic
inheritance remain outside scope.

## 0.30–0.31 conformance additions

| Requirement | Test |
| --- | --- |
| generic struct specialization | `emits_generic_struct_specialization`, `zlang.examples.features.generics` |
| generic function specialization | `emits_generic_function_specialization` |
| generic argument count validation | `rejects_generic_argument_count_mismatch` |
| duplicate generic parameter rejection | `rejects_duplicate_generic_parameter` |
| native `Result<T, Status>` mapping | `emits_native_result_type`, `assigns_native_result_fields` |
| IO byte-transfer results | `emits_io_read_calls`, `zlang.cli.io_read` |
| explicit stream handles | `emits_stream_write_result`, `emits_stream_read_result` |
| stable native stream contract | `streams_use_the_same_contract` |
| explicit write window | `emits_stream_write_result` |
| native result methods | `emits_native_result_methods` |
| generic struct methods | `emits_generic_struct_method` |
