<p align="center"><a href="https://github.com/Zen0x7/Z" target="_blank"><img src="docs/logo.svg" width="400" alt="Zlang"></a></p>

<p align="center">
<a href="https://codecov.io/gh/Zen0x7/Z"><img src="https://codecov.io/gh/Zen0x7/Z/branch/master/graph/badge.svg?token=8TUYTBYEBJ" alt="Coverage"></a>
<a href="https://github.com/Zen0x7/Z/actions/workflows/ci.yml"><img src="https://github.com/Zen0x7/Z/actions/workflows/ci.yml/badge.svg?branch=master" alt="CI"></a>
<a href="https://github.com/Zen0x7/Z/actions/workflows/codeql.yml"><img src="https://github.com/Zen0x7/Z/actions/workflows/codeql.yml/badge.svg?branch=master" alt="CodeQL"></a>
<a href="https://github.com/Zen0x7/Z/actions/workflows/dependabot/dependabot-updates"><img src="https://github.com/Zen0x7/Z/actions/workflows/dependabot/dependabot-updates/badge.svg?branch=master" alt="Dependabot Updates"></a>
</p>

## About Z

> **Note:** This repository contains the Z compiler, its header-only C runtime and the test
> suite. For contributor rules and repository conventions, see [AGENTS.md](AGENTS.md).

Z is a small source-to-source language for bounded, deterministic programs. You write `.zlang`
source; `zlang-compile` emits readable C99; then your normal C toolchain produces the executable.

There is no VM, no bytecode, no garbage collector and no hidden runtime allocator. Types,
capacities and storage decisions are resolved by the compiler. Generated C calls a small audited
`zlang_*` API for checked arithmetic, bounds, capacity and string operations.

## What it is for

Z aims to make the "compile-time decisions, ordinary C output" idea pleasant to use:

- explicit value types: `integer`, `unsigned`, `boolean`, `string`;
- fixed-capacity arrays with compile-time memory planning;
- native C control flow, including `else if` chains;
- bounded string concatenation instead of heap-backed strings;
- direct C calls to user-defined functions;
- fixed numeric traps when a declared limit is violated.

The current language adds comments preserved in generated C, lexical scopes with unique generated
identifiers, array initializers, checked index assignment, bounded strings, records, enums, field
access, user-defined function parameter and return types, logical operators, expression grouping,
loop control and checked arithmetic.

## Pipeline

```text
example.zlang
    |
    |  zlang-compile
    v
example.c  ->  C compiler  ->  executable
```

```sh
./build/tools/zlang-compile/zlang-compile example.zlang --emit-c
./build/tools/zlang-compile/zlang-compile example.zlang
./build/tools/zlang-compile/zlang-compile example.zlang --emit-c --disable-comments
```

The first command only writes `example.c`. The second also invokes `$ZLANG_CC`, or `cc` by
default, to build the native binary. `--disable-comments` omits source comments from the generated C
when traceability is not needed.

## Language tour

This program exercises the language end to end: comments attached to their source constructs,
`const`, records, enums, all four scalar types, explicit and inferred array declarations, nested
array initializers, `const array`, whole-array copy, bounded string
elements, functions with `clone` and `reference` parameters, inferred array parameters and returns,
`return`, user-defined function parameter and return types, scopes,
assignments, indexed access and assignment, field access and assignment, arithmetic,
compound assignment, comparisons, logical operators, `length`, string indexing, scalar and string
printing, `if`/`else if`/`else`, `while`, `repeat`, native `for`, indexed and multidimensional `foreach`, nested loops,
`break` and `continue`.

Every parameter states its qualifier: `clone` is a mutable local copy, `const clone` is a read-only
copy, `reference` is a mutable alias to caller-owned storage and `const reference` is a read-only
alias. There are no `*` or `&` operators in Zlang; the emitter uses them internally when lowering a
reference. References are parameter-only: they cannot be stored or returned, and a reference
argument must be an lvalue. Multidimensional indexing lowers to generated typed helpers with names
such as `_zlang_array_get_2_i32`; those helpers are implementation details and are not available in
Zlang source. The compiler warns with `ZLANG3004` when a reference to a small value type has no
mutational purpose.

Records may embed fixed array fields, and inferred array functions generate one C specialization per
called shape. The emitted C is organized before `main`: generated value constants, user types, capacity macros,
array typedefs, named string literals and user functions. Source comments are preserved in the
generated C next to the construct they document, or omitted with `--disable-comments`.
Local source symbols use a leading `_`, function parameters are emitted exactly as written, record
members use a trailing `_`, user functions use `_u_`, compiler temporaries use `_zlang_`, generated
types use `zlang_` and runtime calls use `zlang_*`. The `zlang`, `_zlang` and `ZLANG` identifier
namespaces are reserved, so generated names cannot collide with source names. Only source blocks
emit `{}`; compiler temporaries are declared directly in the enclosing C block. Zero-argument
functions use `(void)` and unused parameters receive `(void)` guards. Index reads lower to bounded `zlang_*` getters. References to array elements are
resolved once per context: a checked pointer temporary is materialized at the first field access and
reused by later accesses to the same element until an index dependency changes or the scope ends.
Repeated index reads also share one local temporary across a condition chain and its branch bodies,
and `(void)` guards appear only for variables that are genuinely unused.

