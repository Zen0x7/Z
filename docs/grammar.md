# Zlang grammar

## Scope

Z is a small source-to-source language for bounded, deterministic programs. A Zlang program is
translated to readable C99 that calls a small audited `zlang_*` runtime. There is no VM, no heap,
no garbage collector and no runtime name lookup. All types, capacities and storage decisions are
made by the compiler.

## Lexical rules

Identifiers start with a letter or underscore and continue with letters, digits or underscores.
Keywords are `boolean`, `break`, `clone`, `const`, `continue`, `else`, `enum`, `false`, `for`,
`double`, `foreach`, `from`, `function`, `if`, `import`, `in`, `int32`, `int64`, `length`,
`struct`, `reference`, `repeat`, `return`, `array`, `float`, `string`, `true`, `uint64`, `uint32`
and `while`.

Integer literals are decimal and may carry `u`, `i64` or `u64` suffixes. Binary arithmetic and
comparisons require matching operand types. Floating literals use a
decimal point or exponent and may carry `f` to select `float`; without `f` they are `double`. String literals are double-quoted and
support escaped quotes, backslashes, newlines and tabs. Line comments `// ...` and block comments
`/* ... */` do not affect parsing, but the emitter preserves them in the generated C next to the construct they document. Use `--disable-comments` to omit them.

Identifiers beginning with `zlang`, `_zlang` or `ZLANG` are reserved for generated C and are rejected
with `ZLANG1004`.

## Imports

```zlang
import { generate_int32, generate_bytes } from "@std/random";
import { write } from "@std/io";
import { tcp_listener } from "networking";
```

Imports appear at file scope and introduce standard library function names. They do not import user
modules. Generated C activates only the imported runtime headers.

## Declarations

```zlang
const int32 base = 10;
int32 total = 1;
uint32 limit = 3U;
boolean ready = true;
string label = "z" + "lang";
string[32] bounded = label + " value";

array[4] int32 values;
array[] int32 inferred = {1, 2, 3};
array[5] int32 partial = {1, 2, 3};
array[2] string[8] names = {"a", "b"};
```

`array[N]` declares a fixed number of elements. An empty capacity infers N from a brace
initializer. `string[M]` declares a bounded string with M content bytes. Applied inside a sequence
declaration, it sets the capacity of each string element.

A `const` declaration requires an initializer and cannot be assigned to.

A `struct` declares a fixed aggregate with named fields. An `enum` declares named integer values
and a user type. Both can be used as declaration types, array element types, function parameter
types and function return types:

```zlang
struct Point {
    int32 x;
    int32 y;
}

enum Color {
    RED,
    GREEN,
    BLUE
}

array[4] Point points;
Color current = GREEN;
```

A struct can inherit from one previously declared struct:

```zlang
struct Shape {
    int32 value;
    function area(): int32 { return value; }
}

struct Square : Shape {
    int32 side;
    function area(): int32 { return side * side; }
}
```

The base is the first subobject of the derived value. `Square` can be passed where `Shape` or
`reference Shape` is expected, but a `Shape` cannot be assigned to `Square`. Instance methods are
virtual by default, so a call through a base receiver dispatches to the effective override. An
override must keep the same return type, parameter types and static/instance kind.

Traits provide static behavior reuse:

```zlang
trait Named requires int32 value {
    function label(): int32 { return value; }
}

struct Counter uses Named {
    int32 value;
}
```

A trait requirement names a field that the using struct must provide with the same type. Trait
methods are compiled into the using struct, are called directly and never use the vtable. They
cannot have the same name as an inherited method or as a method from another trait used by the
same struct. Multiple inheritance and downcasting are not supported.

Every parameter needs `clone` or `reference`, with optional `const` before the mode. `clone` is a
local value copy; `reference` aliases caller storage and cannot be stored or returned. Shadowing a
visible name is rejected;
sibling blocks may reuse a source name because the emitter generates unique C identifiers when
needed.

## Statements

| Syntax | Meaning |
| --- | --- |
| `type name = expression;` | Scalar declaration |
| `array[N] type name;` | Fixed-shape array with explicit extents |
| `array type name = {{...}, {...}};` | Nested initializer with inferred shape |
| `array type name;` | Array with inferred rank and extents |
| `array[] type name = {...};` | Array with inferred extent |
| `if (expression) {...} else if (expression) {...} else {...}` | Conditional chain |
| `while (expression) {...}` | Pre-tested loop |
| `for (init; condition; update) {...}` | Native C-style loop |
| `foreach (item in sequence) {...}` | Native indexed loop |
| `foreach (index, item in sequence) {...}` | Indexed loop exposing the index as `uint32` |
| `foreach (row, col in grid) {...}` | Multidimensional index loop in row-major order |
| `repeat (count) {...}` | Native counted loop |
| `break;` / `continue;` | Loop control, valid only inside a loop |
| `function name(const? clone array name, ...): array {...}` | Function with inferred array parameters and return |
| `function name(const? clone type name, ...): type {...}` | Function with value parameters |
| `function name(const? reference type name, ...): type {...}` | Function with caller-owned references |
| `return expression;` / `return;` | Return from a function |
| `write(expression);` | Write a string or scalar |
| `name = expression;` | Assignment |
| `struct Name { field* }` | Fixed aggregate type |
| `enum Name { VALUE, ... }` | Named integer values and their type |
| `value.field` | Field read |
| `value.field = expression;` | Field assignment; checked for array elements |

## Expressions

