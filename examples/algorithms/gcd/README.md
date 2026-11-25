# Greatest common divisor

Computes `gcd(1071, 462)` with Euclid's algorithm.

This example exercises:

- a function with two `clone integer` parameters;
- `while` iteration and a `return` value;
- modulo and variable reassignment;
- calling a user function inside `print`.

The generated C names the function `_u_gcd` and uses `zlang_mod_i32` for checked modulo.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.algorithms.gcd
```