```zlang
// A guided tour of the language.
const integer base = 2;

// A fixed point type.
record Point {
    // Horizontal coordinate.
    integer x;
    /* Vertical coordinate
       in the same record. */
    integer y;
}

// A named color set.
enum Color {
    // First color.
    RED,
    // Primary colors
    // in order.
    GREEN,
    BLUE
}
array[4] integer values = {1, 2, 3};
array[5] integer fixed = {4, 5};
unsigned limit = 3U;
boolean ready = true;
string name = "z" + "lang";
string[32] label = name + " tour";
array[3] string[8] names = {"a", "b"};
array[2] Point points;

// Adds two integers.
function add(clone integer left, clone integer right): integer {
    // Checked addition lowers to a runtime call.
    return left + right;
}

// Creates a point.
function make(): Point {
    Point result;
    result.x = 1;
    result.y = 2;
    return result;
}

// Paints a copy; the color parameter is deliberately unused.
function paint(clone Point value, clone Color color): Point {
    value.x = 2;
    return value;
}

// Mutates the caller's record through a reference parameter.
function shift(reference Point point, clone integer amount) {
    point.x = point.x + amount;
}

// Copies an array through an inferred array parameter and return type.
function stamp(clone array source): array {
    array result;
    result[0][1] = source[0][1] + 1;
    return result;
}

Point made = make();
Color chosen = GREEN;
Point painted = paint(made, chosen);
shift(made, 3);
array[2][3] integer grid;
grid[0][1] = 5;
array[2][3] integer stamped = stamp(grid);

fixed[0] = add(values[1], base);
names[0] = "z";
points[0].x = 9;
points[0].y = add(points[0].x, base);

integer total = 1;
total += base;

{
    integer scoped = add(1, 2) * 3;
    print(scoped);
}

if (fixed[0] > 3) {
    print("big");
} else if (fixed[0] == 3) {
    print(fixed[0]);
} else {
    print("small");
}

while (ready) {
    break;
}

repeat (limit) {
    continue;
}

for (unsigned outer = 0U; outer < 2U; outer += 1U) {
    for (unsigned inner = 0U; inner < 2U; inner += 1U) {
        print(outer);
        print(inner);
    }
}

foreach (row, item in values) {
    foreach (column, other in values) {
        print(row);
        print(column);
        print(item);
        print(other);
    }
}

unsigned count = length(values);
unsigned first = label[0];
boolean ok = !(ready && !ready) || fixed[0] != 99;
Color current = GREEN;
print(count);
print(first);
print(ok);
print(label);
print(names[0]);
print(points[0].x);
print(made.x);
print(painted.x);
integer cell = grid[0][1];
integer stamped_cell = stamped[0][1];
print(cell);
print(stamped_cell);
print(current);
```

`zlang-compile example.zlang --emit-c` writes the generated C next to the source; without
`--emit-c`, the driver also invokes the C compiler and produces a native executable.

The language tour above is intentionally a compact feature map. Executable examples live under
[`examples/`](examples/), split into `algorithms/` and `features/`. Each example keeps three files:
the `.zlang` source, the committed generated `.c` golden and the expected stdout in
`.expected.txt`. CTest compiles every example, compares generated C byte-for-byte against the
golden, runs the native binary and compares stdout.

| Category | Examples |
| --- | --- |
| `algorithms` | `bubble_sort`, `collatz`, `factorial`, `fibonacci`, `fizzbuzz`, `gcd`, `matrix_multiply`, `pi`, `sieve`, `sum_of_squares` |
| `features` | `arithmetic`, `array_fields`, `arrays_1d`, `arrays_const_copy`, `arrays_inferred`, `arrays_nd`, `comments`, `conditions`, `const_qualifiers`, `functions`, `functions_array_specializations`, `functions_arrays`, `functions_user_types`, `hello`, `loops`, `records_enums`, `scopes`, `strings` |

Run the whole set with:

```sh
ctest --test-dir build --output-on-failure -L module:examples
```

## Quick start

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang
cmake --build build -j
ctest --test-dir build --output-on-failure -j
```

## Documentation

- [Live API reference](https://z.iantorres.cl)
- [Language specification](SPEC.md)
- [Roadmap and locked decisions](ROADMAP.md)
- [API reference source](docs/index.md)
- [Language grammar](docs/grammar.md)

The generated Doxygen site is written to `build/docs/html/index.html`.
