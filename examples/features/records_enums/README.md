# Records and enums

Declares a `Point` struct and a `Color` enum, assigns fields, selects an enum item and prints all
three values.

This example exercises:

- struct declarations and field assignment;
- enum declarations and named enum constants;
- scalar printing of fields and enum values;
- named generated C typedefs and constants.

The golden C emits `zlang_point_t`, `zlang_color_t` and `ZLANG_COLOR_BLUE`.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.structs_enums
```
