# Const qualifiers

Reads a record through a `const reference` parameter and a scalar through a `const clone`
parameter.

This example exercises:

- mandatory `clone` or `reference` parameter qualifiers;
- read-only references to user-defined records;
- read-only by-value scalar parameters;
- field reads through a generated pointer;
- avoiding `ZLANG3004` by using a record reference instead of a small scalar reference.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.const_qualifiers
```
