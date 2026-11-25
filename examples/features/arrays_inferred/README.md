# Inferred arrays

Declares arrays without explicit extents and lets initializers and constant indices determine
their shapes.

This example exercises:

- `array[] T` inferred from a flat initializer;
- `array T` inferred from a nested initializer;
- `length` on both inferred arrays;
- mixing rank-1 and rank-2 indexed reads in one expression;
- distinct generated C typedefs for each inferred shape.

The generated extents come from each symbol, not from the active function specialization.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.arrays_inferred
```
