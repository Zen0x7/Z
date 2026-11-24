# Zlang Language Specification

**Status:** Current

**Scope:** the core language implemented by `zlang-compile`.

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
| Integer literal | decimal digits, with optional `u` suffix for unsigned |
| String literal | `"..."` with escapes `\" \\ \n \t` |
| Comment | `//` to end of line, or `/* ... */` |
| Whitespace | space, tab, carriage return, line feed |

Keywords are reserved and cannot be used as identifiers:

```text
boolean break clone const continue else enum false for foreach function if integer
length print record reference repeat return sequence string true unsigned while
```

The generated-code namespace is also reserved. An identifier beginning with `zlang`, `_zlang` or
`ZLANG` is rejected with `ZLANG1004`.

### 6.3 Numeric literals

An integer literal without `u` has type `integer`. With `u`, it has type `unsigned`. A literal outside the range of its type is a type diagnostic.

---

## 7. Values and types

Zlang has four scalar value types, plus user-defined record and enum types.

### 7.1 `integer`

A signed 32-bit value. Emitted as `int32_t` from `<stdint.h>`.

### 7.2 `unsigned`

An unsigned 32-bit value. Emitted as `uint32_t` from `<stdint.h>`.

### 7.3 `boolean`

The values `true` and `false`. Emitted as `bool` from `<stdbool.h>`.

### 7.4 `string`

An immutable, non-owning view over UTF-8 bytes. It has two fields: a pointer to the first byte and a byte length. It never allocates and never owns memory.

A string literal is interned by the compiler and emitted in read-only storage. String concatenation is supported, but it never allocates. When every operand is a literal, the compiler folds the result into one emitted literal. When any operand is a variable, the declaration must use explicit capacity syntax, the compiler reserves a fixed backing buffer, and `zlang_string_concat` copies the views into it. Overflow traps instead of growing the buffer.

### 7.5 Sequence values

An array is a fixed-shape container written as `<type>[<capacity>]`. Capacity is part of the type, not a dynamic property.

```zlang
array[4] integer values;
array[8] string[32] names;
```

The capacity must be a positive integer literal. `array[] integer values;` is illegal and produces a resource diagnostic. A sequence may also be initialized with a brace list; the compiler infers the capacity from the number of elements. An explicit capacity may be combined with a shorter initializer, in which case capacity is unchanged and length is the number of initializer elements. `array[] integer values = {};` is rejected with `ZLANG3001`, and an initializer longer than the declared capacity is rejected with `ZLANG3003`.

An array has no logical length. Every extent is fixed at compile time and every index is bounds checked. Its storage is decided at compile time:

- total size at or below `ZLANG_STACK_LIMIT` (default `4096` bytes) is emitted as a local C aggregate;
- larger arrays are emitted as `static` storage with internal linkage;
- the compiler must document the threshold used when it differs from the default.

### 7.6 Type compatibility

There is no implicit conversion between `integer` and `unsigned`, or between numeric types and `boolean`. A string can only be used where a `string` is expected. Array types with different extents are different types. A record or enum type matches only the same user-defined name, including in function parameters, return values and variable initializers.

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
array[4] integer values;
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
| `5` | `ZLANG_TRAP_DIVISION_BY_ZERO` |

### 9.5 `EmitForEach(node)`

Validate that the iterable is a sequence and that the iteration variable has the array element type. Emit a native C `for` loop over indices from `0` to `length_ - 1`. The loop must not become a runtime callback.

---

## 10. Statements and control flow

### 10.1 Expression statements

An expression followed by `;`. Expression statements include assignment, indexed assignment,
`print` and calls to user-defined functions.

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

Equivalent to a counted loop from `0` to `count - 1`. The expression must be `unsigned`.

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
`<type>` is a scalar or a previously declared record or enum name. The return type uses the same set
of types, without a mode. Record parameters and return values lower to generated C typedefs.
Functions have no access to mutable globals, because Zlang has no global variables.

