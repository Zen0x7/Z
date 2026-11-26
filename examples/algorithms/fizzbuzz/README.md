# FizzBuzz

Prints the classic FizzBuzz sequence from `1` to `15`.

This example exercises:

- a native `for` loop;
- an `if` / `else if` / `else if` / `else` chain;
- modulo tests for divisibility by `15`, `3` and `5`;
- printing both string literals and int32 values.

Each iteration ends with an explicit newline string.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.algorithms.fizzbuzz
```
