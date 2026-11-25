# Arithmetic

Calculates one value for each arithmetic operator and prints them on one line.

This example exercises:

- signed `integer` and unsigned `unsigned` values;
- `+`, `-`, `*`, `/`, `%` and unary `-`;
- unsigned literals such as `5U`;
- all compound numeric assignments;
- scalar printing of both signed and unsigned values.

The generated C dispatches signed operations to `zlang_*_i32` and unsigned addition to
`zlang_add_u32`.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.arithmetic
```
