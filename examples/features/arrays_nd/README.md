# Multidimensional arrays

Sums every cell of rank-2, rank-3 and rank-4 integer arrays.

This example exercises:

- fixed extents at every axis;
- nested initializers up to four dimensions;
- one index name per axis in multidimensional `foreach`;
- generated offset helpers for ranks 2, 3 and 4;
- only the array helpers actually referenced by the generated code.

This is the broadest generated-C shape in the example suite.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.arrays_nd
```
