# Sieve of Eratosthenes

Marks composite numbers below `30` in a boolean array and prints the primes.

This example exercises:

- a fixed boolean array;
- nested `for` loops and a primality condition;
- indexed writes with a computed index;
- a second loop that prints only prime positions.

The generated C uses bounds checks around every dynamic store.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.algorithms.sieve
```
