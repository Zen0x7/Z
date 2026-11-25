# Factorial

Computes `6!` by accumulating a product in a native `for` loop.

This example exercises:

- integer declarations and initialization;
- `for` with an `integer` loop counter;
- checked multiplication through compound assignment;
- scalar printing with a trailing newline.

The generated C calls `zlang_mul_i32` for each checked multiplication.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.algorithms.factorial
```
