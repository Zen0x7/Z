# One-dimensional arrays

Writes four cells of a fixed five-element array and prints all five values.

This example exercises:

- explicit fixed array declarations;
- indexed assignment with constant indices;
- int32 arithmetic feeding a stored value;
- `foreach` over the whole array.

Each dynamic store is lowered to a guarded C block that traps on an out-of-bounds index.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.arrays_1d
```
