# Functions with user-defined types

Returns a struct from one function and mutates it through a reference parameter in another.

This example exercises:

- structs in parameter and return signatures;
- a struct returned by value;
- `reference Point` lowered to a C pointer;
- `clone int32` lowered to a scalar value;
- field reads and field assignment through generated names.

The golden C uses `_u_make` and `_u_shift`.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.functions_user_types
```
