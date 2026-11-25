# Bubble sort

Sorts a fixed six-element integer array with nested native `for` loops. Each inner pass compares
two neighbours and swaps them through checked indexed writes.

This example exercises:

- fixed `array[N] integer` declarations and nested initializers;
- indexed reads and writes with expressions such as `index + 1`;
- compound assignment and a temporary for the swap;
- `foreach` over the sorted array and scalar printing.

The generated C uses `zlang_array_get_i32` for reads and explicit bounds checks for writes.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.algorithms.bubble_sort
```
