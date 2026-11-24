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
- fixed-capacity sequences with compile-time memory planning;
- native C control flow, including `else if` chains;
- bounded string concatenation instead of heap-backed strings;
- direct C calls to user-defined functions;
- fixed numeric traps when a declared limit is violated.

The current language adds comments preserved in generated C, lexical scopes with unique generated
identifiers, sequence initializers, checked index assignment, bounded strings, records, enums, field
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
`const`, records, enums, all four scalar types, leading `sequence[N]` declarations, bounded string
elements, functions and `return`, user-defined function parameter and return types, scopes,
assignments, indexed access and assignment, field access and assignment, `append`, arithmetic,
compound assignment, comparisons, logical operators, `length`, string indexing, scalar and string
printing, `if`/`else if`/`else`, `while`, `repeat`, native `for`, indexed `foreach`, nested loops,
`break` and `continue`.

The emitted C is organized before `main`: generated value constants, user types, capacity macros,
sequence typedefs, named string literals and user functions. Source comments are preserved in the
generated C next to the construct they document, or omitted with `--disable-comments`.
Local source symbols use a leading `_`, function parameters are emitted exactly as written, record
members use a trailing `_`, user functions use `_u_`, compiler temporaries use `_zlang_`, generated
types use `zlang_` and runtime calls use `zlang_*`. The `zlang`, `_zlang` and `ZLANG` identifier
namespaces are reserved, so generated names cannot collide with source names. Only source blocks
emit `{}`; compiler temporaries are declared directly in the enclosing C block. Zero-argument
functions use `(void)` and unused parameters receive `(void)` guards. Index reads lower to bounded `zlang_*` getters. References to sequence elements are
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
sequence[4] integer values = {1, 2, 3};
append(values, 4);
sequence[5] integer fixed = {4, 5};
unsigned limit = 3U;
boolean ready = true;
string name = "z" + "lang";
string[32] label = name + " tour";
sequence[3] string[8] names = {"a", "b"};
sequence[2] Point points;

