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

The current language adds comments, lexical scopes with unique generated identifiers, sequence
initializers, checked index assignment, bounded strings, logical operators, expression grouping,
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
```

The first command only writes `example.c`. The second also invokes `$ZLANG_CC`, or `cc` by
default, to build the native binary.

## Language tour

This program uses every construct in the current grammar: comments, `const`, records, enums, all
four scalar types, leading `sequence[N]` declarations, bounded string elements, functions and
`return`, scopes, assignments, indexed access and assignment, field access and assignment,
`append`, arithmetic, compound assignment, comparisons, logical operators, `length`, string
indexing, scalar and string printing, `if`/`else if`/`else`, `while`, `repeat`, native `for`,
indexed `foreach`, nested loops, `break` and `continue`.

The emitted C is organized before `main`: generated value constants, user types, capacity macros,
sequence typedefs, named string literals and user functions. Source symbols use `_u_`, compiler
temporaries use `_zlang_`, generated types use `zlang_` and runtime calls use `zlang_*`. Index reads
lower to bounded `zlang_*` getters, field assignment to a sequence element uses a checked pointer
temporary that is reused by the expression, repeated reads in condition chains become local
temporaries, and `(void)` guards appear only for variables that are genuinely unused.

```zlang
// A guided tour of the language.
const integer base = 2;
record Point {
    integer x;
    integer y;
}
enum Color {
    RED,
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

function add(integer left, integer right): integer {
    return left + right;
}

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
typedef struct zlang_point_t {
    int32_t x;
    int32_t y;
} zlang_point_t;

typedef enum zlang_color_t {
    ZLANG_COLOR_RED = 0U,
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

int32_t _u_add(int32_t _u_left, int32_t _u_right) {
    return zlang_add_i32(_u_left, _u_right);
}

int main(void) {
    const int32_t _u_base = 2;
    zlang_values_t _u_values = { { 1, 2, 3, 0 }, ZLANG_VALUES_LENGTH };
    zlang_array_append(_u_values.data_, &_u_values.length_, ZLANG_VALUES_CAPACITY, sizeof(_u_values.data_[0]), (int32_t[]){ (4) });
    zlang_fixed_t _u_fixed = { { 4, 5, 0, 0, 0 }, ZLANG_FIXED_LENGTH };
    uint32_t _u_limit = 3U;
    bool _u_ready = true;
    zlang_string_t _u_name = _zlang_literal_0;
    char _zlang_label_storage_7[ZLANG_LABEL_CAPACITY + ZLANG_ONE] = { ZLANG_ZERO };
    zlang_string_t _u_label = zlang_string_concat(_zlang_label_storage_7, ZLANG_LABEL_CAPACITY, (zlang_string_t[]){ _u_name, _zlang_literal_1 }, ZLANG_TWO);
    zlang_names_t _u_names = { ZLANG_ZERO };
    _u_names.data_[0] = zlang_string_concat(_u_names.storage_[0], ZLANG_NAMES_ELEMENT_CAPACITY, (zlang_string_t[]){ _zlang_literal_2 }, ZLANG_ONE);
    _u_names.data_[1] = zlang_string_concat(_u_names.storage_[1], ZLANG_NAMES_ELEMENT_CAPACITY, (zlang_string_t[]){ _zlang_literal_3 }, ZLANG_ONE);
    _u_names.length_ = ZLANG_NAMES_LENGTH;
    zlang_points_t _u_points = { { { ZLANG_ZERO }, { ZLANG_ZERO } }, ZLANG_POINTS_LENGTH };
    (void)_u_points;

    {
        uint32_t _zlang_index_9 = 0;
        if (_zlang_index_9 < _u_fixed.length_) {
            _u_fixed.data_[_zlang_index_9] = _u_add(zlang_array_get_i32(_u_values.data_, 1, _u_values.length_), 1);
        } else {
            zlang_trap(ZLANG_TRAP_BOUNDS);
        }
    }

    zlang_array_append(_u_fixed.data_, &_u_fixed.length_, ZLANG_FIXED_CAPACITY, sizeof(_u_fixed.data_[0]), (int32_t[]){ (9) });
    zlang_string_array_append((char *)_u_names.storage_, _u_names.data_, &_u_names.length_, ZLANG_NAMES_CAPACITY, ZLANG_NAMES_ELEMENT_CAPACITY, (zlang_string_t[]){ _u_name }, ZLANG_ONE);

    {
        uint32_t _zlang_index_10 = 0;
        if (_zlang_index_10 < _u_names.length_) {
            _u_names.data_[_zlang_index_10] = zlang_string_concat(_u_names.storage_[_zlang_index_10], ZLANG_NAMES_ELEMENT_CAPACITY, (zlang_string_t[]){ _zlang_literal_4 }, ZLANG_ONE);
        } else {
            zlang_trap(ZLANG_TRAP_BOUNDS);
        }
    }

    {
        zlang_point_t *_zlang_field_ptr_11 = (zlang_point_t *)zlang_array_get(_u_points.data_, 0, _u_points.length_, sizeof(_u_points.data_[0]));
        _zlang_field_ptr_11->x = 9;
    }
    {
        zlang_point_t *_zlang_field_ptr_12 = (zlang_point_t *)zlang_array_get(_u_points.data_, 0, _u_points.length_, sizeof(_u_points.data_[0]));
        _zlang_field_ptr_12->y = _u_add(_zlang_field_ptr_12->x, 0);
    }
    int32_t _u_total = 1;
    _u_total = zlang_add_i32(_u_total, _u_base);

    {
        int32_t _u_scoped = zlang_mul_i32(_u_add(1, 2), 3);
        zlang_print_i32(_u_scoped);
    }


    int32_t _zlang_value_13 = zlang_array_get_i32(_u_fixed.data_, 0, _u_fixed.length_);
    if (_zlang_value_13 > 3) {
        zlang_print(_zlang_literal_5);
    } else if (_zlang_value_13 == 3) {
        zlang_print_i32(zlang_array_get_i32(_u_fixed.data_, 0, _u_fixed.length_));
    } else {
        zlang_print(_zlang_literal_6);
    }

    while (_u_ready) {
        break;
    }

    for (uint32_t _zlang_repeat_14 = ZLANG_ZERO; _zlang_repeat_14 < _u_limit; ++_zlang_repeat_14) {
        continue;
    }

    for (uint32_t _u_outer = 0U; (_u_outer < 2U); _u_outer = zlang_add_u32(_u_outer, 1U)) {
        for (uint32_t _u_inner = 0U; (_u_inner < 2U); _u_inner = zlang_add_u32(_u_inner, 1U)) {
            zlang_print_u32(_u_outer);
            zlang_print_u32(_u_inner);
        }
    }

    for (uint32_t _zlang_foreach_15 = ZLANG_ZERO; _zlang_foreach_15 < _u_values.length_; ++_zlang_foreach_15) {
        uint32_t _u_row = _zlang_foreach_15;
        int32_t _u_item = _u_values.data_[_zlang_foreach_15];
        for (uint32_t _zlang_foreach_16 = ZLANG_ZERO; _zlang_foreach_16 < _u_values.length_; ++_zlang_foreach_16) {
            uint32_t _u_column = _zlang_foreach_16;
            int32_t _u_other = _u_values.data_[_zlang_foreach_16];
            zlang_print_u32(_u_row);
            zlang_print_u32(_u_column);
            zlang_print_i32(_u_item);
            zlang_print_i32(_u_other);
        }
    }

    uint32_t _u_count = _u_values.length_;
    uint32_t _u_first = zlang_string_byte(_u_label, 0);
    bool _u_ok = ((!(_u_ready && (!_u_ready))) || zlang_array_get_i32(_u_fixed.data_, 0, _u_fixed.length_) != 99);
    zlang_color_t _u_current = ZLANG_COLOR_GREEN;
    zlang_print_u32(_u_count);
    zlang_print_u32(_u_first);
    zlang_print_bool(_u_ok);
    zlang_print(_u_label);
    zlang_print(zlang_array_get_string(_u_names.data_, 0, _u_names.length_));
    zlang_print_i32((*((zlang_point_t *)zlang_array_get(_u_points.data_, 0, _u_points.length_, sizeof(_u_points.data_[0])))).x);
    zlang_print_u32(_u_current);

    return ZLANG_ZERO;
}
```

The exact whitespace and temporary names are not part of the contract; the C constructs and
`zlang_*` calls are.

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
