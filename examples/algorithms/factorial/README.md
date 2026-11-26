# Factorial

Computes `6!` by accumulating a product in a native `for` loop.

This example exercises:

- int32 declarations and initialization;
- `for` with an `int32` loop counter;
- checked multiplication through compound assignment;
- scalar printing with a trailing newline.

The generated C calls `zlang_mul_i32` for each checked multiplication.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.algorithms.factorial
```
