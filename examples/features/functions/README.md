# Functions

Calls a function that returns a value and another function that mutates a caller-owned scalar.

This example exercises:

- `clone int32` parameters and an int32 return type;
- a `reference int32` parameter;
- compound assignment through a dereferenced pointer in generated C;
- calling both functions from `main`.

The golden C names user functions with the `_u_` prefix.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.functions
```
