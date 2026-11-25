# Array fields

Embeds a fixed `2 x 3` integer array inside a struct and accesses one of its cells.

This example exercises:

- a struct declaration with an array field;
- generation of the named C typedef for the embedded array;
- rank-2 field indexing for both read and write;
- literal extents passed to the generated typed helpers.

The golden C shows `_zlang_array_set_2_i32(_matrix.cells_.data_, 2, 3, 1, 2, 7)`.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.array_fields
```
