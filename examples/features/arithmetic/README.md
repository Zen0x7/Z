# Arithmetic

Calculates one value for each arithmetic operator and prints them on one line.

This example exercises:

- signed `int32` and uint32 `uint32` values;
- `+`, `-`, `*`, `/`, `%` and unary `-`;
- uint32 literals such as `5U`;
- all compound numeric assignments;
- scalar printing of both signed and uint32 values.

The generated C dispatches signed operations to `zlang_*_i32` and uint32 addition to
`zlang_add_u32`.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.arithmetic
```