// Adds two integers.
function add(integer left, integer right): integer {
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
function paint(Point value, Color color): Point {
    value.x = 2;
    return value;
}

Point made = make();
Color chosen = GREEN;
Point painted = paint(made, chosen);
append(points, made);

fixed[0] = add(values[1], base);
append(fixed, 9);
append(names, name);
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
print(current);
```

`zlang-compile example.zlang --emit-c` produces C along these lines:

```c
#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

/* generated value constants */
#define ZLANG_ZERO 0U
#define ZLANG_ONE 1U
#define ZLANG_TWO 2U

/* generated user types */
// A fixed point type.
typedef struct zlang_point_t {
    // Horizontal coordinate.
    int32_t x_;
    /* Vertical coordinate
       in the same record. */
    int32_t y_;
} zlang_point_t;

// A named color set.
typedef enum zlang_color_t {
    // First color.
    ZLANG_COLOR_RED = 0U,
    // Primary colors
    // in order.
    ZLANG_COLOR_GREEN = 1U,
    ZLANG_COLOR_BLUE = 2U,
} zlang_color_t;

/* generated capacities */
#define ZLANG_VALUES_CAPACITY 4U
#define ZLANG_VALUES_LENGTH 3U
#define ZLANG_FIXED_CAPACITY 5U
#define ZLANG_FIXED_LENGTH 2U
#define ZLANG_NAMES_CAPACITY 3U
#define ZLANG_NAMES_LENGTH 2U
#define ZLANG_NAMES_ELEMENT_CAPACITY 8U
#define ZLANG_POINTS_CAPACITY 2U
#define ZLANG_POINTS_LENGTH 0U
#define ZLANG_LABEL_CAPACITY 32U

/* generated sequence types */
typedef struct zlang_values_t {
    int32_t data_[ZLANG_VALUES_CAPACITY];
    uint32_t length_;
} zlang_values_t;

typedef struct zlang_fixed_t {
    int32_t data_[ZLANG_FIXED_CAPACITY];
    uint32_t length_;
} zlang_fixed_t;

typedef struct zlang_names_t {
    char storage_[ZLANG_NAMES_CAPACITY][ZLANG_NAMES_ELEMENT_CAPACITY + ZLANG_ONE];
    zlang_string_t data_[ZLANG_NAMES_CAPACITY];
    uint32_t length_;
} zlang_names_t;

typedef struct zlang_points_t {
    zlang_point_t data_[ZLANG_POINTS_CAPACITY];
    uint32_t length_;
} zlang_points_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 5U
static const char _zlang_literal_0_data[] = "zlang";
static const zlang_string_t _zlang_literal_0 = { _zlang_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 5U
static const char _zlang_literal_1_data[] = " tour";
static const zlang_string_t _zlang_literal_1 = { _zlang_literal_1_data, ZLANG_LITERAL_1_LENGTH };
#define ZLANG_LITERAL_2_LENGTH 1U
static const char _zlang_literal_2_data[] = "a";
static const zlang_string_t _zlang_literal_2 = { _zlang_literal_2_data, ZLANG_LITERAL_2_LENGTH };
#define ZLANG_LITERAL_3_LENGTH 1U
static const char _zlang_literal_3_data[] = "b";
static const zlang_string_t _zlang_literal_3 = { _zlang_literal_3_data, ZLANG_LITERAL_3_LENGTH };
#define ZLANG_LITERAL_4_LENGTH 1U
static const char _zlang_literal_4_data[] = "z";
static const zlang_string_t _zlang_literal_4 = { _zlang_literal_4_data, ZLANG_LITERAL_4_LENGTH };
#define ZLANG_LITERAL_5_LENGTH 3U
static const char _zlang_literal_5_data[] = "big";
static const zlang_string_t _zlang_literal_5 = { _zlang_literal_5_data, ZLANG_LITERAL_5_LENGTH };
#define ZLANG_LITERAL_6_LENGTH 5U
static const char _zlang_literal_6_data[] = "small";
static const zlang_string_t _zlang_literal_6 = { _zlang_literal_6_data, ZLANG_LITERAL_6_LENGTH };

// Adds two integers.
int32_t _u_add(int32_t left, int32_t right) {
    // Checked addition lowers to a runtime call.
    return zlang_add_i32(left, right);
}

// Creates a point.
zlang_point_t _u_make(void) {
    zlang_point_t _result = { ZLANG_ZERO };
    _result.x_ = 1;
    _result.y_ = 2;

    return _result;
}

// Paints a copy; the color parameter is deliberately unused.
zlang_point_t _u_paint(zlang_point_t value, zlang_color_t color) {
    (void)color;
    value.x_ = 2;

    return value;
}

int main(void) {
    // A guided tour of the language.
    const int32_t _base = 2;
    zlang_values_t _values = { { 1, 2, 3, 0 }, ZLANG_VALUES_LENGTH };
    zlang_array_append(_values.data_, &_values.length_, ZLANG_VALUES_CAPACITY, sizeof(_values.data_[0]), (int32_t[]){ (4) });
    zlang_fixed_t _fixed = { { 4, 5, 0, 0, 0 }, ZLANG_FIXED_LENGTH };
    uint32_t _limit = 3U;
    bool _ready = true;
    zlang_string_t _name = _zlang_literal_0;
    char _zlang_label_storage_7[ZLANG_LABEL_CAPACITY + ZLANG_ONE] = { ZLANG_ZERO };
    zlang_string_t _label = zlang_string_concat(_zlang_label_storage_7, ZLANG_LABEL_CAPACITY, (zlang_string_t[]){ _name, _zlang_literal_1 }, ZLANG_TWO);
    zlang_names_t _names = { ZLANG_ZERO };
    _names.data_[0] = zlang_string_concat(_names.storage_[0], ZLANG_NAMES_ELEMENT_CAPACITY, (zlang_string_t[]){ _zlang_literal_2 }, ZLANG_ONE);
    _names.data_[1] = zlang_string_concat(_names.storage_[1], ZLANG_NAMES_ELEMENT_CAPACITY, (zlang_string_t[]){ _zlang_literal_3 }, ZLANG_ONE);
    _names.length_ = ZLANG_NAMES_LENGTH;
    zlang_points_t _points = { { { ZLANG_ZERO }, { ZLANG_ZERO } }, ZLANG_POINTS_LENGTH };
    zlang_point_t _made = _u_make();
    zlang_color_t _chosen = ZLANG_COLOR_GREEN;
    zlang_point_t _painted = _u_paint(_made, _chosen);
    zlang_array_append(_points.data_, &_points.length_, ZLANG_POINTS_CAPACITY, sizeof(_points.data_[0]), (zlang_point_t[]){ (_made) });

    uint32_t _zlang_index_9 = 0;
    if (_zlang_index_9 < _fixed.length_) {
        _fixed.data_[_zlang_index_9] = _u_add(zlang_array_get_i32(_values.data_, 1, _values.length_), _base);
    } else {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }

    zlang_array_append(_fixed.data_, &_fixed.length_, ZLANG_FIXED_CAPACITY, sizeof(_fixed.data_[0]), (int32_t[]){ (9) });
    zlang_string_array_append((char *)_names.storage_, _names.data_, &_names.length_, ZLANG_NAMES_CAPACITY, ZLANG_NAMES_ELEMENT_CAPACITY, (zlang_string_t[]){ _name }, ZLANG_ONE);

    uint32_t _zlang_index_10 = 0;
    if (_zlang_index_10 < _names.length_) {
        _names.data_[_zlang_index_10] = zlang_string_concat(_names.storage_[_zlang_index_10], ZLANG_NAMES_ELEMENT_CAPACITY, (zlang_string_t[]){ _zlang_literal_4 }, ZLANG_ONE);
    } else {
        zlang_trap(ZLANG_TRAP_BOUNDS);
    }

    zlang_point_t *_zlang_field_ptr_11 = (zlang_point_t *)zlang_array_get(_points.data_, 0, _points.length_, sizeof(_points.data_[0]));
    _zlang_field_ptr_11->x_ = 9;
    _zlang_field_ptr_11->y_ = _u_add(_zlang_field_ptr_11->x_, _base);
    int32_t _total = 1;
    _total = zlang_add_i32(_total, _base);

    {
        int32_t _scoped = zlang_mul_i32(_u_add(1, 2), 3);
        zlang_print_i32(_scoped);
    }


    int32_t _zlang_value_12 = zlang_array_get_i32(_fixed.data_, 0, _fixed.length_);
    if (_zlang_value_12 > 3) {
        zlang_print(_zlang_literal_5);
    } else if (_zlang_value_12 == 3) {
        zlang_print_i32(_zlang_value_12);
    } else {
        zlang_print(_zlang_literal_6);
    }

    while (_ready) {
        break;
    }

    for (uint32_t _zlang_repeat_13 = ZLANG_ZERO; _zlang_repeat_13 < _limit; ++_zlang_repeat_13) {
        continue;
    }

    for (uint32_t _outer = 0U; (_outer < 2U); _outer = zlang_add_u32(_outer, 1U)) {
        for (uint32_t _inner = 0U; (_inner < 2U); _inner = zlang_add_u32(_inner, 1U)) {
            zlang_print_u32(_outer);
            zlang_print_u32(_inner);
        }
    }

    for (uint32_t _zlang_foreach_14 = ZLANG_ZERO; _zlang_foreach_14 < _values.length_; ++_zlang_foreach_14) {
        uint32_t _row = _zlang_foreach_14;
        int32_t _item = _values.data_[_zlang_foreach_14];
        for (uint32_t _zlang_foreach_15 = ZLANG_ZERO; _zlang_foreach_15 < _values.length_; ++_zlang_foreach_15) {
            uint32_t _column = _zlang_foreach_15;
            int32_t _other = _values.data_[_zlang_foreach_15];
            zlang_print_u32(_row);
            zlang_print_u32(_column);
            zlang_print_i32(_item);
            zlang_print_i32(_other);
        }
    }

    uint32_t _count = _values.length_;
    uint32_t _first = zlang_string_byte(_label, 0);
    bool _ok = ((!(_ready && (!_ready))) || zlang_array_get_i32(_fixed.data_, 0, _fixed.length_) != 99);
    zlang_color_t _current = ZLANG_COLOR_GREEN;
    zlang_print_u32(_count);
    zlang_print_u32(_first);
    zlang_print_bool(_ok);
    zlang_print(_label);
    zlang_print(zlang_array_get_string(_names.data_, 0, _names.length_));
    zlang_print_i32(_zlang_field_ptr_11->x_);
    zlang_print_i32(_made.x_);
    zlang_print_i32(_painted.x_);
    zlang_print_u32(_current);

    return ZLANG_ZERO;
}
```

The exact whitespace and numbering of temporary names are not part of the contract; the C
constructs, name prefixes and `zlang_*` calls are.

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
