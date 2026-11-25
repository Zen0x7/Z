# Const arrays and whole-array copy

Copies a `const array[3] integer` into a mutable array of the same shape, then prints the target.

This example exercises:

- `const` arrays;
- whole-array assignment with identical extents;
- lowering the copy to `memcpy` over the generated array structs;
- `foreach` over the copied values.

Shape mismatch is rejected by semantic analysis; this example uses matching shapes.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.arrays_const_copy
```