Supported forms are literals, identifiers, grouping, indexing, field access and assignment, calls
to user functions, `write` from `@std/io`, `length`, unary `!` and `-`, and binary `+`, `-`, `*`, `/`, `%`,
`==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`. Compound assignment is available for arithmetic
operators.

`if`, `while` and `for` conditions must be `boolean`. `repeat` counts must be `uint32`. A
non-void function must return a value of its declared type; a void function must return without a
value. Loops may nest freely, including `for` inside `for` and `foreach` inside `foreach`.

String concatenation folds literals at compile time. Concatenation involving a variable requires
an explicit string capacity and copies into caller-owned storage. Indexing a string reads one byte
as `uint32` with a bounds trap; string elements are read-only.

## Builtins

`length(sequence)` emits the native first extent macro as an uint32 value. `write`
accepts strings, integers, uint32 integers and booleans, using fixed-stack runtime helpers.

## Diagnostics

| Code | Meaning |
| --- | --- |
| `ZLANG1001` | Unexpected token |
| `ZLANG1002` | Unterminated string |
| `ZLANG1003` | Invalid character |
| `ZLANG1004` | Unsupported construct or invalid placement |
| `ZLANG2001` | Literal out of range |
| `ZLANG2002` | Duplicate declaration |
| `ZLANG2003` | Undefined identifier |
| `ZLANG2004` | Type mismatch |
| `ZLANG2005` | Shadowed declaration |
| `ZLANG2006` | Explicit string capacity required |
| `ZLANG3001` | Unbounded array |
| `ZLANG3002` | Unused capacity warning |
| `ZLANG3003` | Initializer too long |
| `ZLANG3004` | Unnecessary reference parameter warning |

Runtime traps use fixed codes: `1` overflow, `2` bounds, `3` capacity, `4` invalid argument and
`5` division by zero.

## Generics (0.30)

Generic declarations list type parameters after the name:

```zlang
struct Pair<T, U> {
    T first;
    U second;
}

function identity<T>(clone T value): T {
    return value;
}
```

Use `Pair<int32, boolean>` and `identity<int32>(1)` to instantiate them. The compiler
monomorphizes each application into a concrete C99 type or function. Generic structs cannot inherit,
use traits or declare methods. Generic functions must be top-level and have flat bodies. Up to four
type parameters and four arguments per application are supported.

## Native results (0.31–0.37)

`result_type<T, status_code>` is a closed native generic family. Supported applications such as
`result_type<void, status_code>` and `result_type<int32, status_code>` map to canonical C layouts in
`<zlang/standard/result/result.h>`. Results expose `code`, `what` and, for non-void payloads,
`value`. Status constants are `STATUS_SUCCESS`, `STATUS_END_OF_INPUT`, `STATUS_IO_ERROR`,
`STATUS_INVALID_ARGUMENT` and `STATUS_CAPACITY`.

0.32 IO operations return `result_type<int32, status_code>` with the transferred byte count in `value`.
Convenience overloads use implicit stdout or stdin. Explicit overloads accept `STDIN`, `STDOUT` and
`STDERR` as borrowed stream handles, so future facilities such as sockets can use the same API.
`read` takes a bounded destination view, an offset and a maximum byte count. String and stream
parameters are borrowed views; no buffer bytes are copied. Passing a `string` by `clone` copies only
the descriptor.

Zlang 0.37 extends this to every registered standard function: all functions in `io`, `math`,
`program_options`, `random` and `strings` return `result_type<T, status_code>`. `T` is the ideal
value of the operation, `code` is a `status_code`, and `what` is a bounded read-only message view.
Pure and total functions report `STATUS_SUCCESS` for valid input; programmer and resource errors
remain traps and do not return.

`read` and `read_line` each have two overloads. The convenience overload reads from implicit `STDIN`
at offset `0` into a declared-capacity destination; the explicit overload takes a borrowed stream
and a bounded offset/count window. Both return `result_type<int32, status_code>` with the transferred
byte count in `value`. `read` copies raw bytes and keeps a newline when it fits the window;
`read_line` stops at a newline and drops the delimiter. `clone` copies scalars or trivial
descriptors: a cloned `string` copies its descriptor, not its bytes, and a cloned `stream` copies a
borrowed handle. `reference` is a mutable caller-owned destination; `const reference` is a borrowed
view.


## Planned 0.39 grammar

`label:`, `jump label;` and `switch` are accepted. The remaining productions are planned:

```text
labelStatement    = identifier ":" statement ;
jumpStatement     = "jump" identifier ";" ;
switchStatement   = "switch" "(" expression ")" "{" caseClause* defaultClause? "}" ;
caseClause        = "case" literal ":" statement* ;
defaultClause     = "default" ":" statement* ;
matchExpression   = "match" "(" expression ")" "{" matchArm ( "," matchArm )* "}" ;
matchArm          = ( literal | "_" ) "=>" expression ;
ternaryExpression = expression "?" expression ":" expression ;
doWhileStatement  = "do" block "while" "(" expression ")" ";" ;
rangeExpression   = expression ".." expression ;
interfaceDeclaration = "interface" identifier "{" interfaceMember* "}" ;
interfaceMember   = declaration | functionSignature ;
traitDeclaration  = "trait" identifier "requires" identifier ( "," identifier )*
                    "{" functionDeclaration* "}" ;
structDeclaration = "struct" identifier typeParameters? inheritance?
                    implements? "{" structMember* "}" ;
implements        = "implements" identifier ( "," identifier )* ;
useStatement      = "use" identifier ( "," identifier )* ";" ;
```

Named interfaces replace field-only trait requirements. A struct satisfies an interface when its
fields and methods exist directly or through a used trait.
