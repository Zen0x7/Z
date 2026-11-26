# Pi approximation

Approximates pi with the Leibniz series using int32 arithmetic scaled by `100000`.

This example exercises:

- a long native `for` loop;
- checked addition, subtraction and multiplication through compound assignment;
- a boolean toggle for alternating terms;
- printing int32 and fractional parts separately.

The result is printed as `314159...` split around a literal decimal point.

```sh
ctest --test-dir build --output-on-failure -R zlang.examples.algorithms.pi
```