```zlang
function add(clone integer left, clone integer right): integer {
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
| `integer` | `int32_t` |
| `unsigned` | `uint32_t` |
| `boolean` | `bool` |
| `string` | `zlang_string_t` |
| `array[N] T name` | generated typedef `zlang_name_t` with a `ZLANG_NAME_CAPACITY` macro |

### 12.3 Construct mapping

| Zlang | Emitted C |
| --- | --- |
| `integer var = 10;` | `int32_t var = 10;` |
| `unsigned n = 2u;` | `uint32_t n = 2U;` |
| `boolean ready = true;` | `bool ready = true;` |
| `string s = "Hi";` | `zlang_string_t s = { "Hi", 2U };` |
| `a + b` | checked addition, then plain `a + b` |
| `-a`, `a - b`, `a * b`, `a / b`, `a % b` | checked arithmetic helpers |
| `a += b` and other compound assignments | checked operation followed by assignment |
| `length(values)` | native `values.length_` as `uint32_t` |
| `a == b` | `a == b` |
| `a < b`, `a > b`, `a <= b`, `a >= b`, `a != b` | native C comparison |
| `!a`, `a && b`, `a || b` | native C logical operators with short-circuit behavior |
| `f(a, b)` | direct C call to the emitted function |
| `"a" + "b"` | folded string literal |
| `prefix + suffix` with explicit capacity | `zlang_string_concat(storage, capacity, parts, count)` |
| `print(s)` | `zlang_print(s)` |
| `print(i)`, `print(u)`, `print(b)` | bounded scalar print helpers |
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
leading `_`, function parameters keep their source names, record members use a trailing `_`, and
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
bool zlang_print(zlang_string_t value);
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
bool zlang_print_i32(int32_t value);
bool zlang_print_u32(uint32_t value);
bool zlang_print_bool(bool value);
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
`ZLANG3001` unbounded array, `ZLANG3002` unused capacity, and
`ZLANG3003` initializer too long. `ZLANG3004` warns about an unnecessary reference parameter.
Legacy postfix array declarations use `ZLANG1004`.
`ZLANG3002` is a warning: it is printed with
the `warning[...]` prefix and does not stop compilation.

Resource errors point at the declaration that needs a limit, not at the later use.

```text
main.zlang:3:9: error[ZLANG3001]: unbounded array 'values' requires an explicit capacity.
```

The compiler may emit more than one diagnostic, but it must not generate C for a program that has any error.

---

## 14. Non-goals

The following are explicitly outside Zlang:

- runtime reflection, dynamic dispatch, or runtime names;
- classes, inheritance, and objects;
- heap-backed strings or vectors, or any operation that grows storage dynamically;
- garbage collection or reference counting;
- hard realtime timing guarantees beyond predictable memory behavior;
- module imports; each compilation unit is one source file.

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
builtinType    = "integer" | "unsigned" | "boolean" | "string" ;

recordDeclaration = "record" identifier "{" declaration* "}" ;

enumDeclaration = "enum" identifier "{" identifier ( "," identifier )* "}" ;

arrayModifier = "sequence" "[" integerLiteral? "]" ;
typeModifier     = "[" integerLiteral "]" ;

arrayInitializer = "{" ( expression ( "," expression )* )? "}" ;

type           = typeName ;

ifStatement    = "if" "(" expression ")" block
                 ( "else" ( block | ifStatement ) )? ;

whileStatement = "while" "(" expression ")" block ;

forStatement   = "for" "(" ( declaration | expression? ) ";"
                 expression ";" expression ")" block ;

foreachStatement = "foreach" "(" identifier ( "," identifier )? "in"
                   identifier ")" block ;

repeatStatement  = "repeat" "(" expression ")" block ;

functionDeclaration = "function" identifier "(" parameters ")"
                      ( ":" type )? block ;

parameterMode  = "clone" | "reference" ;
parameter      = "const"? parameterMode type identifier ;
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
               | "print" "(" expression ")"
               | "length" "(" identifier ")"
               | "array_get" "(" identifier "," indexTuple ")"
               | "array_set" "(" identifier "," indexTuple "," expression ")"
               | identifier "[" expression "]"
               | expression "." identifier ;

indexTuple     = "{" expression ( "," expression )* "}" ;

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
array[4] integer values;

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

## Implementation status

The language described here is implemented across the lexer, parser, semantic analyzer, memory
planner, C emitter, runtime and `zlang-compile` driver. Diagnostics use the exact format in
clause 13, and every diagnostic carries the line and column of the source construct that caused
it. Version-by-version history is kept in `ROADMAP.md`.

## Annex C - Testability contract

A conformance test may assert on emitted C text without running the program:

```text
parse("integer var = 10;").transpile().contains("int32_t var = 10;")
```

```text
parse("array[] integer values;").diagnostics().contains("ZLANG3001")
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
| boolean and unsigned conditions | `rejects_non_boolean_if_condition`, `rejects_non_unsigned_repeat_count` |
| return validation | `validates_return_types`, `rejects_return_outside_function` |
| self concatenation | `accepts_self_concatenation_with_explicit_capacity` |
| nested statements emitted once | `does_not_duplicate_nested_statements` |
| generated C compiles | `zlang.cli.compiles_generated_c` |
| `-`, unary `-`, `*`, `/`, `%` | `emits_arithmetic_operators` |
| compound assignment | `emits_compound_assignments` |
| `length(sequence)` | `emits_length_builtin` |
| scalar printing | `emits_scalar_print` |
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
| records and enums | `emits_records_and_enums` |
| repeated index read reuse | `reuses_repeated_index_reads` |
| emitted C symbol naming | `emits_functions_with_user_defined_types`, `uses_readable_emitted_layout` |
| generated temporaries without synthetic blocks | `emits_temporaries_without_synthetic_blocks` |
| contextual field-pointer CSE | `reuses_field_pointers_contextually` |
| contextual index-value CSE across branches | `reuses_index_values_across_if_branches` |
| reserved generated namespace | `rejects_reserved_identifiers` |
| tuple array access | `emits_array_get_and_set`, `rejects_array_tuple_rank_mismatch` |
| short-circuit index reads stay inline | `keeps_short_circuit_index_reads_inline` |
| parameter qualifiers | `emits_parameter_qualifiers` |
| missing parameter qualifier rejection | `rejects_missing_parameter_qualifier` |
| const parameter mutation rejection | `rejects_mutation_of_const_parameter` |
| reference lvalue rejection | `rejects_non_lvalue_reference_argument` |
| diagnostic source locations | `reports_syntax_diagnostics_with_source_locations`, `reports_type_and_resource_diagnostics_with_source_locations`, `zlang.cli.diagnostic_format` |


