# Array function specializations

Calls one inferred-array function with two different array shapes in the same program.

This example exercises:

- `clone array` parameters without explicit extents;
- inference of the parameter shape from each call argument;
- one generated C function per shape;
- a scalar return type on a function with an inferred array parameter;
- resolving the return type of specialized calls during emission.

The golden C contains both `_u_first_2_3_i32` and `_u_first_4_i32`.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.functions_array_specializations
```
