# Sum of squares

Sums the squares of the first five positive integers.

This example exercises:

- a fixed array initializer;
- `foreach` without an index name;
- checked multiplication through `value * value`;
- checked accumulation through compound assignment.

It is a compact example of array iteration combined with checked arithmetic.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.algorithms.sum_of_squares
```