## 0.6 implementation status

Zlang 0.6 adds record and enum declarations, user-defined declaration types, field reads and field
assignments, and arrays of records. Record fields are scalar or enum types. Enums emit named C
constants, and records emit named C typedefs.


## 0.7 implementation status

Zlang 0.7 allows record and enum names in function parameters and return values, allows record
variables to be initialized from same-type values, and emits `(void)` for zero-argument functions.
Source comments attach to the construct they document in the generated C, with `--disable-comments`
available when the comments are not wanted.


## 0.8 implementation status

Zlang 0.8 separates emitted C names by role: local source symbols start with `_`, function
parameters keep their source names, record members end with `_`, and user functions keep `_u_`.
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
parameters and references to records are accepted without this warning.


## 0.14 implementation status

Zlang 0.14 replaces `sequence` with fixed-shape `array`, adds multidimensional declarations and
indexing, emits extent macros, and lowers rank-2 and higher access through generated offset helpers
and typed get/set helpers. Arrays have no logical length and no append operation.


## 0.15 implementation status

Zlang 0.15 adds source-level `array_get(array, {i, ...})` and `array_set(array, {i, ...}, value)`.
The tuple rank must match the declared array rank, and both operations lower to the generated typed
helpers. `array_set` participates in mutation tracking and CSE invalidation.


## Emitted comments

Zlang comments are lexed but never become parser tokens. The emitter writes them back into the
generated C next to the source construct they document: a comment before a declaration is emitted
before that declaration, a comment inside a record is emitted before the matching field, and a
comment inside an enum is emitted before the matching enumerator. Consecutive `//` lines stay
together as one block, and block comments keep their internal line breaks. Comments that do not
document a specific construct are emitted at the start of `main`. They are documentation only and
have no semantic effect. `--disable-comments` omits them from the generated C.
