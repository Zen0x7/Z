# Functions

Calls a function that returns a value and another function that mutates a caller-owned scalar.

This example exercises:

- `clone integer` parameters and an integer return type;
- a `reference integer` parameter;
- compound assignment through a dereferenced pointer in generated C;
- calling both functions from `main`.

The golden C names user functions with the `_u_` prefix.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.functions
```
