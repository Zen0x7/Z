# Collatz

Computes how many steps the Collatz sequence starting at `27` takes to reach `1`.

This example exercises:

- `while` with a non-trivial condition;
- modulo and integer division;
- conditional arithmetic through `if` / `else`;
- compound assignment for the step counter.

The program prints only the final step count.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.algorithms.collatz
```
