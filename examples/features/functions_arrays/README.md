# Array parameters and returns

Passes an inferred array to a function, updates a cell and returns the whole array.

This example exercises:

- inferred `clone array` parameters;
- inferred `array` return types;
- a local inferred array result;
- whole-array return by value;
- one monomorphized C function for the called shape.

The golden C shows a canonical array typedef shared by the argument and return value.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.functions_arrays
```
