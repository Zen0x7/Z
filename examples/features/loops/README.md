# Loops

Uses every loop form and both loop-control statements in one program.

This example exercises:

- `while` and `repeat`;
- native `for` with a signed counter;
- `break` and `continue`;
- `foreach` with only an item name and with both index and item names;
- checked compound assignment inside loops.

The generated C uses native `for` loops and `_zlang_foreach_*` index names.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.features.loops
```
