---
title: Emitted C
description: Type mapping, construct mapping and generated names.
navigation:
  title: Emitted C
---

# Emitted C

`zc` writes C99 into `<stem>.c`. The file starts with activation macros for imported functions, then the runtime includes. The Fibonacci example imports `write` from `@std/io`, so its golden C activates `ZLANG_STD_IO_WRITE_STRING` for strings and `ZLANG_STD_IO_WRITE_I32` for integers. This is an excerpt from `examples/algorithms/fibonacci/main.c`:

```c
#define ZLANG_STD_IO_WRITE_STRING

#define ZLANG_STD_IO_WRITE_I32

#include <stdbool.h>
#include <stdint.h>

#include <zlang/runtime/runtime.h>
```

## Type mapping

| Zlang | Emitted C |
| --- | --- |
| `integer` | `int32_t` |
| `unsigned` | `uint32_t` |
| `int64` | `int64_t` |
| `uint64` | `uint64_t` |
| `float` | `float` |
| `double` | `double` |
| `boolean` | `bool` |
| `string` | `zlang_string_t` |
| `array[N] T name` | `app_array_name_t` with `ZLANG_NAME_CAPACITY`; Fibonacci uses `app_array_fib_t` and `ZLANG_FIB_CAPACITY` |

## Construct mapping

- Arithmetic calls checked helpers such as `zlang_add_i32`.
- Logical operators use native C short-circuit behavior.
- `foreach` becomes a native `for` loop with a generated index.
- Indexed access uses bounds checks.
- `write(value)` calls the `@std/io` symbol registered for the argument type. With an `integer`, it calls `zlang_std_io_write_i32`.
- Bounds failures call `zlang_trap(ZLANG_TRAP_BOUNDS)`.

## Generated names

The emitter derives generated names from source constructs. The array declaration `array[10] integer fib = {0, 1};` produces the capacity macro and typedef. The string literals in `write(" ")` and `write("\n")` produce interned views. This excerpt from `examples/algorithms/fibonacci/main.c` shows the declarations:

```c
/* generated capacities */
#define ZLANG_FIB_CAPACITY 10U
#define ZLANG_FIB_LENGTH ZLANG_FIB_EXTENT_0
#define ZLANG_FIB_EXTENT_0 10U


/* generated sequence types */
typedef struct app_array_fib_t {
    int32_t data_[ZLANG_FIB_CAPACITY];
} app_array_fib_t;

/* generated string literals */
#define ZLANG_LITERAL_0_LENGTH 1U
static const char zlang_gen_literal_0_data[] = " ";
static const zlang_string_t zlang_gen_literal_0 = { zlang_gen_literal_0_data, ZLANG_LITERAL_0_LENGTH };
#define ZLANG_LITERAL_1_LENGTH 1U
static const char zlang_gen_literal_1_data[] = "\n";
static const zlang_string_t zlang_gen_literal_1 = { zlang_gen_literal_1_data, ZLANG_LITERAL_1_LENGTH };
```

| Generated name | Source construct |
| --- | --- |
| `app_array_fib_t` | the array `fib`; declared in the typedef above |
| `ZLANG_FIB_CAPACITY` | the capacity `10` in `array[10] integer fib` |
| `zlang_gen_literal_0` | `" "` in `write(" ")`, declared above |
| `zlang_gen_literal_1` | `"\n"` in `write("\n")`, declared above |

The two indexed reads in the assignment `fib[index] = fib[index - 1] + fib[index - 2];` become temporaries computed with `zlang_sub_i32` before the addition. The `foreach (value in fib)` loop gets its own index. This excerpt from the same golden file shows both loops:

```c
    for (int32_t app_local_index = 2; (app_local_index < 10); app_local_index = zlang_add_i32(app_local_index, 1)) {
        int32_t zlang_gen_read_3 = app_local_fib.data_[(uint32_t)zlang_sub_i32(app_local_index, 1)];
        int32_t zlang_gen_read_4 = app_local_fib.data_[(uint32_t)zlang_sub_i32(app_local_index, 2)];
        app_local_fib.data_[(uint32_t)app_local_index] = zlang_add_i32(zlang_gen_read_3, zlang_gen_read_4);
    }

    for (uint32_t zlang_gen_foreach_5 = ZLANG_ZERO; zlang_gen_foreach_5 < 10; ++zlang_gen_foreach_5) {
        int32_t app_local_value = app_local_fib.data_[zlang_gen_foreach_5];
        zlang_std_io_write_i32(app_local_value);
        zlang_std_io_write_string(zlang_gen_literal_0);
    }
```

| Generated name | Source construct |
| --- | --- |
| `app_fn_main` | `function main(): integer` |
| `app_local_fib` | the local variable `fib` |
| `app_local_index` | the `for` variable `index` |
| `app_local_value` | the `foreach` variable `value` |
| `zlang_gen_read_3` | `fib[index - 1]`, declared in the loop above |
| `zlang_gen_read_4` | `fib[index - 2]`, declared in the loop above |
| `zlang_gen_foreach_5` | the index of `foreach (value in fib)`, declared in the loop header above |

The calls to `zlang_std_io_write_i32` and `zlang_std_io_write_string` above come from the example's `write(value)` and `write(" ")` calls. They are not part of any generated function's contract.

The full generated file is committed next to the example.
