# Matrix multiplication

Multiplies two fixed `2 x 2` int32 matrices and prints the result in row-major order.

This example exercises:

- multidimensional arrays with nested initializers;
- three nested native `for` loops;
- rank-2 indexed reads and writes;
- multidimensional `foreach` with one index name per axis;
- the generated `_zlang_array_get_2_i32` and `_zlang_array_set_2_i32` helpers.

The golden C shows literal extents passed to each typed helper call.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.algorithms.matrix_multiply
```
