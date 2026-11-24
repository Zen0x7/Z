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

Version 0.2 adds comments, lexical scopes with unique generated identifiers, sequence
initializers, checked index assignment, bounded strings, logical operators and expression
grouping.

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

This single program uses every construct in the current grammar: comments, all four value types,
inferred and explicit sequences, functions and `return`, scopes, assignments, indexed access and
assignment, `append`, arithmetic, comparisons, logical operators, `if`/`else if`/`else`, `while`,
`repeat`, `foreach`, `break`, `continue`, literal string folding and bounded string concatenation.

```zlang
// A guided tour of the language.
integer values[] = {1, 2, 3};
integer fixed[5] = {4, 5};
unsigned limit = 3U;
boolean ready = true;
string name = "z" + "lang";
string[32] label = name + " 0.3";

function add(integer left, integer right): integer {
    return left + right;
}

fixed[0] = add(values[1], 2);
append(fixed, 9);

{
    integer scoped = add(1, 2);
    print("scoped");
}

if (fixed[0] > 3) {
    print("big");
} else if (fixed[0] == 3) {
    print("three");
} else {
    print("small");
}

while (ready) {
    break;
}

repeat (limit) {
    continue;
}

foreach (item in values) {
    if (item == 2) { continue; }
    print("item");
}

boolean ok = !(ready && !ready) || fixed[0] != 99;
print(label);
```

`zlang-compile example.zlang --emit-c` produces C along these lines:

```c
#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>

int32_t add(int32_t left, int32_t right) {
    return zlang_add_i32(left, right);
}

int main(void) {
    struct zlang_sequence_i32_3_t {
        int32_t data_[3];
        uint32_t length_;
    };
    struct zlang_sequence_i32_3_t values = { { 1, 2, 3 }, 3U };
    (void)values;
    struct zlang_sequence_i32_5_t {
        int32_t data_[5];
        uint32_t length_;
    };
    struct zlang_sequence_i32_5_t fixed = { { 4, 5, 0, 0, 0 }, 2U };
    (void)fixed;
    uint32_t limit = 3U;
    (void)limit;
    bool ready = true;
    (void)ready;
    zlang_string_t name = (zlang_string_t){ "zlang", 5U };
    (void)name;
    char label_storage[33] = { 0 };
    zlang_string_t label =
        zlang_string_concat(label_storage, 32U,
                            (zlang_string_t[]){ name, (zlang_string_t){ " 0.3", 4U } }, 2U);
    {
        uint32_t _zlang_index_0 = 0;
        if (_zlang_index_0 < fixed.length_) {
            fixed.data_[_zlang_index_0] =
                add((((1) < values.length_)
                         ? values.data_[1]
                         : (zlang_trap(2U), values.data_[0])),
                    1);
        } else {
            zlang_trap(2U);
        }
    }
    zlang_array_append(fixed.data_, &fixed.length_, 5U, sizeof(fixed.data_[0]),
                       (int32_t[]){ (9) });
    {
        int32_t scoped = add(1, 2);
        (void)scoped;
        zlang_print((zlang_string_t){ "scoped", 6U });
    }
    if ((((0) < fixed.length_) ? fixed.data_[0]
                              : (zlang_trap(2U), fixed.data_[0])) > 3) {
        zlang_print((zlang_string_t){ "big", 3U });
    } else if ((((0) < fixed.length_) ? fixed.data_[0]
                                      : (zlang_trap(2U), fixed.data_[0])) == 3) {
        zlang_print((zlang_string_t){ "three", 5U });
    } else {
        zlang_print((zlang_string_t){ "small", 5U });
    }
    while (ready) {
        break;
    }
    for (uint32_t _repeat = 0U; _repeat < limit; ++_repeat) {
        continue;
    }
    for (uint32_t index = 0U; index < values.length_; ++index) {
        int32_t item = values.data_[index];
        (void)item;
        if (item == 2) {
            continue;
        }
        zlang_print((zlang_string_t){ "item", 4U });
    }
    bool ok = ((!(ready && (!ready))) ||
               ((((0) < fixed.length_) ? fixed.data_[0]
                                       : (zlang_trap(2U), fixed.data_[0])) != 99));
    (void)ok;
    zlang_print(label);
    return 0;
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
- [0.2 grammar additions](docs/grammar_02.md)

The generated Doxygen site is written to `build/docs/html/index.html`.
