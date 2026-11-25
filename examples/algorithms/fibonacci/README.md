# Fibonacci

Fills a fixed ten-element array with the first Fibonacci numbers, then prints them separated by
spaces.

This example exercises:

- fixed arrays with a partial initializer;
- indexed reads with `index - 1` and `index - 2`;
- checked indexed writes inside a `for` loop;
- `foreach` and scalar printing.

Dynamic signed indices are emitted with explicit `uint32_t` casts before bounds checks.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.algorithms.fibonacci
```
